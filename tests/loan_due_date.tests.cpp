#include <catch2/catch_test_macros.hpp>
#include "Library.h"
#include "Utils.h"

TEST_CASE("Due date respects Book::maxLoanPeriodDays", "[loan][due]") {
    Library lib;

    // Buch mit 7 Tagen Leihfrist
    int id1 = lib.addBook("111", "Kurzfrist", std::vector<std::string>{"A Autor"}, "Pub",
                          "GEN", "", "", 0.0, MediaType::Book);
    Book* b1 = lib.findBookByID(id1);
    REQUIRE(b1 != nullptr);
    b1->maxLoanPeriodDays = 7;

    // Buch mit 21 Tagen Leihfrist
    int id2 = lib.addBook("222", "Langfrist", std::vector<std::string>{"B Autor"}, "Pub",
                          "GEN", "", "", 0.0, MediaType::Book);
    Book* b2 = lib.findBookByID(id2);
    REQUIRE(b2 != nullptr);
    b2->maxLoanPeriodDays = 21;

    int member = lib.addMember("Tester", "t@e.st", "Addr");

    // Leihe 1
    REQUIRE(lib.borrowBook(id1, member));
    REQUIRE_FALSE(b1->isAvailable);
    const auto& loans1 = lib.getLoans();
    REQUIRE_FALSE(loans1.empty());
    const Loan& l1 = loans1.back();
    REQUIRE(l1.bookInventoryID == id1);
    // Delta in Sekunden muss exakt 7 Tage sein
    REQUIRE(l1.dueDate - l1.loanDate == 7 * 24 * 60 * 60);

    // Buch 1 zurückgeben, dann Buch 2 leihen
    REQUIRE(lib.returnBook(id1));
    REQUIRE(lib.borrowBook(id2, member));
    const auto& loans2 = lib.getLoans();
    REQUIRE(loans2.size() >= 2);
    const Loan& l2 = loans2.back();
    REQUIRE(l2.bookInventoryID == id2);
    REQUIRE(l2.dueDate - l2.loanDate == 21 * 24 * 60 * 60);
}
