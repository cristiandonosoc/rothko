// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#if defined(__clang__)

#define THREAD_ANNOTATION(x) __attribute__((x))

#else

#define THREAD_ANNOTATION(x)

#endif

#define GUARDED_BY(...) THREAD_ANNOTATION(guarded_by(__VA_ARGS__))
