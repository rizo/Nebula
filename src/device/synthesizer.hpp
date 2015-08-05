/**
   \file synthesizer.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief A simple audio synthesizer with 16 channels.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   Each channel is capable of producing a pure tone with a variable volume.
 */

#ifndef NEBULA_DEVICE_SYNTHESIZER_HPP_
#define NEBULA_DEVICE_SYNTHESIZER_HPP_

#pragma once

#include <platform/audio.hpp>
#include <platform/computer.hpp>

DEFINE_LOGGER(SYNTHESIZER)

namespace nebula {

const std::chrono::microseconds kSynthesizerClockPeriod{1000};

const std::chrono::milliseconds kSynthesizerDefaultWholeNoteDuration{4000};

enum class SynthesizerOperation {
  SetWholeNote,
  SetTone,
  Play,
  Pause,
  EnableInterrupts,
};

class SynthesizerState {
 private:
  audio::Device device_;
  std::chrono::microseconds whole_note_duration_;
  std::vector<std::chrono::microseconds> remaining_duration_;
  bool is_playing_{false};
  bool interrupts_enabled_{false};
  Word interrupt_message_{0u};

 public:
  explicit SynthesizerState()
      : device_{make_unique<audio::Mixer>()},
        whole_note_duration_{kSynthesizerDefaultWholeNoteDuration},
        remaining_duration_(audio::Mixer::NumChannels,
                            std::chrono::microseconds{0}) {}

  friend class Synthesizer;
};

class Synthesizer : public Simulation<SynthesizerState>, public IDevice {
 private:
  shared_ptr<Computer> computer_;
  InterruptSink interrupt_sink_;
  unique_ptr<SynthesizerState> state_;

  void handleInterrupt(SynthesizerOperation operation,
                       ProcessorState& processor_state);

  void decrementRemainingDurations(
      std::chrono::microseconds time_spent_playing);

 public:
  explicit Synthesizer(shared_ptr<Computer> computer)
      : Simulation<SynthesizerState>{},
        computer_{computer},
        interrupt_sink_{computer->registerDevice(deviceInfo())},
        state_{make_unique<SynthesizerState>()} {}

  inline DeviceInfo deviceInfo() const override {
    return {DeviceID{0xf649003d}, DeviceManufacturer{0xdecaf000},
            DeviceVersion{0x0002}};
  }

  unique_ptr<SynthesizerState> start() override;
};

}  // namespace nebula

#endif  // NEBULA_DEVICE_SYNTHESIZER_HPP_
