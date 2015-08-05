/**
   \file graphics.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Simple 2D drawing and OpenGL renderng.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   This is presently an abstraction over SDL2, but the external inteface is
   independent of this underlying graphics library. Doing so hopefully allows
   for easier porting.

   For the simple 2D drawing interface, shapes are drawn to an internal canvas
   rather than the window itself. Invoking render() will render the canvas to
   the window and update the display.

   OpenGL version 2.1 and GLSL version 1.20 is supported (as a result of being
   limited by the implementation of Mesa on Unix-y platforms).
 */

#ifndef NEBULA_GRAPHICS_HPP_
#define NEBULA_GRAPHICS_HPP_

#pragma once

#include "graphics-units.hpp"
#include "prelude.hpp"
#include "sdl.hpp"

extern "C" {
#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL2/SDL_opengl.h>
}

#include <chrono>

DEFINE_LOGGER(GRAPHICS)

namespace nebula {

namespace error {

class OpenGLError : public std::runtime_error {
 public:
  explicit OpenGLError(const std::string& message)
      : std::runtime_error{message} {}
};

}  // namespace error

namespace graphics {

// 60 frames per second.
const std::chrono::microseconds kFramePeriod{16666u};

/**
   \brief The type of graphics that the window supports.

   Windows intended for 2D rendering cannot be used for 3D rendering, and
   vice-versa.
 */
enum class WindowType {
  TwoDimensional,
  ThreeDimensional
};

struct WindowDeleter {
  void operator()(SDL_Window* window) {
    if (window) {
      SDL_DestroyWindow(window);
    }
  }
};

using Window = unique_ptr<SDL_Window, WindowDeleter>;

/**
   \brief Initialize the graphics system.

   \note It is necessary to call this before using other functionality in this
   module.

   \see terminate()
 */
void initialize();

/**
   \brief Terminate the graphics system.

   Invoking this function cleans up any resources used by the graphics system.
 */
void terminate();

/**
   \brief Create a new graphical window.

   The window is created with the window-title \p title, a width of \p width
   phyiscal pixels and a height of \p height physical pixels.

   The \p window_type specifies whether the window is intended to render 2D or
   3D content.
 */
Window createWindow(const std::string& title,
                    units::Quantity<units::real::Length> width,
                    units::Quantity<units::real::Length> height,
                    WindowType window_type = WindowType::TwoDimensional);

/**
   \brief Type-safe representation of the red-component of a color.
 */
struct Red {
  std::uint8_t value;
  explicit Red(std::uint8_t value) : value{value} {}
};

/**
   \brief Type-safe representation of the green-component of a color.
 */
struct Green {
  std::uint8_t value;
  explicit Green(std::uint8_t value) : value{value} {}
};

/**
   \brief Type-safe representation of the blue-component of a color.
 */
struct Blue {
  std::uint8_t value;
  explicit Blue(std::uint8_t value) : value{value} {}
};

/**
   \brief A color in the RGB space.
 */
struct Color {
  Red red;
  Green green;
  Blue blue;

  explicit Color(Red red, Green green, Blue blue)
      : red{red}, green{green}, blue{blue} {}
};

const Color kColorWhite{Red{255u}, Green{255u}, Blue{255u}};
const Color kColorBlack{Red{0u}, Green{0u}, Blue{0u}};

/**
   \brief Set the drawing color of the window.

   Once a drawing color has been set, all subsequent shapes will be drawn with
   that color.
*/
void setDrawingColor(const Window& window, Color color);

/**
   \brief Fill the entire canvas with the current color.
 */
void clear(const Window& window);

/**
   \brief Draw a solid rectangle to the canvas.
 */
void drawRectangle(const Window& window,
                   units::Quantity<units::real::Length> origin_x,
                   units::Quantity<units::real::Length> origin_y,
                   units::Quantity<units::real::Length> width,
                   units::Quantity<units::real::Length> height);

/**
   \brief Render the window from the canvas.
 */
void render(const Window& window);

/**
   \brief Classes which want to render 2D graphics to a window.

   Nebula does all of its graphics rendering the main application thread, since
   that's the model supported by the underlying graphics library. Therefore, in
   order to support arbitrary classes that want to render windows, Nebula can
   call any class's IGraphics::renderGraphics() member in its graphics-rendering
   loop.
 */
class IGraphics {
 public:
  virtual void renderGraphics(const Window& window) = 0;
};

/**
   \brief 3D rendering via OpenGL.
 */
namespace gl {

/**
   \brief An OpenGL rendering context.

   Each context is tied to a Window.

   Only a single OpenGL context can be active at any one time. To switch to a
   Context, invoke Context::getWindow().
 */
class Context {
 private:
  SDL_GLContext context_;
  Window window_;

 public:
  /**
     \brief Create a new context.

     Once a Window has been associated with a Context, it cannot be used outside
     of that context.
   */
  explicit Context(Window window)
      : context_{SDL_GL_CreateContext(window.get())},
        window_{std::move(window)} {
    glewExperimental = GL_TRUE;
    glewInit();
  }

  virtual ~Context() { SDL_GL_DeleteContext(context_); }

  /**
     \brief Get the underlying Window associated with this context.
   */
  const Window& getWindow() { return window_; }

  /**
     \brief Set this Context as the active OpenGL rendering context.
   */
  inline void setActive() { SDL_GL_MakeCurrent(window_.get(), context_); }
};

/**
   \brief Render the OpenGL context to the screen.
 */
inline void swap(Context& context) {
  SDL_GL_SwapWindow(context.getWindow().get());
}

/**
   \brief Compile an OpenGL shader.

   This is mostly a conventient wrapper over the underlying OpenGL APIs which do
   the same thing. \p shader_source is the entire source code of the OpenGL
   shader to compile.

   Compilation errors are logged to the \p GRAPHICSS logging module.

   \throw error::OpenGLError When the shader sources can't be compiled.
 */
GLuint compileShader(GLenum shader_type, const std::string& shader_source);

/**
   \brief Link a sequences of compiled shaders into an OpenGL program.

   \throw error::OpenGLError When the shaders can't be linked.

   \see compileShader()
 */
template <typename ForwardIter>
GLuint linkShaders(ForwardIter begin, ForwardIter end) {
  GLint link_status;

  GLuint program = glCreateProgram();

  for (auto iter = begin; iter != end; ++iter) {
    glAttachShader(program, *iter);
  }

  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &link_status);
  if (link_status == GL_FALSE) {
    throw error::OpenGLError{"Failed to link shaders into program."};
  } else {
    LOG(GRAPHICS, info) << "Successfully linked program.";
  }

  return program;
}

/**
   \brief Classes which want to render 3D graphics to a window.

   Like graphics::IGraphics, but for 3D (OpenGL) graphics, except the
   initialization function initializeGL() is also required.
 */
class IGraphics {
 public:
  virtual void renderGL(Context& context) = 0;

  /**
     \brief Initialize OpenGL resources.

     This function is invoked by the main graphics rendering through prior to
     invoking renderGL. The implemening class can initiate any necessary
     resources here.
   */
  virtual void initializeGL(Context& context) = 0;
};

}  // namespace gl

}  // namespace graphics

}  // namespace nebula

#endif  // NEBULA_GRAPHICS_HPP_
