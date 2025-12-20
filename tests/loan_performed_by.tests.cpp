#include <catch2/catch_test_macros.hpp>
#include "Library.h"

TEST_CASE("Loan performedBy is set on borrow and updated on return", "[loan][audit]") {
    Library lib;
    lib.setDataFilePath("test_performed_by.bin");

    // Login als Default-Admin
    REQUIRE(lib.authenticate("admin", "admin"));

    int b = lib.addBook("X", "Y", std::vector<std::string>{"A"}, "P", "GEN", "", "", 0.0, MediaType::Book);
    int m = lib.addMember("Test", "t@e", "Addr");

    REQUIRE(lib.borrowBook(b, m));

    const auto& loans = lib.getLoans();
    REQUIRE_FALSE(loans.empty());
    auto l = loans.back();
    CHECK(l.performedBy == std::string("admin"));

    REQUIRE(lib.returnBook(b));
    const auto& loans2 = lib.getLoans();
    REQUIRE_FALSE(loans2.empty());
    CHECK(loans2.back().performedBy == std::string("admin"));
}
