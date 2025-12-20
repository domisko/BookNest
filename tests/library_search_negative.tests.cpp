#include <catch2/catch_test_macros.hpp>
#include "Library.h"

TEST_CASE("Library search and negative cases", "[library][search][negative]") {
    Library lib;

    int b1 = lib.addBook("978-0001", "C++ Grundlagen", std::vector<std::string>{"A"}, "P",
                         "GEN", "", "", 0.0, MediaType::Book);
    int b2 = lib.addBook("978-0002", "Advanced C++", std::vector<std::string>{"B"}, "P",
                         "GEN", "", "", 0.0, MediaType::Book);
    int m1 = lib.addMember("Erika", "e@x", "Weg 1");

    // Suche nach Titelteil
    auto hits1 = lib.searchBooks("C++");
    REQUIRE(hits1.size() >= 2);

    // Suche nach exakter ISBN
    auto hits2 = lib.searchBooks("978-0002");
    REQUIRE(hits2.size() >= 1);
    bool containsB2 = false;
    for (int id : hits2) if (id == b2) containsB2 = true;
    REQUIRE(containsB2 == true);

    // Negativ: falsche IDs beim Ausleihen
    REQUIRE(lib.borrowBook(999999, m1) == false); // unbekanntes Buch
    REQUIRE(lib.borrowBook(b1, 999999) == false); // unbekanntes Mitglied

    // Buch korrekt ausleihen, dann zweites Mal schlagen
    REQUIRE(lib.borrowBook(b1, m1) == true);
    REQUIRE(lib.borrowBook(b1, m1) == false);

    // Negativ: Rückgabe eines nicht ausgeliehenen Buches
    REQUIRE(lib.returnBook(b2) == false);
}
