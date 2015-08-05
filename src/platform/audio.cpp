/**
   \file audio.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "audio.hpp"

#include <numeric>

namespace nebula {

namespace audio {

static const Frequency kSamplingFrequency = 48000 * kHertz;

//
// Channel
//

void Channel::generateTone(const Frequency& sampling_frequency,
                           std::size_t stream_length,
                           Word* stream) {
  if (is_muted_) {
    std::fill(stream, stream + stream_length, 0u);
    return;
  }

  using namespace boost::math;

  Word num_steps = sampling_frequency / frequency_;
  Word amplitude =
      (sizeof(Word) / sizeof(VolumeType)) * static_cast<double>(volume_);

  for (std::size_t stream_index = 0; stream_index < stream_length;
       ++stream_index) {
    if (sample_index_ > num_steps) {
      sample_index_ = 0u;
    } else {
      sample_index_ += 1u;
    }

    double relative_frequency = frequency_ / sampling_frequency;
    stream[stream_index] = static_cast<Word>(
        amplitude *
        sin(sample_index_ * 2 * constants::pi<double>() * relative_frequency));
  }
}

//
// Mixer
// 

std::size_t Mixer::numNonMutedChannels() const {
  return std::accumulate(std::begin(channels_),
                         std::end(channels_),
                         0u,
                         [](std::size_t sum, const Channel& ch) {
    return sum + (ch.is_muted_ ? 0u : 1u);
  });
}

optional<Channel&> Mixer::getChannel(std::size_t channel_index) {
  if (channel_index >= NumChannels) {
    return {};
  } else {
    return optional<Channel&>{channels_[channel_index]};
  }
}

void Mixer::mixChannels(std::size_t stream_length, Word* stream) {
  std::vector<Word> audio_buffer(stream_length);
  std::fill(stream, stream + stream_length, 0u);
  std::size_t num_non_muted_channels = numNonMutedChannels();

  for (std::size_t channel_index = 0; channel_index < NumChannels;
       ++channel_index) {
    std::fill(std::begin(audio_buffer), std::end(audio_buffer), 0u);

    channels_[channel_index].generateTone(
        kSamplingFrequency, audio_buffer.size(), &audio_buffer[0]);

    // Scale each channel by the number of non-muted channels.
    std::transform(std::begin(audio_buffer),
                   std::end(audio_buffer),
                   std::begin(audio_buffer),
                   [num_non_muted_channels](Word w) {
      return static_cast<Word>(w / static_cast<float>(num_non_muted_channels));
    });

    for (std::size_t stream_index = 0; stream_index < stream_length;
         ++stream_index) {
      stream[stream_index] += audio_buffer[stream_index];
    }
  }
}

static void mixAudioCallback(void* user_data,
                             uint8_t* stream,
                             int stream_length_bytes) {
  Mixer* mixer = reinterpret_cast<Mixer*>(user_data);
  mixer->mixChannels(stream_length_bytes / 2, reinterpret_cast<Word*>(stream));
}

//
// Device
//

Device::Device(unique_ptr<Mixer> mixer) : mixer_{std::move(mixer)} {
  spec_.freq = kSamplingFrequency.value();
  spec_.format = AUDIO_U16;
  spec_.channels = 1;
  spec_.samples = 32;
  spec_.callback = &mixAudioCallback;
  spec_.userdata = mixer_.get();

  // This function breaks SDL convention and uses a non-zero value to indicate
  // success. Work around the expectations of the wrapSdl() function, which
  // expects that a non-zero value indicates failure.
  id_ = SDL_OpenAudioDevice(nullptr, 0, &spec_, nullptr, 1);
  if (id_ == 0) {
    wrapSdl(1, "Failed to open audio device for playback");
  }
}

void Device::withMixer(std::function<void(Mixer& mixer)> f) {
  DeviceLock lock{*this};
  f(*(this->mixer_));
}

//
// Auxillary functions
// 

void initialize() {
  wrapSdl(SDL_Init(SDL_INIT_AUDIO),
          "Failed to initialize the audio sub-system");
  LOG(AUDIO, info) << "Initialized.";
}

}  // namespace audio

}  // namespace nebula
