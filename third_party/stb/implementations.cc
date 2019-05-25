// This is glue for third party STB code. It is licensed with the same license
// as any other STB source code.
//
// Author: 2019, Cristián Donoso.

/**
 * STB libraries are created by Sean Barret (https://github.com/nothings).
 * They are single header libraries, in which the implementation is in the same
 * file as the header.
 * To get the declarations, simply include the header. The implementation is
 * obtained by defining the corresponding STB_XXXX_IMPLEMENTATION macro, which
 * will expand the header to include the declarations.
 *
 * This file includes all the implementations of the stb libraries imported by
 * the engine.
 *
 * NOTE: The STB libraries are *highly* configurable by overriding symbols with
 *       macros. Read each library for more documentation.
 */

// Import images.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Quick n' simple sprintf
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
