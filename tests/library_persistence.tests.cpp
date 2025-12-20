#include <catch2/catch_test_macros.hpp>
#include "Library.h"
#include <cstdio>

static const char* kTempFile = "test_library.bin";

TEST_CASE("Library save/load with history and counters", "[library][persistence]") {
    // Aufräumen, falls vorhanden
    std::remove(kTempFile);

    Library lib;
    lib.setDataFilePath(kTempFile);

    int b1 = lib.addBook("111", "T1", std::vector<std::string>{"A1"}, "P1",
                         "GEN", "", "", 0.0, MediaType::Book);
    int b2 = lib.addBook("222", "T2", std::vector<std::string>{"A2"}, "P2",
                         "GEN", "", "", 0.0, MediaType::Book);
    int m1 = lib.addMember("Max", "max@x", "Addr");

    REQUIRE(lib.borrowBook(b1, m1) == true);
    REQUIRE(lib.saveData() == true);

    Library lib2;
    lib2.setDataFilePath(kTempFile);
    REQUIRE(lib2.loadData() == true);

    // Bestände
    REQUIRE(lib2.getBooks().size() == 2);
    REQUIRE(lib2.getBorrowers().size() == 1);
    REQUIRE(lib2.getLoans().size() == 1);

    // Offener Loan vorhanden
    REQUIRE(lib2.getLoans()[0].bookInventoryID == b1);
    REQUIRE(lib2.getLoans()[0].returnDate == 0);

    // Rückgabe, erneut speichern/laden
    REQUIRE(lib2.returnBook(b1) == true);
    REQUIRE(lib2.saveData() == true);

    Library lib3;
    lib3.setDataFilePath(kTempFile);
    REQUIRE(lib3.loadData() == true);
    REQUIRE(lib3.getLoans().size() == 1);
    REQUIRE(lib3.getLoans()[0].returnDate != 0);

    // ID-Zähler sollten fortgesetzt werden (neues Buch > letzter vergebener ID)
    int newBookId = lib3.addBook("333", "T3", std::vector<std::string>{"A3"}, "P3",
                                 "GEN", "", "", 0.0, MediaType::Book);
    REQUIRE(newBookId > b2);

    // Aufräumen
    std::remove(kTempFile);
}
