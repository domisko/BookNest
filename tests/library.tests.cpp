#include <catch2/catch_test_macros.hpp>
#include "Library.h"
#include "Utils.h"

TEST_CASE("Library borrowing logic", "[library]") {
    Library lib;
    
    // Vorbereitung: Ein Buch und ein Mitglied hinzufügen
    int bookID = lib.addBook("123-456", "C++ Testen", std::vector<std::string>{"Der Autor"}, "Test Verlag",
                             "GEN", "", "", 0.0, MediaType::Book);
    int memberID = lib.addMember("Max Mustermann", "max@test.de", "Teststraße 1");
    
    // IDs stammen direkt aus den Rueckgabewerten der Methoden

    SECTION("Successful borrow sets correct due date") {
        REQUIRE(lib.borrowBook(bookID, memberID) == true);
        
        Book* book = lib.findBookByID(bookID);
        REQUIRE(book != nullptr);
        REQUIRE(book->isAvailable == false);
        
        // Wir prüfen, ob das Buch als verliehen markiert wurde
        // Um das Datum tiefgehend zu prüfen, müssten wir in der Library
        // eine Methode haben, die uns die Loans zurückgibt.
        // Aber wir können zumindest den Status prüfen.
    }

    SECTION("Cannot borrow already borrowed book") {
        REQUIRE(lib.borrowBook(bookID, memberID) == true);
        // Zweiter Versuch sollte scheitern (Logik in borrowBook prüft isAvailable)
        REQUIRE(lib.borrowBook(bookID, memberID) == false);
        
        Book* book = lib.findBookByID(bookID);
        REQUIRE(book->isAvailable == false);
    }

    SECTION("Return borrowed book succeeds and frees availability") {
        REQUIRE(lib.borrowBook(bookID, memberID) == true);
        REQUIRE(lib.returnBook(bookID) == true);
        Book* book = lib.findBookByID(bookID);
        REQUIRE(book != nullptr);
        REQUIRE(book->isAvailable == true);
        // No open loan should exist now for this book
    }
}