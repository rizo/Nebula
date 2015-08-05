/**
   \file vector-display.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "vector-display.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>

namespace nebula {

static const std::string kVertexShaderSource{
    "#version 110\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "attribute vec3 position;\n"
    "attribute vec3 color;\n"
    "varying vec3 fragment_color;\n"
    "void main() {\n"
    "  fragment_color = color;\n"
    "  gl_Position = projection * view * model * vec4(position, 1.0);\n"
    "}\n"};

static const std::string kFragmentShaderSource{
    "#version 110\n"
    "varying vec3 fragment_color;\n"
    "void main() {"
    "  gl_FragColor = vec4(fragment_color, 1.0);\n"
    "}\n"};

static float fractionalPart(float x) { return x - std::floor(x); }

static bool isRotating(const VectorDisplayState& state) {
  return state.rotation_angle != state.rotation_angle_target;
}

void VectorDisplay::updateRotationAngle() {
  using std::chrono::duration_cast;

  // If there are no vertices queued, just rotate
  // constantly. Otherwise, only rotate to the target angle.
  if ((!isRotating(state_)) &&
      (state_.state_code != VectorDisplayState::StateCode::NoData)) {
    return;
  }

  auto now = std::chrono::system_clock::now();
  auto elapsed =
      duration_cast<std::chrono::milliseconds>(now - creation_time_).count();

  // See http://mathworld.wolfram.com/SawtoothWave.html
  //
  // A periodic saw-tooth wave of the variable x with period T and
  // amplitude A has the form
  //
  //   A * FractionalPart(x / T)
  //
  // where
  //
  //   A = 360 degrees of rotation.
  //
  //   T = The period of time it takes to traverse 360 degrees, in seconds.
  //       AKA 'ramp_period'.
  auto ramp_period = 360.0 / kVectorDisplayRotationDegreesPerSecond;
  auto argument = elapsed / ramp_period / 1000.0;
  state_.rotation_angle = 360.0 * fractionalPart(argument);
}

static const std::unordered_map<Word, VectorDisplayOperation> kOperationMap{
    {0u, VectorDisplayOperation::Poll},
    {1u, VectorDisplayOperation::MapVertexMemory},
    {2u, VectorDisplayOperation::SetRotation}};

void VectorDisplay::handleInterrupt(VectorDisplayOperation operation,
                                    ProcessorState& processor_state) {

  switch (operation) {
    case VectorDisplayOperation::Poll: {
      LOG(VECTOR, info) << "'Poll'";

      processor_state.write(Register::B, static_cast<Word>(state_.state_code));
      processor_state.write(Register::C, static_cast<Word>(state_.error_code));
    } break;

    case VectorDisplayOperation::MapVertexMemory: {
      LOG(VECTOR, info) << "'MapVertexMemory'";

      Word x = processor_state.read(Register::X);
      Word y = processor_state.read(Register::Y);

      state_.vertex_memory_offset = x;
      state_.num_vertices = y;

      if (state_.num_vertices != 0u) {
        if (isRotating(state_)) {
          state_.state_code = VectorDisplayState::StateCode::Rotating;
        } else {
          state_.state_code = VectorDisplayState::StateCode::Running;
        }

        LOG(VECTOR, info)
            << format("Rendering %u vertices from memory offset 0x%04x.") % y %
                   x;
      } else {
        state_.state_code = VectorDisplayState::StateCode::NoData;
        LOG(VECTOR, info) << "No vertices to render.";
      }
    } break;

    case VectorDisplayOperation::SetRotation: {
      LOG(VECTOR, info) << "'SetRotation'";

      Word x = processor_state.read(Register::X);
      state_.rotation_angle_target = x % 360;

      if (isRotating(state_)) {
        state_.state_code = VectorDisplayState::StateCode::Rotating;
      }

      LOG(VECTOR, info) << format("Set rotation angle target to %s deg.") %
                               state_.rotation_angle_target;
    } break;
  }
}

unique_ptr<VectorDisplayState> VectorDisplay::start() {
  notify();

  LOG(VECTOR, info) << "Started.";

  while (status() == SimulationStatus::Running) {
    auto now = std::chrono::system_clock::now();

    // Scope for lock guard.
    {
      std::lock_guard<std::mutex> lock{mutex_};

      if (interrupt_sink_.isActive()) {
        LOG(VECTOR, info) << "Got interrupt.";

        auto& processor_state = interrupt_sink_.processorState();
        auto a = processor_state.read(Register::A);
        auto operation = get(a, kOperationMap);

        if (operation) {
          handleInterrupt(*operation, processor_state);
        }

        interrupt_sink_.respond();
        LOG(VECTOR, info) << "Finishd handling interrupt.";
      }
    }  // end lock guard.

    std::this_thread::sleep_until(now + kVectorDisplayClockPeriod);
  }

  LOG(VECTOR, info) << "Shutting down.";

  return make_unique<VectorDisplayState>();
}

void VectorDisplay::initializeGL(graphics::gl::Context& context) {
  std::lock_guard<std::mutex> lock{mutex_};

  context.setActive();

  glGenBuffers(1, &gl_resources_.vertex_buffer);

  glGenBuffers(1, &gl_resources_.element_buffer);

  LOG(VECTOR, info) << "Compiling vertex shader.";
  gl_resources_.vertex_shader =
      graphics::gl::compileShader(GL_VERTEX_SHADER, kVertexShaderSource);

  LOG(VECTOR, info) << "Compiling fragment shader.";
  gl_resources_.fragment_shader =
      graphics::gl::compileShader(GL_FRAGMENT_SHADER, kFragmentShaderSource);

  std::vector<GLuint> shaders{gl_resources_.vertex_shader,
                              gl_resources_.fragment_shader};

  LOG(VECTOR, info) << "Linking shaders.";
  gl_resources_.program =
      graphics::gl::linkShaders(shaders.begin(), shaders.end());

  gl_resources_.position_attribute =
      glGetAttribLocation(gl_resources_.program, "position");

  gl_resources_.color_attribute =
      glGetAttribLocation(gl_resources_.program, "color");

  gl_resources_.model_uniform =
      glGetUniformLocation(gl_resources_.program, "model");

  gl_resources_.view_uniform =
      glGetUniformLocation(gl_resources_.program, "view");

  gl_resources_.projection_uniform =
      glGetUniformLocation(gl_resources_.program, "projection");

  // The "view" and "perspective" matrices are constant.

  // We're still on the same scale as the
  // "object" but now we can refer to points outside of it. So y =
  // -4.5 is looking at the object from afar.
  //
  // The camera is pointed along the y axis, so z now goes "up"
  // instead of out of the screen. The negative position along the y
  // axis of the camera refers to a person standing some distance from
  // the floating object, at eye level.

  gl_resources_.view =
      glm::lookAt(glm::vec3(0, -4.5, 0.0001),  // The position of the camera.
                  glm::vec3(0, 0, 0),   // The center position of the screen.
                  glm::vec3(0, 1, 0));  // the "up" unit vector of the camera.

  gl_resources_.projection =
      glm::perspective(45.0f,  // Field of view angle.
                       1.0f,   // Aspect ratio.
                       1.0f,  // Vertices closer to the "eye" than this distance
                              // are not rendered.
                       8.0f);  // Vertices further from the "eye" than this
                               // distance are not rendered.
}

static std::array<GLfloat, 3 * 8> kBorderVertices{
    {-1, -1, -1,  // Lower, back, left.
     1,  -1, -1,  // Lower, back, right.
     -1, 1,  -1,  // Upper, back, left.
     1,  1,  -1,  // Upper, back, right.
     -1, -1, 1,   // Lower, front, left.
     1,  -1, 1,   // Lower, front, right.
     -1, 1,  1,   // Upper, front, left.
     1,  1,  1,   // Upper, front, right.
    }};

static std::array<GLfloat, 3> kBorderColor{{0.5, 0.5, 0.5}};

static const std::array<GLushort, 2 * 12> kBorderElements{
    {0, 1, 0, 2, 0, 4, 5, 4, 5, 1, 5, 7, 6, 7, 6, 4, 6, 2, 3, 2, 3, 1, 3, 7}};

static const std::array<std::array<GLfloat, 3>, 4> kVertexColors{
    {{{0.1, 0.1, 0.1}},  // Black.
     {{1, 0, 0}},        // Red.
     {{0, 1, 0}},        // Green.
     {{0, 0, 1}},        // Blue.
    }};

template <typename InputIter, typename OutputIter>
OutputIter darkenVertexColor(InputIter color_begin,
                             InputIter color_end,
                             OutputIter output_color_begin) {
  return std::transform(color_begin,
                        color_end,
                        output_color_begin,
                        [](GLfloat component) { return component *= 0.5; });
}

bool VectorDisplay::isVertexVisible() {
  double proportion = 0.2 * state_.num_vertices / kVectorDisplayMaxVertices;
  std::bernoulli_distribution dist{proportion};
  return !dist(random_gen_);
}

std::vector<GLfloat> VectorDisplay::computeGLVertexAttributes() {
  std::vector<GLfloat> vertices;

  // First add the border.

  for (auto iter = kBorderVertices.begin(); iter != kBorderVertices.end();
       iter += 3) {
    vertices.push_back(*iter);
    vertices.push_back(*(iter + 1));
    vertices.push_back(*(iter + 2));

    // Add the color.
    vertices.insert(vertices.end(), kBorderColor.begin(), kBorderColor.end());
  }

  // Then add the program vertices.

  for (std::size_t vertex_index = 0u; vertex_index < state_.num_vertices;
       ++vertex_index) {
    Word offset1 = state_.vertex_memory_offset + (2u * vertex_index);
    Word offset2 = offset1 + 1u;

    LOG(VECTOR, debug) << format("Vertex %u at 0x%04x.") % vertex_index %
                              offset1;

    auto word1 = memory_->read(offset1);
    auto word2 = memory_->read(offset2);

    auto x = word1 & 0xff;
    auto y = (word1 >> 8) & 0xff;
    auto z = word2 & 0xff;
    auto color_index = (word2 >> 8) & 0x03;
    auto is_intense = static_cast<bool>((word2 >> 8) & 0x04);

    LOG(VECTOR, debug) << "x: " << x;
    LOG(VECTOR, debug) << "y: " << y;
    LOG(VECTOR, debug) << "z: " << z;
    LOG(VECTOR, debug) << "color: " << color_index;
    LOG(VECTOR, debug) << "intense?: " << is_intense;

    // Add the vertex coordinates, mapping
    // [0, 255] to [-1, 1].
    vertices.push_back((2 * x / 256.0) - 1.0);
    vertices.push_back((2 * y / 256.0) - 1.0);
    vertices.push_back((2 * z / 256.0) - 1.0);

    std::array<GLfloat, 3> color;
    if (isVertexVisible()) {
      color = kVertexColors[color_index];

      if (!is_intense) {
        darkenVertexColor(color.begin(), color.end(), color.begin());
      }
    } else {
      // Black is not very visible.
      color = kVertexColors[0];
      darkenVertexColor(color.begin(), color.end(), color.begin());
    }

    vertices.insert(vertices.end(), color.begin(), color.end());
  }

  return vertices;
}

void VectorDisplay::renderGL(graphics::gl::Context& context) {
  std::lock_guard<std::mutex> lock{mutex_};

  context.setActive();

  glUseProgram(gl_resources_.program);

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  auto vertex_attributes = computeGLVertexAttributes();

  glBindBuffer(GL_ARRAY_BUFFER, gl_resources_.vertex_buffer);

  glBufferData(GL_ARRAY_BUFFER,
               vertex_attributes.size() * sizeof(GLfloat),
               &vertex_attributes[0],
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_resources_.element_buffer);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               kBorderElements.size() * sizeof(GLushort),
               &kBorderElements[0],
               GL_STATIC_DRAW);

  // Compute the transformation matrix from the current rotation angle.
  updateRotationAngle();

  glm::mat4 model;
  model = glm::rotate(model,
                      glm::radians(static_cast<float>(state_.rotation_angle)),
                      glm::vec3(0.0, 0.0, 1.0));

  glUniformMatrix4fv(
      gl_resources_.model_uniform, 1, GL_FALSE, glm::value_ptr(model));

  glUniformMatrix4fv(gl_resources_.view_uniform,
                     1,
                     GL_FALSE,
                     glm::value_ptr(gl_resources_.view));

  glUniformMatrix4fv(gl_resources_.projection_uniform,
                     1,
                     GL_FALSE,
                     glm::value_ptr(gl_resources_.projection));

  glVertexAttribPointer(gl_resources_.position_attribute,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        6 * sizeof(GLfloat),
                        0);

  glVertexAttribPointer(gl_resources_.color_attribute,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        6 * sizeof(GLfloat),
                        reinterpret_cast<void*>(3 * sizeof(GLfloat)));

  glEnableVertexAttribArray(gl_resources_.position_attribute);
  glEnableVertexAttribArray(gl_resources_.color_attribute);

  // Now we actually draw the vertices which compose the border...
  glDrawElements(GL_LINES, kBorderElements.size(), GL_UNSIGNED_SHORT, nullptr);

  // ... and finally the user vertices, which do not have any
  // non-standard element ordering.  These vertices start after
  // 'KborderVertices' vertices in the array.
  glDrawArrays(GL_LINE_LOOP, kBorderVertices.size() / 3, state_.num_vertices);

  graphics::gl::swap(context);
}

}  // namespace nebula
