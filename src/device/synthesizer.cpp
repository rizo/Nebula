/**
   \file synthesizer.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "synthesizer.hpp"

namespace nebula {

static const Word kMinChannelIndexForInterrupt = 4u;

// There is a special case for handling SetTone.
static const std::unordered_map<Word, SynthesizerOperation> kOperationMap{
    {0u, SynthesizerOperation::SetWholeNote},
    {1u, SynthesizerOperation::Play},
    {2u, SynthesizerOperation::Pause},
    {3u, SynthesizerOperation::EnableInterrupts}};

void Synthesizer::handleInterrupt(SynthesizerOperation operation,
                                  ProcessorState& processor_state) {
  Word a = processor_state.read(Register::A);
  Word b = processor_state.read(Register::B);

  switch (operation) {
    case SynthesizerOperation::EnableInterrupts: {
      LOG(SYNTHESIZER, info) << "'EnableInterrupts'";

      if (b != 0) {
        state_->interrupts_enabled_ = true;
        state_->interrupt_message_ = b;
      } else {
        state_->interrupts_enabled_ = false;
      }
    } break;

    case SynthesizerOperation::SetWholeNote: {
      LOG(SYNTHESIZER, info) << "'SetWholeNote'";
      state_->whole_note_duration_ = std::chrono::microseconds{b * 1000};
    } break;

    case SynthesizerOperation::SetTone: {
      LOG(SYNTHESIZER, info) << "'SetTone'";

      Word c = processor_state.read(Register::C);

      //
      // The volume is stored in the lower-order byte and the divider is stored
      // in the higher-order byte.
      //

      std::size_t channel_index = a - kMinChannelIndexForInterrupt;
      double frequency = b / 100.0;
      std::uint8_t volume = c & 0xff;
      std::uint8_t divider = (c & 0xff00) >> 8;

      auto note_duration =
          std::chrono::duration_cast<std::chrono::microseconds>(
              state_->whole_note_duration_ * (1.0 / divider));

      LOG(SYNTHESIZER, info) << format("Setting channel %u\n") % channel_index
                             << format("Frequency: %lf Hz\n") % frequency
                             << format("Volume   : %d/255\n") %
                                    static_cast<int>(volume)
                             << format("Duration : %u ms") %
                                    (note_duration.count() / 1000);

      state_->device_.withMixer([&](audio::Mixer& mixer) {
        mixer.getChannel(channel_index)
            ->frequency(frequency * audio::kHertz)
            .isMuted(false)
            .volume(volume);
      });

      state_->remaining_duration_[channel_index] = note_duration;
    } break;

    case SynthesizerOperation::Play: {
      LOG(SYNTHESIZER, info) << "'Play'";

      state_->device_.play();
      state_->is_playing_ = true;
    } break;

    case SynthesizerOperation::Pause: {
      LOG(SYNTHESIZER, info) << "'Pause'";

      state_->device_.pause();
      state_->is_playing_ = false;
    } break;
  }
}

void Synthesizer::decrementRemainingDurations(
    std::chrono::microseconds time_spent_playing) {

  for (std::size_t channel_index = 0u;
       channel_index < audio::Mixer::NumChannels;
       ++channel_index) {
    auto& remaining = state_->remaining_duration_[channel_index];

    if (remaining > std::chrono::milliseconds{0}) {
      remaining -= std::min(remaining, time_spent_playing);
    } else {
      state_->device_.withMixer([this, channel_index](audio::Mixer& mixer) {
        auto channel = mixer.getChannel(channel_index);

        if (!channel->isMuted()) {
          channel->isMuted(true);

          if (state_->interrupts_enabled_) {
            computer_->interruptQueue().push(state_->interrupt_message_ +
                                             channel_index);
          }
        }
      });
    }
  }
}

unique_ptr<SynthesizerState> Synthesizer::start() {
  notify();

  LOG(SYNTHESIZER, info) << "Started.";

  while (status() == SimulationStatus::Running) {
    if (interrupt_sink_.isActive()) {
      LOG(SYNTHESIZER, info) << "Got interrupt.";

      auto& processor_state = interrupt_sink_.processorState();
      auto a = processor_state.read(Register::A);

      if ((a >= kMinChannelIndexForInterrupt) &&
          (a < (kMinChannelIndexForInterrupt + audio::Mixer::NumChannels))) {
        handleInterrupt(SynthesizerOperation::SetTone, processor_state);
      } else {
        auto operation = get(a, kOperationMap);
        if (operation) {
          handleInterrupt(*operation, processor_state);
        }
      }

      interrupt_sink_.respond();
      LOG(SYNTHESIZER, info) << "Finished handling interrupt.";
    }

    auto time_started_playing = std::chrono::system_clock::now();
    std::this_thread::sleep_for(kSynthesizerClockPeriod);
    decrementRemainingDurations(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now() - time_started_playing));
  }

  LOG(SYNTHESIZER, info) << "Shutting down.";
  return std::move(state_);
}

}  // namespace nebula
