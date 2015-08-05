/**
   \file vector-display.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief A virtual holographic 3D display.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   \see /doc/hw/vector-display.txt
 */

#ifndef NEBULA_DEVICE_VECTOR_DISPLAY_HPP_
#define NEBULA_DEVICE_VECTOR_DISPLAY_HPP_

#pragma once

#include <platform/computer.hpp>
#include <platform/prelude.hpp>
#include <platform/graphics.hpp>
#include <platform/simulation.hpp>

#include <glm/glm.hpp>

#include <random>

DEFINE_LOGGER(VECTOR)

namespace nebula {

/**
   \brief The size of the real window showing the display.
 */
const auto kVectorDisplayResolution = 512_px;

/**
   \brief The rate of rotation of the display.
 */
const std::size_t kVectorDisplayRotationDegreesPerSecond = 50u;

/**
   \brief The maximum number of vertices that can be rendered.
 */
const Word kVectorDisplayMaxVertices = 128;

/**
   \brief the internal clock period of the display.

   This is effectively the resolution at which the display can process
   interrupts.
 */
const std::chrono::milliseconds kVectorDisplayClockPeriod{5u};

enum class VectorDisplayOperation {
  Poll,
  MapVertexMemory,
  SetRotation
};

struct VectorDisplayState {
  enum class StateCode : Word {
    NoData,
    Running,
    Rotating
  } state_code{StateCode::NoData};

  enum class ErrorCode : Word {
    None,
    Broken
  } error_code{ErrorCode::None};

  std::int16_t rotation_angle{0};
  std::int16_t rotation_angle_target{0};
  Word vertex_memory_offset{0};
  Word num_vertices{0};
};

class VectorDisplay : public Simulation<VectorDisplayState>,
                      public IDevice,
                      public graphics::gl::IGraphics {
 private:
  shared_ptr<Memory> memory_;
  InterruptSink interrupt_sink_;
  std::mutex mutex_{};
  std::chrono::time_point<std::chrono::system_clock> creation_time_;
  std::random_device random_device_{};
  std::mt19937 random_gen_;
  VectorDisplayState state_;

  struct GLResources {
    GLuint vertex_buffer{};
    GLuint element_buffer{};

    GLuint vertex_shader{};
    GLuint fragment_shader{};

    GLuint program{};

    GLint position_attribute{};
    GLint color_attribute{};

    GLint model_uniform{};
    GLint view_uniform{};
    GLint projection_uniform{};

    glm::mat4 view{};
    glm::mat4 projection{};
  } gl_resources_{};

  void updateRotationAngle();

  /**
     \brief Used to simulate flickering vertices.
   */
  bool isVertexVisible();

  std::vector<GLfloat> computeGLVertexAttributes();

  void handleInterrupt(VectorDisplayOperation operation,
                       ProcessorState& processor_state);

 public:
  explicit VectorDisplay(shared_ptr<Computer> computer,
                         shared_ptr<Memory> memory)
      : Simulation<VectorDisplayState>{},
        memory_{memory},
        interrupt_sink_{computer->registerDevice(deviceInfo())},
        creation_time_{std::chrono::system_clock::now()},
        random_gen_{random_device_()} {}

  unique_ptr<VectorDisplayState> start() override;

  void initializeGL(graphics::gl::Context& context) override;

  void renderGL(graphics::gl::Context& context) override;

  inline DeviceInfo deviceInfo() const override {
    return {DeviceID{0x42babf3c}, DeviceManufacturer{0x1eb37e91},
            DeviceVersion{0x0003}};
  }
};

}  // namespace nebula

#endif  // NEBULA_DEVICE_VECTOR_DISPLAY_HPP_
