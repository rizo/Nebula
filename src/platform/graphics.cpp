/**
   \file graphics.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "graphics.hpp"

#include <array>
#include <cstdlib>

namespace nebula {

namespace graphics {

void initialize() {
  wrapSdl(SDL_Init(SDL_INIT_VIDEO), "SDL_Init");

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

  LOG(GRAPHICS, info) << "Initialized.";
}

void terminate() {
  SDL_Quit();
  LOG(GRAPHICS, info) << "Terminated.";
}

Window createWindow(const std::string& title,
                    units::Quantity<units::real::Length> width,
                    units::Quantity<units::real::Length> height,
                    WindowType window_type) {

  std::uint32_t window_flags{0u};

  if (window_type == WindowType::ThreeDimensional) {
    window_flags |= SDL_WINDOW_OPENGL;
  }

  auto window = Window(SDL_CreateWindow(title.c_str(),
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        width.value(),
                                        height.value(),
                                        window_flags));

  if (window_type == WindowType::TwoDimensional) {
    SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_SOFTWARE);
  }

  return window;
}

void setDrawingColor(const Window& window, Color color) {
  auto renderer = SDL_GetRenderer(window.get());
  wrapSdl(SDL_SetRenderDrawColor(renderer,
                              color.red.value,
                              color.green.value,
                              color.blue.value,
                              SDL_ALPHA_OPAQUE),
       "SDL_SetRenderDrawColor");
}

void clear(const Window& window) {
  wrapSdl(SDL_RenderClear(SDL_GetRenderer(window.get())), "SDL_RenderClear");
}

void drawRectangle(const Window& window,
                   units::Quantity<units::real::Length> origin_x,
                   units::Quantity<units::real::Length> origin_y,
                   units::Quantity<units::real::Length> width,
                   units::Quantity<units::real::Length> height) {
  auto renderer = SDL_GetRenderer(window.get());

  SDL_Rect rectangle;
  rectangle.x = origin_x.value();
  rectangle.y = origin_y.value();
  rectangle.w = width.value();
  rectangle.h = height.value();

  wrapSdl(SDL_RenderFillRect(renderer, &rectangle), "SDL_RenderFillRect");
}

void render(const Window& window) {
  SDL_RenderPresent(SDL_GetRenderer(window.get()));
}

namespace gl {

GLuint compileShader(GLenum shader_type, const std::string& shader_source) {
  GLuint shader = glCreateShader(shader_type);

  const GLchar* shader_source_array = shader_source.c_str();
  GLint shader_source_length{static_cast<GLint>(shader_source.size())};

  glShaderSource(shader, 1, &shader_source_array, &shader_source_length);
  glCompileShader(shader);

  // Check for compilation errors.
  GLint compilation_status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compilation_status);

  // Print the compilation log to the application log.
  std::array<char, 512u> compilation_log;
  GLsizei log_length;

  glGetShaderInfoLog(
      shader, compilation_log.size(), &log_length, &compilation_log.data()[0]);

  LOG(GRAPHICS, info) << "Shader compilation log:" << compilation_log.data();

  if (compilation_status == GL_FALSE) {
    throw error::OpenGLError{"Failed to compile shader."};
  } else {
    LOG(GRAPHICS, info) << "Shader successfully compiled.";
  }

  return shader;
}

}  // namespace gl

}  // namespace graphics

}  // namespace nebula
