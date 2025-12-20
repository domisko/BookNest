#include <catch2/catch_test_macros.hpp>
#include "Borrower.h"

TEST_CASE("Borrower constructor sets fields", "[borrower]") {
    Borrower m(7, "Max", "max@example.com", "Straße 1");
    REQUIRE(m.memberID == 7);
    REQUIRE(m.name == "Max");
    REQUIRE(m.email == "max@example.com");
    REQUIRE(m.address == "Straße 1");
}
