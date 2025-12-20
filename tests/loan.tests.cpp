#include <catch2/catch_test_macros.hpp>
#include "Loan.h"
#include <chrono>

TEST_CASE("Loan defaults and due date ~ +14 days", "[loan]") {
    Loan l(1, 1000, 5);
    REQUIRE(l.loanID == 1);
    REQUIRE(l.bookInventoryID == 1000);
    REQUIRE(l.borrowerID == 5);
    REQUIRE(l.isReturned() == false);
    REQUIRE(l.returnDate == 0);

    // dueDate sollte ca. 14 Tage nach loanDate liegen (Toleranz ein paar Sekunden)
    auto diff = static_cast<long long>(l.dueDate - l.loanDate);
    REQUIRE(diff >= 14LL * 24 * 60 * 60 - 5);
    REQUIRE(diff <= 14LL * 24 * 60 * 60 + 5);
}
