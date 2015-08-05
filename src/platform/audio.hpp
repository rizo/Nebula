/**
   \file audio.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Audio tone generation and mixing.

   One way to use this module would be something like this:

   \code{.cpp}
   audio::Device ad(make_unique<audio::Mixer>());
   
   ad.withMixer([](audio::Mixer& mixer) {
     mixer.getChannel(0)->frequency(550 * audio::kHertz).isMuted(false);
     mixer.getChannel(1)->frequency(123 * audio::kHertz).volume(50).isMuted(false);
   });

   ad.play();
   \endcode

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#ifndef NEBULA_AUDIO_HPP_
#define NEBULA_AUDIO_HPP_

#pragma once

#include "prelude.hpp"
#include "sdl.hpp"

#include <boost/math/constants/constants.hpp>
#include <boost/units/systems/si.hpp>

DEFINE_LOGGER(AUDIO)

namespace nebula {

namespace audio {

/**
   \brief A double-valued frequency quantity, expressed in Hertz.

   \seealso kHertz
 */
using Frequency = boost::units::quantity<boost::units::si::frequency>;

using VolumeType = std::uint8_t;

/**
   \brief A Frequency of 1 Hz.

   To create a frequency quantity, multiply a numerical literal by the quantity.
   For instance:

   \code{.cpp}
   // A frequency of 10 Hz.
   auto f = 10 * kHertz
   \endcode

   \seealso Frequency
 */

const auto kHertz = boost::units::si::hertz;

/**
   \brief Middle A note, for reference and/or convenience.
 */
const Frequency kMiddleA = 440 * kHertz;

/**
   \brief An independent audio channel.

   Audio channels can generate sinusoidal tones at a constant frequency, with
   volume adjustment.

   Channels start out muted, at maximum volume, and at kMiddleA frequency.
 */
class Channel {
 public:
  static const VolumeType VolumeMax = std::numeric_limits<VolumeType>::max();

 private:
  bool is_muted_ = true;
  Frequency frequency_ = kMiddleA;
  VolumeType volume_ = VolumeMax;

  /**
     This is the discrete sample index that is maintained across multiple calls
     to generateTone() so that the sinusoidal wave is continuous. If the index
     started at zero every time a tone was generated, then there would be
     discontinuous jumps in the amplitude of the output.
   */
  std::size_t sample_index_ = 0u;

 public:
  /**
     \brief Create a new channel.

     Channels are created with default configuration (see the documentation for
     Channel) and modified as per the following example:

     \code{.cpp}
     auto channel = Channel().isMuted(false).frequency(20 * kHertz);
     \endcode
   */
  explicit Channel() = default;

  /**
     \brief Mute and un-mute the channel.

     Muted channels produce silence, even when asked to generate a tone.
   */
  inline Channel& isMuted(bool value) {
    is_muted_ = value;
    return *this;
  }

  /**
     \brief Check if the channel is muted.
   */
  inline bool isMuted() const { return is_muted_; }

  /**
     \brief Set the channel frequency.

     Channels generate pure sinusoidal tones at the requested frequency.
   */
  inline Channel& frequency(Frequency value) {
    frequency_ = value;
    return *this;
  }

  /**
     \brief Get the current channel frequency.
   */
  inline const Frequency& frequency() const { return frequency_; }

  /**
     \brief Set the channel volume.

     The volume scale is based on the capacity of VolumeType. A value of zero
     indicates a muted channel. The maximum supported value of VolumeType
     represents the maximum volume.
   */
  inline Channel& volume(VolumeType value) {
    volume_ = value;
    return *this;
  }

  /**
     \brief Get the current channel volume.
   */
  inline VolumeType volume() const { return volume_; }

  /**
     \brief Fill a buffer with a sinusoidal tone.

     The sinusoidal tone is generated based on the configuration of the channel,
     including its frequency(), volume(), and whether or not it isMuted().

     The tone data is expressed as a series of unsigned 16 bit words, where each
     word represents a discretized sample of a continuous sinusoidal function.

     \note The current state of the tone is maintained internally. This means
     that if a tone is generated so that the last sample is the peak of wave,
     the next time that generateTone() is called, the tone will resume from that
     peak rather than start at zero.

     \param sampling_frequency The number of samples that are
     generated for every second of audio play-back.

     \param stream_length The number of tone samples to generate. Note that it
     is assumed that @p stream_length samples can be stored in @p stream.

     \param[out] stream A pointer to storage where the generated tone samples
     are written to.
   */
  void generateTone(const Frequency& sampling_frequency,
                    std::size_t stream_length,
                    Word* stream);

  friend class Mixer;
};

/**
   \brief Mix audio channels.

   A Mixer combines and scales multiple audio channels into a single audio
   stream.

   Mixers instantiate and have ownership over the audio channels that they mix.
   To modify the
   properties of a channel, access it via getChannel().
 */
class Mixer {

 public:
  /**
     \brief This is the maximum number of independent channels that can be
     mixed.

     \note It is possible to mix less than this many channels by simply keeping
     unwanted channels muted.
   */
  static const std::size_t NumChannels = 4u;

 private:
  std::vector<Channel> channels_;

 public:
  explicit Mixer() : channels_(NumChannels) {}

  /**
     \brief The number of audio channels that are currently non-muted.
   */
  std::size_t numNonMutedChannels() const;

  /**
     \brief Access and modify an audio channel.

     Channels are accessed via their zero-based index, with the maximum index
     being \f$\mbox{NumChannels} - 1\f$.

     If a channel is requested that doesn't exist, then an empty option value is
     returned.
   */
  optional<Channel&> getChannel(std::size_t channel_index);

  /**
     \brief Sample and mix channels, producing an audio stream.

     Each of the NumChannels audio channels is used to generate a tone, and then
     all the channel's tones are combined and scaled appropriately to produce a
     single audio stream.

     Each audio channel is scaled by the number of currently non-muted audio
     channels.

     \param stream_length The number of audio samples to generate.

     \param[out] stream The audio stream that is generated is written to this
     buffer. It is assumed that the destination is capable of storing @p
     stream_length samples.
   */
  void mixChannels(std::size_t stream_length, Word* stream);
};

/**
   \brief A physical audio play-back device.

   This is the means through which audio is actually played.
 */
class Device {
 private:
  unique_ptr<Mixer> mixer_;
  SDL_AudioSpec spec_{};
  SDL_AudioDeviceID id_{};

 public:
  /**
     \brief Create a new audio device.

     Each device has ownership over a mixer, which actually generates the audio
     stream that is fed to the device. It is through access to this mixer that
     each audio channel is configured.
   */
  explicit Device(unique_ptr<Mixer> mixer);

  ~Device() { SDL_CloseAudioDevice(id_); }

  /**
     \brief Get read-only access to the mixer.
     
     \see withMixer()
   */
  const Mixer& mixer() const { return *mixer_; }

  /**
     \brief Modify the mixer and its channels.

     Changes to the mixer and it's channels need to happen while the audio
     device is locked. This function locks the device,
     supplies the mixer with read and write access to a user-supplied function,
     and then unlocks the device at the termination of the function.
   */
  void withMixer(std::function<void(Mixer& mixer)> f);

  /**
     \brief Enable play-back on the device.

     Subsequent invocations of play() without first invoking pause() have no
     effect.

     \see pause()
   */
  void play() { SDL_PauseAudioDevice(id_, 0); }

  /**
     \brief Pause play-back on the device.

     Subsequence invocations of pause() without first invoking play() have no
     effect.

     \see play()
   */
  void pause() { SDL_PauseAudioDevice(id_, 1); }

  friend class DeviceLock;
};

/**
   \brief Lock the audio device to make modifications to the mixer.
   
   The lock is acquired when an instance of this class is instantiated, and the
   lock is released when the instance goes out of scope.

   \note This class should probably not be instantiated manually. Instead, it is
   better to make modifications to a device via Device::withMixer().
 */
class DeviceLock {
 private:
  SDL_AudioDeviceID id_;

 public:
  explicit DeviceLock(const Device& device) : id_{device.id_} {
    SDL_LockAudioDevice(id_);
  }
  explicit DeviceLock() = delete;
  explicit DeviceLock(const DeviceLock&) = delete;

  ~DeviceLock() { SDL_UnlockAudioDevice(id_); }
};

/**
   \brief Initialize the audio sub-systems. 

   A error::SdlError will be thrown in the event that the audio sub-systems
   cannot be initialized.

   There is no need to terminate the audio sub-systems at the program's
   completion.
 */
void initialize();

}  // namespace audio

}  // namespace nebula

#endif  // NEBULA_AUDIO_HPP_
