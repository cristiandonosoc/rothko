// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/utils/strings.h>

#include <third_party/catch2/catch.hpp>

namespace rothko {
namespace test {

TEST_CASE("Trim") {
  std::string result;

  /* result = Trim(""); */
  /* CHECK(result.empty()); */

  /* result = Trim("AAAA", "A"); */
  /* CHECK(result.empty()); */

  /* result = Trim("AAAA aaa bbb AAAAA", "A"); */
  /* CHECK(result == " aaa bbb "); */

  result = Trim("    Some string with spaces    ");
  CHECK(result == "Some string with spaces");

  /* result = Trim("\t\r\n another string ABCD \t\r\n", "\t\r\nABCD "); */
  /* CHECK(result == "another string"); */
}

namespace {

  const char kTestInput[] = "\r"
"\r"
"\t\t\rSome test that\n"
"has weird lines\n"
"\t\t\t\r\t\n"
"indentation.\n"
"o.O";

}  // namespace

TEST_CASE("SplitToLines") {
  SECTION("Normal split") {
    auto result = SplitToLines(kTestInput);

    REQUIRE(result.size() == 4u);
    CHECK(result[0] == "Some test that");
    CHECK(result[1] == "has weird lines");
    CHECK(result[2] == "indentation.");
    CHECK(result[3] == "o.O");
  }

  SECTION("Split by several characters") {
    auto result = SplitToLines(kTestInput, "\nwOl");

    REQUIRE(result.size() == 6u);
    CHECK(result[0] == "Some test that");
    CHECK(result[1] == "has");
    CHECK(result[2] == "eird");
    CHECK(result[3] == "ines");
    CHECK(result[4] == "indentation.");
    CHECK(result[5] == "o.");
  }
}

namespace {

std::vector<std::string> lines = {
  "Some lines ",
  "to be joined ",
  "together.",
  "    ",
  "... yeah!",
};

}  // namespace

TEST_CASE("Join") {
  SECTION("Without separator") {
    auto result = Join(lines);

    CHECK(result == "Some lines to be joined together.    ... yeah!");
  }

  SECTION("Some weird separator.") {
    auto result = Join(lines, "ABCD");
    CHECK(result ==
          "Some lines ABCDto be joined ABCDtogether.ABCD    ABCD... yeah!");
  }

}

}  // namespace test
}  // namespace rothko
