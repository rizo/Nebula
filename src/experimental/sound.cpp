#include <boost/format.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/optional.hpp>
#include <boost/units/systems/si.hpp>

extern "C" {
#include <SDL2/SDL.h>
}

#include <chrono>
#include <numeric>
#include <thread>

using Word = std::uint16_t;

using boost::optional;
using boost::format;

using std::unique_ptr;

namespace error {

class SdlError : public std::runtime_error {
 public:
  explicit SdlError(const std::string& message) : std::runtime_error{message} {}
};

}  // namespace error

inline void wrapSdl(int result, const std::string& message) {
  if (result != 0) {
    throw error::SdlError{(format("%s: %s.") % message % SDL_GetError()).str()};
  }
}

namespace audio {

using Frequency = boost::units::quantity<boost::units::si::frequency>;

using VolumeType = std::uint8_t;

const auto kHertz = boost::units::si::hertz;

static const Frequency kMiddleA = 440 * kHertz;

static const Frequency kSamplingFrequency = 48000 * kHertz;

class Channel {
 public:
  static const VolumeType VolumeMax = std::numeric_limits<VolumeType>::max();

 private:
  bool is_muted_ = true;
  Frequency frequency_ = kMiddleA;
  VolumeType volume_ = VolumeMax;

  std::size_t sample_index_ = 0u;

 public:
  explicit Channel() = default;

  inline Channel& isMuted(bool value) {
    is_muted_ = value;
    return *this;
  }

  inline bool isMuted() const { return is_muted_; }

  inline Channel& frequency(Frequency value) {
    frequency_ = value;
    return *this;
  }
  inline const Frequency& frequency() const { return frequency_; }

  inline Channel& volume(VolumeType value) {
    volume_ = value;
    return *this;
  }

  inline VolumeType volume() const { return volume_; }

  void generateTone(const Frequency& sampling_frequency,
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
          amplitude * sin(sample_index_ * 2 * constants::pi<double>() *
                          relative_frequency));
    }
  }

  friend class Mixer;
};

class Mixer {
 public:
  static const std::size_t NumChannels = 4u;

 private:
  std::vector<Channel> channels_;

 public:
  explicit Mixer() : channels_(NumChannels) {}

  std::size_t numNonMutedChannels() const {
    return std::accumulate(std::begin(channels_),
                           std::end(channels_),
                           0u,
                           [](std::size_t sum, const Channel& ch) {
      return sum + (ch.is_muted_ ? 0u : 1u);
    });
  }

  optional<Channel&> getChannel(std::size_t channel_index) {
    if (channel_index >= NumChannels) {
      return {};
    } else {
      return optional<Channel&>{channels_[channel_index]};
    }
  }

  void mixChannels(std::size_t stream_length, Word* stream) {
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
        return static_cast<Word>(w /
                                 static_cast<float>(num_non_muted_channels));
      });

      for (std::size_t stream_index = 0; stream_index < stream_length;
           ++stream_index) {
        stream[stream_index] += audio_buffer[stream_index];
      }
    }
  }
};

static void mixAudioCallback(void* user_data,
                             uint8_t* stream,
                             int stream_length_bytes) {
  Mixer* mixer = reinterpret_cast<Mixer*>(user_data);
  mixer->mixChannels(stream_length_bytes / 2, reinterpret_cast<Word*>(stream));
}

// Forward declaration.
class Device;

class DeviceLock {
 private:
  SDL_AudioDeviceID id_;

 public:
  explicit DeviceLock(const Device& device);
  explicit DeviceLock() = delete;
  explicit DeviceLock(const DeviceLock&) = delete;

  ~DeviceLock() { SDL_UnlockAudioDevice(id_); }
};

class Device {
 private:
  unique_ptr<Mixer> mixer_;
  SDL_AudioSpec spec_{};
  SDL_AudioDeviceID id_{};

 public:
  Device(unique_ptr<Mixer> mixer) : mixer_{std::move(mixer)} {
    spec_.freq = kSamplingFrequency.value();
    spec_.format = AUDIO_U16;
    spec_.channels = 1;
    spec_.samples = 32;
    spec_.callback = &mixAudioCallback;
    spec_.userdata = mixer_.get();

    // This function breaks SDL convention and uses a non-zero value to
    // indicate
    // success. Work around the expectations of the wrapSdl() function, which
    // expects that a non-zero value indicates failure.
    id_ = SDL_OpenAudioDevice(nullptr, 0, &spec_, nullptr, 1);
    if (id_ == 0) {
      wrapSdl(1, "Failed to open audio device for playback");
    }
  }

  ~Device() { SDL_CloseAudioDevice(id_); }

  const Mixer& mixer() const { return *mixer_; }

  void withMixer(std::function<void(Mixer& mixer)> f) {
    DeviceLock lock{*this};
    f(*(this->mixer_));
  }

  void play() { SDL_PauseAudioDevice(id_, 0); }

  void pause() { SDL_PauseAudioDevice(id_, 1); }

  friend class DeviceLock;
};

DeviceLock::DeviceLock(const Device& device) : id_{device.id_} {
  SDL_LockAudioDevice(id_);
}

void initialize() {
  wrapSdl(SDL_Init(SDL_INIT_AUDIO),
          "Failed to initialize the audio sub-system");
}

}  // namespace audio

int main() {
  audio::initialize();

  audio::Device device{std::unique_ptr<audio::Mixer>(new audio::Mixer())};
  device.play();

  bool is_muted = false;
  while (true) {
    device.withMixer([&is_muted](audio::Mixer& mixer) {
      mixer.getChannel(0)->isMuted(is_muted);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    is_muted = !is_muted;
  }
}
