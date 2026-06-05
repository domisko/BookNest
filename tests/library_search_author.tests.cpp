#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include "Library.h"

TEST_CASE("searchBooks matches authors list (including non-primary)", "[search][author]") {
    Library lib;
    // Erstes Exemplar mit mehreren Autoren
    int id1 = lib.addBook("978-0-1", "Patterns in C++", std::vector<std::string>{"Gamma"}, "AW",
                          "GEN", "", "", 0.0, MediaType::Book);
    Book* b1 = lib.findBookByID(id1);
    REQUIRE(b1 != nullptr);
    b1->authors = {"Erich Gamma", "Richard Helm", "Ralph Johnson", "John Vlissides"};

    // Zweites Exemplar gleiche ISBN, anderer Standort/Autorensatz
    int id2 = lib.addBook("978-0-1", "Patterns in C++", std::vector<std::string>{"Gamma"}, "AW",
                          "GEN", "", "", 0.0, MediaType::Book);
    Book* b2 = lib.findBookByID(id2);
    REQUIRE(b2 != nullptr);
    b2->authors = {"Erich Gamma", "Richard Helm"};

    // Suche nach einem Autor, der NICHT im Feld "author" (primary) steht
    auto matches = lib.searchBooks("Vlissides");
    REQUIRE_FALSE(matches.empty());
    // id1 sollte dabei sein, id2 nicht
    bool has1 = std::find(matches.begin(), matches.end(), id1) != matches.end();
    bool has2 = std::find(matches.begin(), matches.end(), id2) != matches.end();
    REQUIRE(has1);
    REQUIRE_FALSE(has2);
}
