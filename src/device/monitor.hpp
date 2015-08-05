/**
   \file monitor.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief A simulated computer monitor.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   \see /doc/hw/monitor.txt
 */

#ifndef NEBULA_DEVICE_MONITOR_HPP_
#define NEBULA_DEVICE_MONITOR_HPP_

#pragma once

#include "monitor-units.hpp"

#include <platform/computer.hpp>
#include <platform/graphics.hpp>
#include <platform/simulation.hpp>

#include <array>
#include <mutex>

DEFINE_LOGGER(MONITOR)

namespace nebula {

/**
   \brief The duration over which monitor cells blink.
 */
const std::chrono::milliseconds kMonitorBlinkPeriod{1000};

/**
   \brief The internal clock period of the monitor.

   This is effectively the resolution at which the monitor can respond to
   interrupts. Actual rendering is done in the graphics rendering loop and is
   independent of this value.
 */
const std::chrono::microseconds kMonitorClockPeriod{10};

/**
   \brief The default palette of the monitor.
 */
const std::array<Word, 16> kMonitorDefaultPalette{
    {0x0000, 0x000a, 0x00a0, 0x00aa, 0x0a00, 0x0a0a, 0x0a50, 0x0aaa,
     0x0555, 0x055f, 0x05f5, 0x05ff, 0x0f55, 0x0f5f, 0x0ff5, 0x0fff}};

enum class MonitorOperation {
  MapVideoMemory,
  MapFontMemory,
  MapPaletteMemory,
  SetBorderColor,
  DumpFont,
  DumpPalette
};

struct MonitorState {
  bool is_connected{false};
  std::chrono::time_point<std::chrono::system_clock> last_render_time{};
  std::chrono::microseconds since_last_blink{0};
  bool is_blink_visible{true};
  Word border_color_offset{9};
  Word video_memory_offset{0};
  Word font_memory_offset{0};
  Word palette_memory_offset{0};
};

/**
   \brief A type-safe wrapper over an index into the monitor font.
 */
struct CharacterOffset {
  std::uint8_t value;
  explicit CharacterOffset(std::uint8_t value) : value{value} {}
};

/**
   \brief A type-safe wrapper over a background color.
   This is an offset into a color palette.
 */
struct BackgroundColorOffset {
  std::uint8_t value;
  explicit BackgroundColorOffset(std::uint8_t value) : value{value} {}
};

/**
   \brief A type-safe wrapper over a foreground color.
   This is an offset into a color palette.
 */
struct ForegroundColorOffset {
  std::uint8_t value;
  explicit ForegroundColorOffset(std::uint8_t value) : value{value} {}
};

class Monitor : public Simulation<MonitorState>,
                public IDevice,
                public graphics::IGraphics {
 private:
  shared_ptr<Computer> computer_;
  shared_ptr<Memory> memory_;
  InterruptSink interrupt_sink_;
  MonitorState state_{};
  std::mutex mutex_{};

  void handleInterrupt(MonitorOperation operation,
                       ProcessorState& processor_state);

  /**
     \brief Get a color from a palette based on an offset.
   */
  graphics::Color colorFromOffset(std::uint8_t offset);

  /**
     \brief Extract a real color from a BackgroundColorOffset.
   */
  inline graphics::Color colorFromOffset(BackgroundColorOffset bg) {
    return colorFromOffset(bg.value);
  }
  
  /**
     \brief Extract a real color from a ForegroundColorOffset.
   */
  inline graphics::Color colorFromOffset(ForegroundColorOffset bg) {
    return colorFromOffset(bg.value);
  }

  /**
     \brief Draw the indexed virtual pixel from memory.
     
     This is a low-level function that draws according to the currently set
     canvas color.
   */
  void drawPixel(const graphics::Window& window,
                 units::Quantity<units::simulated::Width> x,
                 units::Quantity<units::simulated::Height> y);

  /**
     \brief Draw the indexed cell from memory.
   */
  void drawCell(const graphics::Window& window,
                std::size_t x,
                std::size_t y,
                CharacterOffset ch_offset,
                ForegroundColorOffset fg_offset,
                BackgroundColorOffset bg_offset,
                bool do_blink);

  /**
     \brief Clear the monitor screen.
   */
  void clear(const graphics::Window& window);

  void drawBorder(const graphics::Window& window);

  void drawFromMemory(const graphics::Window& window);

 public:
  explicit Monitor(shared_ptr<Computer> computer, shared_ptr<Memory> memory)
      : Simulation<MonitorState>{},
        computer_{computer},
        memory_{memory},
        interrupt_sink_{computer->registerDevice(deviceInfo())} {}

  void renderGraphics(const graphics::Window& window) override;

  unique_ptr<MonitorState> start() override;

  inline DeviceInfo deviceInfo() const override {
    return {DeviceID{0x7349f615}, DeviceManufacturer{0x1c6c8b36},
            DeviceVersion{0x1802}};
  }
};

}  // namespace nebula

#endif  // NEBULA_DEVICE_MONITOR_HPP_
