/**
   \file main.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief The main entry-point of Nebula.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   The start-up process of Nebula entails:
   - Parsing command-line options.
   - Initializing program logging (if it's enabled).
   - Loading a data image from an external file and populating the virtual
   memory with its contents.
   - Launching each hardware device (including the processor) with its own
   thread.
   - Entering the event loop, which polls for input events and renders
   graphics.

   When a termination event is received, all devices and I/O threads should
   gracefully terminate.
 */

#include "device/clock.hpp"
#include "device/keyboard.hpp"
#include "device/monitor.hpp"
#include "device/synthesizer.hpp"
#include "device/vector-display.hpp"
#include "platform/computer.hpp"
#include "platform/event.hpp"
#include "platform/execution-manager.hpp"
#include "platform/graphics.hpp"

#include <boost/program_options.hpp>

#include <iostream>

DEFINE_LOGGER(MAIN)

using namespace nebula;

namespace po = boost::program_options;

static void executeAll(const po::variables_map &vars) {
  auto do_logging = vars.count("verbose") != 0;
  if (do_logging) {
    auto log_path = fs::path{vars["verbose"].as<std::string>()};
    logging::initialize(log_path, logging::Severity::info);
  } else {
    logging::initialize();
  }

  graphics::initialize();
  audio::initialize();

  std::atexit([] { graphics::terminate(); });

  auto memory = std::make_shared<Memory>(0x10000u * units::binary::Word);
  auto processor_state = make_unique<ProcessorState>();

  auto endianness = vars.count("little-endian") != 0u ? ByteOrder::LittleEndian
                                                      : ByteOrder::BigEndian;
  auto file = fs::path{vars["memory-file"].as<std::string>()};
  memory->fillFromFile(file, endianness);

  auto computer =
      std::make_shared<Computer>(std::move(processor_state), memory);

  ExecutionManagerOptions manager_options;

  if (vars.count("period") != 0u) {
    manager_options.clockPeriod(
        ProcessorClockPeriod{vars["period"].as<std::size_t>()});
  }

  if (vars.count("halt-first") != 0u) {
    manager_options.doInitialHalt(true);
  }

  ExecutionManager manager{computer, manager_options};

  auto monitor_window = graphics::createWindow(
      "Monitor", kWindowHorizontalResolution, kWindowVerticalResolution);

  auto vector_display_window =
      graphics::createWindow("Vector Display",
                             kVectorDisplayResolution,
                             kVectorDisplayResolution,
                             graphics::WindowType::ThreeDimensional);

  graphics::gl::Context vector_display_context{
      std::move(vector_display_window)};

  Clock clock{computer};
  Monitor monitor{computer, memory};
  VectorDisplay vector_display{computer, memory};
  Keyboard keyboard{computer};
  Synthesizer synthesizer{computer};

  auto processor_state_future = launch(manager);
  auto clock_state_future = launch(clock);
  auto monitor_state_future = launch(monitor);
  auto vector_display_state_future = launch(vector_display);
  auto keyboard_state_future = launch(keyboard);
  auto synthesizer_state_future = launch(synthesizer);

  vector_display.initializeGL(vector_display_context);

  bool do_quit{false};

  auto event_handler =
      event::makeHandler([&](const event::Quit &) { do_quit = true; },
                         [&](const event::KeyInput &key_input) {
        keyboard.state().setKey(key_input.code);
      });

  do {
    auto now = std::chrono::system_clock::now();

    auto event = event::poll();
    if (event) {
      event::handle(event_handler, event.get());
    }

    if (do_quit) {
      break;
    }

    monitor.renderGraphics(monitor_window);
    vector_display.renderGL(vector_display_context);

    std::this_thread::sleep_until(now + graphics::kFramePeriod);
  } while (
      !(isFinished(processor_state_future) || isFinished(clock_state_future) ||
        isFinished(monitor_state_future) || isFinished(keyboard_state_future) ||
        isFinished(vector_display_state_future) ||
        isFinished(synthesizer_state_future)));

  manager.stop();
  clock.stop();
  monitor.stop();
  keyboard.stop();
  vector_display.stop();
  synthesizer.stop();

  processor_state = processor_state_future.get();
  auto clock_state = clock_state_future.get();
  auto monitor_state = monitor_state_future.get();
  auto vector_display_state = vector_display_state_future.get();
  auto keyboard_state = keyboard_state_future.get();
  auto synthesizer_state = synthesizer_state_future.get();

  if (processor_state->read(Flag::Aborted)) {
    auto error_info = processor_state->getError();

    std::cout << "The DCPU16 aborted." << std::endl;
    std::cout << format(
                     "ABT was invoked near 0x%04x. The message was:\n\"%s\"\n") %
                     error_info->pc % error_info->message;
  }

  // The newline is there because the pretty-printing assumes it's starting on a
  // new line.
  LOG(MAIN, info) << "\n" << *processor_state;

  if (vars.count("dump") != 0u) {
    fs::path memory_file_path{vars["dump"].as<std::string>()};
    memory->writeToFile(memory_file_path, endianness);
  }
}

int main(int argc, char *argv[]) {
  po::options_description visible_options;
  visible_options.add_options()
      // Help message.
      ("help,h", "Produce this message.")

      // Endianness.
      ("little-endian,e", "Assume the little-endian memory encoding.")

      // Halt initially.
      ("halt-first,a",
       "Enter the halted state prior to executing any instructions.")

      // Verbosity.
      ("verbose,v",
       po::value<std::string>(),
       "Enable verbose logging to the named file.")

      // Processor clock period.
      ("period,p",
       po::value<std::size_t>(),
       "Target processor clock period, in nanoseconds. Omitting this "
       "options "
       "results in a free-running processor.")

      // Dumping memory.
      ("dump,d",
       po::value<std::string>(),
       "Dump the state of memory to the named file at the conclusion of "
       "execution.");

  po::options_description hidden_options;
  hidden_options.add_options()
      // Input file.
      ("memory-file",
       po::value<std::string>(),
       "The memory file initially loaded by the DCPU-16.");

  po::options_description all_options;
  all_options.add(visible_options).add(hidden_options);

  po::positional_options_description positional_options;
  positional_options.add("memory-file", 1);

  po::variables_map vars;

  try {
    po::store(po::command_line_parser(argc, argv)
                  .options(all_options)
                  .positional(positional_options)
                  .run(),
              vars);

    po::notify(vars);
  }
  catch (po::error &err) {
    std::cerr << "nebula: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }

  if (vars.count("help") || !vars.count("memory-file")) {
    std::cout << "This is nebula, the DCPU-16 emulator." << std::endl;
    std::cout << "Copyright 2015 Jesse Haber-Kucharsky" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: nebula [OPTIONS] memory-file" << std::endl;
    std::cout << std::endl;
    std::cout << visible_options << std::endl;
    return EXIT_SUCCESS;
  }

  try {
    executeAll(vars);
  }
  catch (std::exception &e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
