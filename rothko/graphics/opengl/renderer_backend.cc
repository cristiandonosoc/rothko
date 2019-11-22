// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/opengl/renderer_backend.h"

#include <GL/gl3w.h>

#include <memory>
#include <sstream>

#include "rothko/graphics/opengl/mesh.h"
#include "rothko/graphics/opengl/shader.h"
#include "rothko/graphics/opengl/texture.h"
#include "rothko/graphics/renderer.h"
#include "rothko/logging/logging.h"
#include "rothko/window/window.h"

namespace rothko {

using namespace opengl;

namespace opengl {

// Init --------------------------------------------------------------------------------------------

namespace {

#ifdef DEBUG_MODE

void APIENTRY OpenGLDebugCallback(GLenum source, GLenum type, GLuint id,
                                  GLenum severity, GLsizei length,
                                  const GLchar* message,
                                  const void* user_param) {
  (void)user_param;
  (void)length;
  if (type == GL_DEBUG_TYPE_OTHER)
    return;

  std::stringstream ss;

  ss << std::endl;
  ss << "---------------------opengl-callback-start---------------------" << std::endl;
  // TODO(Cristian): Add GLEnumToString
  (void)source;
  // ss << "Source: " << GLEnumToString(source) << std::endl;
  ss << "message: " << message << std::endl;
  ss << "type: ";
  switch (type) {
    case GL_DEBUG_TYPE_ERROR: ss << "ERROR"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ss << "DEPRECATED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: ss << "UNDEFINED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_PORTABILITY: ss << "PORTABILITY"; break;
    case GL_DEBUG_TYPE_PERFORMANCE: ss << "PERFORMANCE"; break;
    case GL_DEBUG_TYPE_OTHER: ss << "OTHER"; break;
  }
  ss << std::endl;

  ss << "id: " << id << std::endl;
  ss << "severity: ";
  switch (severity) {
    case GL_DEBUG_SEVERITY_LOW: ss << "LOW"; break;
    case GL_DEBUG_SEVERITY_MEDIUM: ss << "MEDIUM"; break;
    case GL_DEBUG_SEVERITY_HIGH: ss << "HIGH"; break;
    default: ss << "<unknown>"; break;
  }
  ss << std::endl;
  ss << "---------------------opengl-callback-end-----------------------" << std::endl;

  printf("%s", ss.str().c_str());

  // REALLY flush.
  fflush(stdout);
  fflush(stdout);
  fflush(stdout);
  fflush(stdout);
  fflush(stdout);
  if (severity == GL_DEBUG_SEVERITY_HIGH)
    exit(1);
}

#endif

const char*
Gl3wInitResultToString(int res) {
  switch (res) {
    case GL3W_OK: return "GL3W_OK";
    case GL3W_ERROR_INIT: return "GL3W_ERROR_INIT";
    case GL3W_ERROR_LIBRARY_OPEN: return "GL3W_ERROR_LIBRARY_OPEN";
    case GL3W_ERROR_OPENGL_VERSION: return "GL3W_ERROR_OPENGL_VERSION";
    default:
      break;
  }

  NOT_REACHED_MSG("Got unknown GL3W init result: %d", res);
  return "<unknown>";
}

std::unique_ptr<OpenGLRendererBackend> gBackend;

}  // namespace

OpenGLRendererBackend* GetOpenGL() {
  ASSERT(gBackend);
  return gBackend.get();
}

}  // opengl

std::unique_ptr<Renderer> InitRenderer() {
  ASSERT(!gBackend);

  int res = gl3wInit();
  if (res != GL3W_OK) {
    ERROR(OpenGL, "Got non-ok GL3W result: %s", Gl3wInitResultToString(res));
    return nullptr;
  }

  LOG(OpenGL, "Init gl3w");

#if DEBUG_MODE
  // Suscribe the debug messaging.
  if (glDebugMessageCallback) {
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLDebugCallback, nullptr);

    GLuint unused_ids = 0;
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                          &unused_ids, true);

    LOG(OpenGL, "Suscribed debug messaging.");
  }
#endif

  glLineWidth(3);
  glEnable(GL_PRIMITIVE_RESTART);
  glPrimitiveRestartIndex(line_strip::kPrimitiveReset);

  gBackend = std::make_unique<OpenGLRendererBackend>();

  auto renderer = std::make_unique<Renderer>();
  renderer->renderer_type = "OpenGL";
  return renderer;
}

void ShutdownRenderer() {
  ASSERT(gBackend);
  gBackend.reset();
}

// StartFrame --------------------------------------------------------------------------------------

void RendererStartFrame(Renderer*) {}

// EndFrame ----------------------------------------------------------------------------------------

namespace {

void ResetRendererState() {
  glDisable(GL_SCISSOR_TEST);
}

}  // namespace

void RendererEndFrame(Renderer*, Window* window) {
  /* auto* opengl = GetOpenGL(); */
  /* ASSERT(opengl->camera_index == -1);     // All cameras should be popped. */

  ResetRendererState();
  WindowSwapBuffers(window);
}

// Meshes ------------------------------------------------------------------------------------------

bool RendererStageMesh(Renderer*, Mesh* mesh) {
  return OpenGLStageMesh(gBackend.get(), mesh);
}

void RendererUnstageMesh(Renderer*, Mesh* mesh) {
  OpenGLUnstageMesh(gBackend.get(), mesh);
}

bool RendererUploadMeshRange(Renderer*, Mesh* mesh, Int2 vertex_range, Int2 index_range) {
  return OpenGLUploadMeshRange(gBackend.get(), mesh, vertex_range, index_range);
}

// Shaders -----------------------------------------------------------------------------------------

std::unique_ptr<Shader> RendererStageShader(Renderer*,
                                            const ShaderConfig& config,
                                            const std::string& vert_src,
                                            const std::string& frag_src) {
  return OpenGLStageShader(gBackend.get(), config, vert_src, frag_src);
}

void RendererUnstageShader(Renderer*, Shader* shader) {
  OpenGLUnstageShader(gBackend.get(), shader);
}

// Textures ----------------------------------------------------------------------------------------

bool RendererStageTexture(Renderer*, Texture* texture) {
  return OpenGLStageTexture(gBackend.get(), texture);
}

void RendererUnstageTexture(Renderer*, Texture* texture) {
  OpenGLUnstageTexture(gBackend.get(), texture);
}

void RendererSubTexture(Renderer*, Texture* texture, void* data, Int2 offset, Int2 range) {
  OpenGLSubTexture(gBackend.get(), texture, data, offset, range);
}

}  // namespace rothko
