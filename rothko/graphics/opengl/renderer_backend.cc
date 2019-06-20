// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/opengl/renderer_backend.h"

#include <GL/gl3w.h>

#include <memory>
#include <sstream>

#include "rothko/graphics/common/renderer.h"
#include "rothko/graphics/opengl/mesh.h"
#include "rothko/graphics/opengl/shader.h"
#include "rothko/graphics/opengl/texture.h"
#include "rothko/utils/logging.h"
#include "rothko/window/window.h"

namespace rothko {
namespace opengl {

// Backend suscription ---------------------------------------------------------

namespace {

std::unique_ptr<RendererBackend> CreateOpenGLRenderer() {
  return std::make_unique<OpenGLRendererBackend>();
}

struct BackendSuscriptor {
  BackendSuscriptor() {
    SuscribeRendererBackendFactory(RendererType::kOpenGL, CreateOpenGLRenderer);
  }
};

// Trigger the suscription.
BackendSuscriptor backend_suscriptor;

} // namespace

OpenGLRendererBackend::OpenGLRendererBackend() = default;

// Init ------------------------------------------------------------------------

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
  ss << "---------------------opengl-callback-start------------" << std::endl;
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
  ss << "---------------------opengl-callback-end--------------";

  LOG(ERROR, "%s", ss.str().c_str());
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

bool OpenGLInit(OpenGLRendererBackend* opengl, InitRendererConfig* config) {
  if (Valid(*opengl)) {
    NOT_REACHED_MSG("Backend should not be initialized twice.");
    return false;
  }

  int res = gl3wInit();
  if (res != GL3W_OK) {
    LOG(ERROR, "Got non-ok GL3W result: %s", Gl3wInitResultToString(res));
    return false;
  }

  LOG(DEBUG, "Init gl3w");

#if DEBUG_MODE
  // Suscribe the debug messaging.
  if (glDebugMessageCallback) {
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLDebugCallback, nullptr);

    GLuint unused_ids = 0;
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                          &unused_ids, true);

    LOG(DEBUG, "Suscribed debug messaging.");
  }
#endif

  opengl->window = config->window;
  return true;
}

}  // namespace

bool OpenGLRendererBackend::Init(Renderer*, InitRendererConfig* config) {
  return OpenGLInit(this, config);
}

// Shutdown --------------------------------------------------------------------

OpenGLRendererBackend::~OpenGLRendererBackend() = default;

// StartFrame ------------------------------------------------------------------

void OpenGLRendererBackend::StartFrame() {
  ASSERT(Valid(*this));

  glClearColor(0.3f, 0.4f, 0.8f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// EndFrame --------------------------------------------------------------------

void OpenGLRendererBackend::EndFrame() {
  ASSERT(Valid(*this));
  WindowSwapBuffers(this->window);
}

// Meshes ----------------------------------------------------------------------

bool OpenGLRendererBackend::StageMesh(Mesh* mesh) {
  return OpenGLStageMesh(this, mesh);
}

void OpenGLRendererBackend::UnstageMesh(Mesh*) {
  NOT_IMPLEMENTED();
}

// Shaders ---------------------------------------------------------------------

bool OpenGLRendererBackend::StageShader(Shader* shader) {
  return OpenGLStageShader(shader);
}

void OpenGLRendererBackend::UnstageShader(Shader*) {
  NOT_IMPLEMENTED();
}

// Textures --------------------------------------------------------------------

bool OpenGLRendererBackend::StageTexture(const StageTextureConfig& config,
                                         Texture* texture) {
  return OpenGLStageTexture(config, this, texture);
}

void OpenGLRendererBackend::UnstageTexture(Texture* texture) {
  OpenGLUnstageTexture(this, texture);
}

}  // namespace opengl
}  // namespace rothko
