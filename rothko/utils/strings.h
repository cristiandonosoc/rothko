// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

#include "rothko/utils/macros.h"

namespace rothko {

std::string StringPrintf(const char *fmt, ...) PRINTF_FORMAT(1, 2);
std::string StringPrintfV(const char *fmt, va_list);

std::string Concatenate(std::vector<std::string> strings);

std::string Trim(const std::string &input,
                 const std::string &chars_to_trim = "\t\r ");

std::vector<std::string>
SplitToLines(const std::string &input, const std::string &delimiters = "\n",
             const std::string &chars_to_trim = "\t\r ");

template <typename StringContainer>
std::string Join(const StringContainer& lines,
                 const std::string &separator = "") {
  // Add the size of the lines.
  size_t length = 0;
  for (auto &line : lines) {
    length += line.size();
  }
  if (separator.size() > 0)
    length += separator.size() * (lines.size() - 1);

  if (length == 0)
    return {};

  std::string result;
  result.reserve(length);

  bool first_line = true;
  for (auto &line : lines) {
    if (first_line) {
      first_line = false;
    } else {
      result.append(separator);
    }

    result.append(line.begin(), line.end());
  }

  return result;
}

} // namespace rothko
