#include <catch2/catch_test_macros.hpp>
#include "Utils.h"
#include <sstream>

TEST_CASE("writeString/readString round-trip", "[utils]") {
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    std::string in = "Hallo Welt! 123 äöü";
    writeString(ss, in);
    ss.seekg(0);
    auto out = readString(ss);
    REQUIRE(out == in);
}

TEST_CASE("addDays adds 2 days in seconds", "[utils]") {
    time_t t = 1000000; // fixed
    auto t2 = addDays(t, 2);
    REQUIRE(t2 - t == 2 * 24 * 60 * 60);
}

TEST_CASE("dateToString basic format", "[utils]") {
    time_t t = 1700000000; // feste Epoche
    auto s = dateToString(t);
    REQUIRE(s.size() == 10);
    REQUIRE(s[4] == '-');
    REQUIRE(s[7] == '-');
}
