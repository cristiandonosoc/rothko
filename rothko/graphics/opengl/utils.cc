// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/opengl/utils.h"

#include "rothko/utils/logging.h"

namespace rothko {
namespace opengl {

size_t ToSize(GLenum type) {
  switch (type) {
    case GL_FLOAT: return 1 * sizeof(GLfloat);
    case GL_FLOAT_VEC2: return 2 * sizeof(GLfloat);
    case GL_FLOAT_VEC3: return 3 * sizeof(GLfloat);
    case GL_FLOAT_VEC4: return 4 * sizeof(GLfloat);
    case GL_DOUBLE: return 1 * sizeof(GLdouble);
    case GL_DOUBLE_VEC2: return 2 * sizeof(GLdouble);
    case GL_DOUBLE_VEC3: return 3 * sizeof(GLdouble);
    case GL_DOUBLE_VEC4: return 4 * sizeof(GLdouble);
    case GL_INT: return 1 * sizeof(GLint);
    case GL_INT_VEC2: return 2 * sizeof(GLint);
    case GL_INT_VEC3: return 3 * sizeof(GLint);
    case GL_INT_VEC4: return 4 * sizeof(GLint);
    case GL_UNSIGNED_INT: return 1 * sizeof(GLuint);
    case GL_UNSIGNED_INT_VEC2: return 2 * sizeof(GLuint);
    case GL_UNSIGNED_INT_VEC3: return 3 * sizeof(GLuint);
    case GL_UNSIGNED_INT_VEC4: return 4 * sizeof(GLuint);
    case GL_BOOL: return 1 * sizeof(GLboolean);
    case GL_BOOL_VEC2: return 2 * sizeof(GLboolean);
    case GL_BOOL_VEC3: return 3 * sizeof(GLboolean);
    case GL_BOOL_VEC4: return 4 * sizeof(GLboolean);
    case GL_FLOAT_MAT2: return 2 * 2 * sizeof(GLfloat);
    case GL_FLOAT_MAT3: return 3 * 3 * sizeof(GLfloat);
    case GL_FLOAT_MAT4: return 4 * 4 * sizeof(GLfloat);
    case GL_FLOAT_MAT2x3: return 2 * 3 * sizeof(GLfloat);
    case GL_FLOAT_MAT2x4: return 2 * 4 * sizeof(GLfloat);
    case GL_FLOAT_MAT3x2: return 3 * 2 * sizeof(GLfloat);
    case GL_FLOAT_MAT3x4: return 3 * 4 * sizeof(GLfloat);
    case GL_FLOAT_MAT4x2: return 4 * 2 * sizeof(GLfloat);
    case GL_FLOAT_MAT4x3: return 4 * 3 * sizeof(GLfloat);
    case GL_DOUBLE_MAT2: return 2 * 2 * sizeof(GLdouble);
    case GL_DOUBLE_MAT3: return 3 * 3 * sizeof(GLdouble);
    case GL_DOUBLE_MAT4: return 4 * 4 * sizeof(GLdouble);
    case GL_DOUBLE_MAT2x3: return 2 * 3 * sizeof(GLdouble);
    case GL_DOUBLE_MAT2x4: return 2 * 4 * sizeof(GLdouble);
    case GL_DOUBLE_MAT3x2: return 3 * 2 * sizeof(GLdouble);
    case GL_DOUBLE_MAT3x4: return 3 * 4 * sizeof(GLdouble);
    case GL_DOUBLE_MAT4x2: return 4 * 2 * sizeof(GLdouble);
    case GL_DOUBLE_MAT4x3: return 4 * 3 * sizeof(GLdouble);
    case GL_SAMPLER_1D: return 1 * sizeof(uint32_t);
    case GL_SAMPLER_2D: return 1 * sizeof(uint32_t);
    case GL_SAMPLER_3D: return 1 * sizeof(uint32_t);
    case GL_RGBA: return 4 * sizeof(uint8_t);
    case GL_SAMPLER_2D_ARRAY: return 1 * sizeof(uint32_t);

#ifdef GLENUM_TO_SIZE_COMPLETE_TYPES_AS_NEEDED
    case GL_SAMPLER_CUBE: return;
    case GL_SAMPLER_1D_SHADOW: return;
    case GL_SAMPLER_2D_SHADOW: return;
    case GL_SAMPLER_1D_ARRAY: return;
    case GL_SAMPLER_1D_ARRAY_SHADOW: return;
    case GL_SAMPLER_2D_ARRAY_SHADOW: return;
    case GL_SAMPLER_2D_MULTISAMPLE: return;
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY: return;
    case GL_SAMPLER_CUBE_SHADOW: return;
    case GL_SAMPLER_BUFFER: return;
    case GL_SAMPLER_2D_RECT: return;
    case GL_SAMPLER_2D_RECT_SHADOW: return;
    case GL_INT_SAMPLER_1D: return;
    case GL_INT_SAMPLER_2D: return;
    case GL_INT_SAMPLER_3D: return;
    case GL_INT_SAMPLER_CUBE: return;
    case GL_INT_SAMPLER_1D_ARRAY: return;
    case GL_INT_SAMPLER_2D_ARRAY: return;
    case GL_INT_SAMPLER_2D_MULTISAMPLE: return;
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return;
    case GL_INT_SAMPLER_BUFFER: return;
    case GL_INT_SAMPLER_2D_RECT: return;
    case GL_UNSIGNED_INT_SAMPLER_1D: return;
    case GL_UNSIGNED_INT_SAMPLER_2D: return;
    case GL_UNSIGNED_INT_SAMPLER_3D: return;
    case GL_UNSIGNED_INT_SAMPLER_CUBE: return;
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY: return;
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: return;
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE: return;
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return;
    case GL_UNSIGNED_INT_SAMPLER_BUFFER: return;
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT: return;
    case GL_IMAGE_1D: return;
    case GL_IMAGE_2D: return;
    case GL_IMAGE_3D: return;
    case GL_IMAGE_2D_RECT: return;
    case GL_IMAGE_CUBE: return;
    case GL_IMAGE_BUFFER: return;
    case GL_IMAGE_1D_ARRAY: return;
    case GL_IMAGE_2D_ARRAY: return;
    case GL_IMAGE_2D_MULTISAMPLE: return;
    case GL_IMAGE_2D_MULTISAMPLE_ARRAY: return;
    case GL_INT_IMAGE_1D: return;
    case GL_INT_IMAGE_2D: return;
    case GL_INT_IMAGE_3D: return;
    case GL_INT_IMAGE_2D_RECT: return;
    case GL_INT_IMAGE_CUBE: return;
    case GL_INT_IMAGE_BUFFER: return;
    case GL_INT_IMAGE_1D_ARRAY: return;
    case GL_INT_IMAGE_2D_ARRAY: return;
    case GL_INT_IMAGE_2D_MULTISAMPLE: return;
    case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY: return;
    case GL_UNSIGNED_INT_IMAGE_1D: return;
    case GL_UNSIGNED_INT_IMAGE_2D: return;
    case GL_UNSIGNED_INT_IMAGE_3D: return;
    case GL_UNSIGNED_INT_IMAGE_2D_RECT: return;
    case GL_UNSIGNED_INT_IMAGE_CUBE: return;
    case GL_UNSIGNED_INT_IMAGE_BUFFER: return;
    case GL_UNSIGNED_INT_IMAGE_1D_ARRAY: return;
    case GL_UNSIGNED_INT_IMAGE_2D_ARRAY: return;
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE: return;
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY: return;
    case GL_UNSIGNED_INT_ATOMIC_COUNTER: return;
#endif

    default:
      NOT_REACHED();
      return 0;
  }
}

const char* ToString(GLenum type) {
  switch (type) {
    case GL_FLOAT: return "GL_FLOAT";
    case GL_FLOAT_VEC2: return "GL_FLOAT_VEC2";
    case GL_FLOAT_VEC3: return "GL_FLOAT_VEC3";
    case GL_FLOAT_VEC4: return "GL_FLOAT_VEC4";
    case GL_DOUBLE: return "GL_DOUBLE";
    case GL_DOUBLE_VEC2: return "GL_DOUBLE_VEC2";
    case GL_DOUBLE_VEC3: return "GL_DOUBLE_VEC3";
    case GL_DOUBLE_VEC4: return "GL_DOUBLE_VEC4";
    case GL_INT: return "GL_INT";
    case GL_INT_VEC2: return "GL_INT_VEC2";
    case GL_INT_VEC3: return "GL_INT_VEC3";
    case GL_INT_VEC4: return "GL_INT_VEC4";
    case GL_UNSIGNED_INT: return "GL_UNSIGNED_INT";
    case GL_UNSIGNED_INT_VEC2: return "GL_UNSIGNED_INT_VEC2";
    case GL_UNSIGNED_INT_VEC3: return "GL_UNSIGNED_INT_VEC3";
    case GL_UNSIGNED_INT_VEC4: return "GL_UNSIGNED_INT_VEC4";
    case GL_BOOL: return "GL_BOOL";
    case GL_BOOL_VEC2: return "GL_BOOL_VEC2";
    case GL_BOOL_VEC3: return "GL_BOOL_VEC3";
    case GL_BOOL_VEC4: return "GL_BOOL_VEC4";
    case GL_FLOAT_MAT2: return "GL_FLOAT_MAT2";
    case GL_FLOAT_MAT3: return "GL_FLOAT_MAT3";
    case GL_FLOAT_MAT4: return "GL_FLOAT_MAT4";
    case GL_FLOAT_MAT2x3: return "GL_FLOAT_MAT2x3";
    case GL_FLOAT_MAT2x4: return "GL_FLOAT_MAT2x4";
    case GL_FLOAT_MAT3x2: return "GL_FLOAT_MAT3x2";
    case GL_FLOAT_MAT3x4: return "GL_FLOAT_MAT3x4";
    case GL_FLOAT_MAT4x2: return "GL_FLOAT_MAT4x2";
    case GL_FLOAT_MAT4x3: return "GL_FLOAT_MAT4x3";
    case GL_DOUBLE_MAT2: return "GL_DOUBLE_MAT2";
    case GL_DOUBLE_MAT3: return "GL_DOUBLE_MAT3";
    case GL_DOUBLE_MAT4: return "GL_DOUBLE_MAT4";
    case GL_DOUBLE_MAT2x3: return "GL_DOUBLE_MAT2x3";
    case GL_DOUBLE_MAT2x4: return "GL_DOUBLE_MAT2x4";
    case GL_DOUBLE_MAT3x2: return "GL_DOUBLE_MAT3x2";
    case GL_DOUBLE_MAT3x4: return "GL_DOUBLE_MAT3x4";
    case GL_DOUBLE_MAT4x2: return "GL_DOUBLE_MAT4x2";
    case GL_DOUBLE_MAT4x3: return "GL_DOUBLE_MAT4x3";
    case GL_SAMPLER_1D: return "GL_SAMPLER_1D";
    case GL_SAMPLER_2D: return "GL_SAMPLER_2D";
    case GL_SAMPLER_3D: return "GL_SAMPLER_3D";
    case GL_SAMPLER_CUBE: return "GL_SAMPLER_CUBE";
    case GL_SAMPLER_1D_SHADOW: return "GL_SAMPLER_1D_SHADOW";
    case GL_SAMPLER_2D_SHADOW: return "GL_SAMPLER_2D_SHADOW";
    case GL_SAMPLER_1D_ARRAY: return "GL_SAMPLER_1D_ARRAY";
    case GL_SAMPLER_2D_ARRAY: return "GL_SAMPLER_2D_ARRAY";
    case GL_SAMPLER_1D_ARRAY_SHADOW: return "GL_SAMPLER_1D_ARRAY_SHADOW";
    case GL_SAMPLER_2D_ARRAY_SHADOW: return "GL_SAMPLER_2D_ARRAY_SHADOW";
    case GL_SAMPLER_2D_MULTISAMPLE: return "GL_SAMPLER_2D_MULTISAMPLE";
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
      return "GL_SAMPLER_2D_MULTISAMPLE_ARRAY";
    case GL_SAMPLER_CUBE_SHADOW: return "GL_SAMPLER_CUBE_SHADOW";
    case GL_SAMPLER_BUFFER: return "GL_SAMPLER_BUFFER";
    case GL_SAMPLER_2D_RECT: return "GL_SAMPLER_2D_RECT";
    case GL_SAMPLER_2D_RECT_SHADOW: return "GL_SAMPLER_2D_RECT_SHADOW";
    case GL_INT_SAMPLER_1D: return "GL_INT_SAMPLER_1D";
    case GL_INT_SAMPLER_2D: return "GL_INT_SAMPLER_2D";
    case GL_INT_SAMPLER_3D: return "GL_INT_SAMPLER_3D";
    case GL_INT_SAMPLER_CUBE: return "GL_INT_SAMPLER_CUBE";
    case GL_INT_SAMPLER_1D_ARRAY: return "GL_INT_SAMPLER_1D_ARRAY";
    case GL_INT_SAMPLER_2D_ARRAY: return "GL_INT_SAMPLER_2D_ARRAY";
    case GL_INT_SAMPLER_2D_MULTISAMPLE: return "GL_INT_SAMPLER_2D_MULTISAMPLE";
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
      return "GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY";
    case GL_INT_SAMPLER_BUFFER: return "GL_INT_SAMPLER_BUFFER";
    case GL_INT_SAMPLER_2D_RECT: return "GL_INT_SAMPLER_2D_RECT";
    case GL_UNSIGNED_INT_SAMPLER_1D: return "GL_UNSIGNED_INT_SAMPLER_1D";
    case GL_UNSIGNED_INT_SAMPLER_2D: return "GL_UNSIGNED_INT_SAMPLER_2D";
    case GL_UNSIGNED_INT_SAMPLER_3D: return "GL_UNSIGNED_INT_SAMPLER_3D";
    case GL_UNSIGNED_INT_SAMPLER_CUBE: return "GL_UNSIGNED_INT_SAMPLER_CUBE";
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
      return "GL_UNSIGNED_INT_SAMPLER_1D_ARRAY";
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
      return "GL_UNSIGNED_INT_SAMPLER_2D_ARRAY";
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
      return "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE";
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
      return "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY";
    case GL_UNSIGNED_INT_SAMPLER_BUFFER:
      return "GL_UNSIGNED_INT_SAMPLER_BUFFER";
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
      return "GL_UNSIGNED_INT_SAMPLER_2D_RECT";
    case GL_IMAGE_1D: return "GL_IMAGE_1D";
    case GL_IMAGE_2D: return "GL_IMAGE_2D";
    case GL_IMAGE_3D: return "GL_IMAGE_3D";
    case GL_IMAGE_2D_RECT: return "GL_IMAGE_2D_RECT";
    case GL_IMAGE_CUBE: return "GL_IMAGE_CUBE";
    case GL_IMAGE_BUFFER: return "GL_IMAGE_BUFFER";
    case GL_IMAGE_1D_ARRAY: return "GL_IMAGE_1D_ARRAY";
    case GL_IMAGE_2D_ARRAY: return "GL_IMAGE_2D_ARRAY";
    case GL_IMAGE_2D_MULTISAMPLE: return "GL_IMAGE_2D_MULTISAMPLE";
    case GL_IMAGE_2D_MULTISAMPLE_ARRAY: return "GL_IMAGE_2D_MULTISAMPLE_ARRAY";
    case GL_INT_IMAGE_1D: return "GL_INT_IMAGE_1D";
    case GL_INT_IMAGE_2D: return "GL_INT_IMAGE_2D";
    case GL_INT_IMAGE_3D: return "GL_INT_IMAGE_3D";
    case GL_INT_IMAGE_2D_RECT: return "GL_INT_IMAGE_2D_RECT";
    case GL_INT_IMAGE_CUBE: return "GL_INT_IMAGE_CUBE";
    case GL_INT_IMAGE_BUFFER: return "GL_INT_IMAGE_BUFFER";
    case GL_INT_IMAGE_1D_ARRAY: return "GL_INT_IMAGE_1D_ARRAY";
    case GL_INT_IMAGE_2D_ARRAY: return "GL_INT_IMAGE_2D_ARRAY";
    case GL_INT_IMAGE_2D_MULTISAMPLE: return "GL_INT_IMAGE_2D_MULTISAMPLE";
    case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
      return "GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY";
    case GL_UNSIGNED_INT_IMAGE_1D: return "GL_UNSIGNED_INT_IMAGE_1D";
    case GL_UNSIGNED_INT_IMAGE_2D: return "GL_UNSIGNED_INT_IMAGE_2D";
    case GL_UNSIGNED_INT_IMAGE_3D: return "GL_UNSIGNED_INT_IMAGE_3D";
    case GL_UNSIGNED_INT_IMAGE_2D_RECT: return "GL_UNSIGNED_INT_IMAGE_2D_RECT";
    case GL_UNSIGNED_INT_IMAGE_CUBE: return "GL_UNSIGNED_INT_IMAGE_CUBE";
    case GL_UNSIGNED_INT_IMAGE_BUFFER: return "GL_UNSIGNED_INT_IMAGE_BUFFER";
    case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
      return "GL_UNSIGNED_INT_IMAGE_1D_ARRAY";
    case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
      return "GL_UNSIGNED_INT_IMAGE_2D_ARRAY";
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
      return "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE";
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
      return "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY";
    case GL_UNSIGNED_INT_ATOMIC_COUNTER:
      return "GL_UNSIGNED_INT_ATOMIC_COUNTER";
    case GL_VERTEX_SHADER: return "GL_VERTEX_SHADER";
    case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
    case GL_LINK_STATUS: return "GL_LINK_STATUS";

    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return "GL_INVALID_FRAMEBUFFER_OPERATION";

    case GL_DEPTH_BUFFER_BIT: return "GL_DEPTH_BUFFER_BIT";
    case GL_STENCIL_BUFFER_BIT: return "GL_STENCIL_BUFFER_BIT";
    case GL_COLOR_BUFFER_BIT: return "GL_COLOR_BUFFER_BIT";
    case GL_FALSE: return "GL_FALSE";
    case GL_TRUE: return "GL_TRUE";
    /* case GL_POINTS: return "GL_POINTS"; */
    /* case GL_LINES: return "GL_LINES"; */
    case GL_LINE_LOOP: return "GL_LINE_LOOP";
    case GL_LINE_STRIP: return "GL_LINE_STRIP";
    case GL_TRIANGLES: return "GL_TRIANGLES";
    case GL_TRIANGLE_STRIP: return "GL_TRIANGLE_STRIP";
    case GL_TRIANGLE_FAN: return "GL_TRIANGLE_FAN";
    case GL_QUADS: return "GL_QUADS";
    case GL_NEVER: return "GL_NEVER";
    case GL_LESS: return "GL_LESS";
    case GL_EQUAL: return "GL_EQUAL";
    case GL_LEQUAL: return "GL_LEQUAL";
    case GL_GREATER: return "GL_GREATER";
    case GL_NOTEQUAL: return "GL_NOTEQUAL";
    case GL_GEQUAL: return "GL_GEQUAL";
    case GL_ALWAYS: return "GL_ALWAYS";
    /* case GL_ZERO: return "GL_ZERO"; */
    /* case GL_ONE: return "GL_ONE"; */
    case GL_SRC_COLOR: return "GL_SRC_COLOR";
    case GL_ONE_MINUS_SRC_COLOR: return "GL_ONE_MINUS_SRC_COLOR";
    case GL_SRC_ALPHA: return "GL_SRC_ALPHA";
    case GL_ONE_MINUS_SRC_ALPHA: return "GL_ONE_MINUS_SRC_ALPHA";
    case GL_DST_ALPHA: return "GL_DST_ALPHA";
    case GL_ONE_MINUS_DST_ALPHA: return "GL_ONE_MINUS_DST_ALPHA";
    case GL_DST_COLOR: return "GL_DST_COLOR";
    case GL_ONE_MINUS_DST_COLOR: return "GL_ONE_MINUS_DST_COLOR";
    case GL_SRC_ALPHA_SATURATE: return "GL_SRC_ALPHA_SATURATE";
    /* case GL_NONE: return "GL_NONE"; */
    /* case GL_FRONT_LEFT: return "GL_FRONT_LEFT"; */
    case GL_FRONT_RIGHT: return "GL_FRONT_RIGHT";
    case GL_BACK_LEFT: return "GL_BACK_LEFT";
    case GL_BACK_RIGHT: return "GL_BACK_RIGHT";
    case GL_FRONT: return "GL_FRONT";
    case GL_BACK: return "GL_BACK";
    case GL_LEFT: return "GL_LEFT";
    case GL_RIGHT: return "GL_RIGHT";
    case GL_FRONT_AND_BACK: return "GL_FRONT_AND_BACK";
    /* case GL_NO_ERROR: return "GL_NO_ERROR"; */
    case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
    case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
    case GL_CW: return "GL_CW";
    case GL_CCW: return "GL_CCW";
    case GL_POINT_SIZE: return "GL_POINT_SIZE";
    case GL_POINT_SIZE_RANGE: return "GL_POINT_SIZE_RANGE";
    case GL_POINT_SIZE_GRANULARITY: return "GL_POINT_SIZE_GRANULARITY";
    case GL_LINE_SMOOTH: return "GL_LINE_SMOOTH";
    case GL_LINE_WIDTH: return "GL_LINE_WIDTH";
    case GL_LINE_WIDTH_RANGE: return "GL_LINE_WIDTH_RANGE";
    case GL_LINE_WIDTH_GRANULARITY: return "GL_LINE_WIDTH_GRANULARITY";
    case GL_POLYGON_MODE: return "GL_POLYGON_MODE";
    case GL_POLYGON_SMOOTH: return "GL_POLYGON_SMOOTH";
    case GL_CULL_FACE: return "GL_CULL_FACE";
    case GL_CULL_FACE_MODE: return "GL_CULL_FACE_MODE";
    case GL_FRONT_FACE: return "GL_FRONT_FACE";
    case GL_DEPTH_RANGE: return "GL_DEPTH_RANGE";
    case GL_DEPTH_TEST: return "GL_DEPTH_TEST";
    case GL_DEPTH_WRITEMASK: return "GL_DEPTH_WRITEMASK";
    case GL_DEPTH_CLEAR_VALUE: return "GL_DEPTH_CLEAR_VALUE";
    case GL_DEPTH_FUNC: return "GL_DEPTH_FUNC";
    case GL_STENCIL_TEST: return "GL_STENCIL_TEST";
    case GL_STENCIL_CLEAR_VALUE: return "GL_STENCIL_CLEAR_VALUE";
    case GL_STENCIL_FUNC: return "GL_STENCIL_FUNC";
    case GL_STENCIL_VALUE_MASK: return "GL_STENCIL_VALUE_MASK";
    case GL_STENCIL_FAIL: return "GL_STENCIL_FAIL";
    case GL_STENCIL_PASS_DEPTH_FAIL: return "GL_STENCIL_PASS_DEPTH_FAIL";
    case GL_STENCIL_PASS_DEPTH_PASS: return "GL_STENCIL_PASS_DEPTH_PASS";
    case GL_STENCIL_REF: return "GL_STENCIL_REF";
    case GL_STENCIL_WRITEMASK: return "GL_STENCIL_WRITEMASK";
    case GL_VIEWPORT: return "GL_VIEWPORT";
    case GL_DITHER: return "GL_DITHER";
    case GL_BLEND_DST: return "GL_BLEND_DST";
    case GL_BLEND_SRC: return "GL_BLEND_SRC";
    case GL_BLEND: return "GL_BLEND";
    case GL_LOGIC_OP_MODE: return "GL_LOGIC_OP_MODE";
    case GL_DRAW_BUFFER: return "GL_DRAW_BUFFER";
    case GL_READ_BUFFER: return "GL_READ_BUFFER";
    case GL_SCISSOR_BOX: return "GL_SCISSOR_BOX";
    case GL_SCISSOR_TEST: return "GL_SCISSOR_TEST";
    case GL_COLOR_CLEAR_VALUE: return "GL_COLOR_CLEAR_VALUE";
    case GL_COLOR_WRITEMASK: return "GL_COLOR_WRITEMASK";
    case GL_DOUBLEBUFFER: return "GL_DOUBLEBUFFER";
    case GL_STEREO: return "GL_STEREO";
    case GL_LINE_SMOOTH_HINT: return "GL_LINE_SMOOTH_HINT";
    case GL_POLYGON_SMOOTH_HINT: return "GL_POLYGON_SMOOTH_HINT";
    case GL_UNPACK_SWAP_BYTES: return "GL_UNPACK_SWAP_BYTES";
    case GL_UNPACK_LSB_FIRST: return "GL_UNPACK_LSB_FIRST";
    case GL_UNPACK_ROW_LENGTH: return "GL_UNPACK_ROW_LENGTH";
    case GL_UNPACK_SKIP_ROWS: return "GL_UNPACK_SKIP_ROWS";
    case GL_UNPACK_SKIP_PIXELS: return "GL_UNPACK_SKIP_PIXELS";
    case GL_UNPACK_ALIGNMENT: return "GL_UNPACK_ALIGNMENT";
    case GL_PACK_SWAP_BYTES: return "GL_PACK_SWAP_BYTES";
    case GL_PACK_LSB_FIRST: return "GL_PACK_LSB_FIRST";
    case GL_PACK_ROW_LENGTH: return "GL_PACK_ROW_LENGTH";
    case GL_PACK_SKIP_ROWS: return "GL_PACK_SKIP_ROWS";
    case GL_PACK_SKIP_PIXELS: return "GL_PACK_SKIP_PIXELS";
    case GL_PACK_ALIGNMENT: return "GL_PACK_ALIGNMENT";
    case GL_MAX_TEXTURE_SIZE: return "GL_MAX_TEXTURE_SIZE";
    case GL_MAX_VIEWPORT_DIMS: return "GL_MAX_VIEWPORT_DIMS";
    case GL_SUBPIXEL_BITS: return "GL_SUBPIXEL_BITS";
    case GL_TEXTURE_1D: return "GL_TEXTURE_1D";
    case GL_TEXTURE_2D: return "GL_TEXTURE_2D";
    case GL_TEXTURE_WIDTH: return "GL_TEXTURE_WIDTH";
    case GL_TEXTURE_HEIGHT: return "GL_TEXTURE_HEIGHT";
    case GL_TEXTURE_BORDER_COLOR: return "GL_TEXTURE_BORDER_COLOR";
    case GL_DONT_CARE: return "GL_DONT_CARE";
    case GL_FASTEST: return "GL_FASTEST";
    case GL_NICEST: return "GL_NICEST";
    case GL_BYTE: return "GL_BYTE";
    case GL_UNSIGNED_BYTE: return "GL_UNSIGNED_BYTE";
    case GL_SHORT: return "GL_SHORT";
    case GL_UNSIGNED_SHORT: return "GL_UNSIGNED_SHORT";
    case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
    case GL_CLEAR: return "GL_CLEAR";
    case GL_AND: return "GL_AND";
    case GL_AND_REVERSE: return "GL_AND_REVERSE";
    case GL_COPY: return "GL_COPY";
    case GL_AND_INVERTED: return "GL_AND_INVERTED";
    case GL_NOOP: return "GL_NOOP";
    case GL_XOR: return "GL_XOR";
    case GL_OR: return "GL_OR";
    case GL_NOR: return "GL_NOR";
    case GL_EQUIV: return "GL_EQUIV";
    case GL_INVERT: return "GL_INVERT";
    case GL_OR_REVERSE: return "GL_OR_REVERSE";
    case GL_COPY_INVERTED: return "GL_COPY_INVERTED";
    case GL_OR_INVERTED: return "GL_OR_INVERTED";
    case GL_NAND: return "GL_NAND";
    case GL_SET: return "GL_SET";
    case GL_TEXTURE: return "GL_TEXTURE";
    case GL_COLOR: return "GL_COLOR";
    case GL_DEPTH: return "GL_DEPTH";
    case GL_STENCIL: return "GL_STENCIL";
    case GL_STENCIL_INDEX: return "GL_STENCIL_INDEX";
    case GL_DEPTH_COMPONENT: return "GL_DEPTH_COMPONENT";
    case GL_RED: return "GL_RED";
    case GL_GREEN: return "GL_GREEN";
    case GL_BLUE: return "GL_BLUE";
    case GL_ALPHA: return "GL_ALPHA";
    case GL_RGB: return "GL_RGB";
    case GL_RGBA: return "GL_RGBA";
    case GL_POINT: return "GL_POINT";
    case GL_LINE: return "GL_LINE";
    case GL_FILL: return "GL_FILL";
    case GL_KEEP: return "GL_KEEP";
    case GL_REPLACE: return "GL_REPLACE";
    case GL_INCR: return "GL_INCR";
    case GL_DECR: return "GL_DECR";
    case GL_VENDOR: return "GL_VENDOR";
    case GL_RENDERER: return "GL_RENDERER";
    case GL_VERSION: return "GL_VERSION";
    case GL_EXTENSIONS: return "GL_EXTENSIONS";
    case GL_NEAREST: return "GL_NEAREST";
    case GL_LINEAR: return "GL_LINEAR";
    case GL_NEAREST_MIPMAP_NEAREST: return "GL_NEAREST_MIPMAP_NEAREST";
    case GL_LINEAR_MIPMAP_NEAREST: return "GL_LINEAR_MIPMAP_NEAREST";
    case GL_NEAREST_MIPMAP_LINEAR: return "GL_NEAREST_MIPMAP_LINEAR";
    case GL_LINEAR_MIPMAP_LINEAR: return "GL_LINEAR_MIPMAP_LINEAR";
    case GL_TEXTURE_MAG_FILTER: return "GL_TEXTURE_MAG_FILTER";
    case GL_TEXTURE_MIN_FILTER: return "GL_TEXTURE_MIN_FILTER";
    case GL_TEXTURE_WRAP_S: return "GL_TEXTURE_WRAP_S";
    case GL_TEXTURE_WRAP_T: return "GL_TEXTURE_WRAP_T";
    case GL_REPEAT: return "GL_REPEAT";

    case GL_DEBUG_SOURCE_API: return "GL_DEBUG_SOURCE_API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "GL_DEBUG_SOURCE_WINDOW_SYSTEM";
    case GL_DEBUG_SOURCE_SHADER_COMPILER: return "GL_DEBUG_SOURCE_SHADER_COMPILER";
    case GL_DEBUG_SOURCE_THIRD_PARTY: return "GL_DEBUG_SOURCE_THIRD_PARTY";
    case GL_DEBUG_SOURCE_APPLICATION: return "GL_DEBUG_SOURCE_APPLICATION";
    case GL_DEBUG_SOURCE_OTHER: return "GL_DEBUG_SOURCE_OTHER";
    /* case GL_DONT_CARE: return "GL_DONT_CARE"; */

    default:
      LOG(WARNING, "Uncovered GLenum type: %u", (uint32_t)type);
      return "<unknown>";
  }
}

}  // namespace opengl
}  // namespace rothko
