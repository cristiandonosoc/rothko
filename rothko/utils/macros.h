// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

// Define OS
#ifdef _WIN32
#define WINDOWS 1
#endif

#ifdef __linux__
#define LINUX 1
#endif

#ifdef __APPLE__
#define MACOS 1
#endif

#ifdef WINDOWS
#define FILEPATH_SEPARATOR '\\'
#else
#define FILEPATH_SEPARATOR '/'
#endif

#ifdef _MSC_VER
#define MSVC 1
#endif

#ifdef __GNUC__
#define GCC 1
#endif

#ifdef __clang__
#define CLANG 1
#endif

// [[nodiscard]]
#if defined(MSVC) && _MSC_VER >= 1911
#define NO_DISCARD [[nodiscard]]
#elif defined(GCC) || defined(CLANG)
#define NO_DISCARD [[nodiscard]]
#else
#define NO_DISCARD
#endif

// Hack to have variadic macros work with 0 arguments.
#define VA_ARGS(...) , ##__VA_ARGS__

#ifndef ABS
#define ABS(x) (((x) < 0) ? (x) : (-(x)))
#endif
#define U64_ALL_ONES() (uint64_t)-1

#define SEGFAULT()                 \
  do {                             \
    volatile char* __segfault = 0; \
    *__segfault = 1;               \
  } while (false)

// Join two macros into one string. Typical usage if for using __LINE__:
//
// const char* STRINGIFY(some_string_, __LINE__) = "bla";
//
// turns into:
//
// const char* some_string_22 = "bla";
#define STRINGIFY2(x, y) x##y
#define STRINGIFY(x, y) STRINGIFY2(x, y)

// DEBUG_MODE: Whether the binary is a debug build (-g in clang).
#ifndef DEBUG_MODE
#ifndef NDEBUG
#define DEBUG_MODE 1
#else
#define DEBUG_MODE 0
#endif
#endif

// Should only be used in array types. This won't work with a vector.
#define ARRAY_SIZE(array) (int)(sizeof((array)) / sizeof((array)[0]))

#define SWAP(a, b) \
  {                \
    auto tmp = a;  \
    a = b;         \
    b = tmp;       \
  }

// Combine permits to stringify arguments like __LINE__
#define COMBINE_INTERNAL(x, y) x##y
#define COMBINE(x, y) COMBINE_INTERNAL(x, y)

// Tell the compiler a function is using a printf-style format string.
// |format_param| is the one-based index of the format string parameter;
// |dots_param| is the one-based index of the "..." parameter.
// For v*printf functions (which take a va_list), pass 0 for dots_param.
// (This is undocumented but matches what the system C headers do.)
#if defined(__GNUC__) || defined(__clang__)
#define PRINTF_FORMAT(format_param, dots_param) \
  __attribute__((format(printf, format_param, dots_param)))
#else
#define PRINTF_FORMAT(format_param, dots_param)
#endif

// Ignore warnings for Windows because they don't have a good way
// of ignoring warnings for certain includes
// (I mean, they did add it in 2018...)
#ifdef _WIN32
#define BEGIN_IGNORE_WARNINGS() __pragma(warning(push, 0))
#define END_IGNORE_WARNINGS() __pragma(warning(pop))
#else
// Other compilers have decent ways of dealing with warnings on external code.
#define BEGIN_IGNORE_WARNINGS()
#define END_IGNORE_WARNINGS()
#endif

#if defined(_MSC_VER)
  #define __PRETTY_FUNCTION__ __FUNCTION__
#endif

// Constructors ----------------------------------------------------------------
//
// This macros are ways of shorthanding the annoying C++ constructos syntax into
// something quicker to write and (hopefully) easier to understand.

// Most resource holder objects have this constructor format,
// so a macro here make sense.
#define RAII_CONSTRUCTORS(class_name) \
  class_name() = default;             \
  ~class_name();                      \
  DELETE_COPY_AND_ASSIGN(class_name); \
  DEFAULT_MOVE_AND_ASSIGN(class_name);

#define DEFAULT_ALL_CONSTRUCTORS(class_name) \
  class_name() = default;                    \
  ~class_name() = default;                   \
  DEFAULT_COPY_AND_ASSIGN(class_name);       \
  DEFAULT_MOVE_AND_ASSIGN(class_name);

#define DECLARE_ALL_CONTRUCTORS(class_name) \
  class_name();                             \
  ~class_name();                            \
  DECLARE_COPY_AND_ASSIGN(class_name);      \
  DECLARE_MOVE_AND_ASSIGN(class_name);

#define DECLARE_CONSTRUCTOR(class_name) class_name();
#define DELETE_CONSTRUCTOR(class_name) class_name() = delete;
#define DEFAULT_CONSTRUCTOR(class_name) class_name() = default;

#define DECLARE_DESTRUCTOR(class_name) ~class_name();
#define DEFAULT_DESTRUCTOR(class_name) ~class_name() = default;

#define DECLARE_COPY_AND_ASSIGN(class_name) \
  class_name(const class_name&);            \
  class_name& operator=(const class_name&);

#define DEFAULT_COPY_AND_ASSIGN(class_name) \
  class_name(const class_name&) = default;  \
  class_name& operator=(const class_name&) = default;

#define DELETE_COPY_AND_ASSIGN(class_name) \
  class_name(const class_name&) = delete;  \
  class_name& operator=(const class_name&) = delete;

#define DECLARE_MOVE_AND_ASSIGN(class_name) \
  class_name(class_name&&);                 \
  class_name& operator=(class_name&&);

#define DEFAULT_MOVE_AND_ASSIGN(class_name) \
  class_name(class_name&&) = default;       \
  class_name& operator=(class_name&&) = default;

#define DELETE_MOVE_AND_ASSIGN(class_name) \
  class_name(class_name&&) = delete;       \
  class_name& operator=(class_name&&) = delete;
