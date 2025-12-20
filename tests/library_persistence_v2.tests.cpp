// Tests für Persistenz v2 (Magic+Version; vollständiges Book-Layout)
#include <catch2/catch_test_macros.hpp>
#include "Library.h"
#include "Utils.h"
#include <cmath>

static bool approxTime(time_t a, time_t b, int toleranceSec = 2) {
    return std::llabs((long long)a - (long long)b) <= toleranceSec;
}

TEST_CASE("Persistence v2 roundtrip with extended Book fields", "[persistence][v2]") {
    Library lib;
    lib.setDataFilePath("test_v2_roundtrip.bin");

    // Bücher mit erweiterten Feldern (Minimalargs + Defaults)
    int id1 = lib.addBook("978-0-1", "Refactoring", std::vector<std::string>{"Martin Fowler"}, "Addison-Wesley",
                          "GEN", "", "", 0.0, MediaType::Book);
    int id2 = lib.addBook("978-0-2", "The Pragmatic Programmer", std::vector<std::string>{"Andrew Hunt"}, "Addison-Wesley",
                          "GEN", "", "", 0.0, MediaType::Book);

    // Felder nachträglich anreichern (Authors, MediaType, MaxLoanPeriod, Price ...)
    {
        Book* b1 = lib.findBookByID(id1);
        REQUIRE(b1);
        b1->authors = {"Martin Fowler", "Kent Beck"};
        b1->edition = "2nd";
        b1->location = "A1";
        b1->genre = "Software Engineering";
        b1->price = 49.95;
        b1->maxLoanPeriodDays = 21;
        b1->mediaType = MediaType::Book;
        b1->createdBy = "test";
        b1->lastModifiedBy = "test";
        b1->isAvailable = false; // markiert als ausgeliehen
    }
    {
        Book* b2 = lib.findBookByID(id2);
        REQUIRE(b2);
        b2->authors = {"Andrew Hunt", "David Thomas"};
        b2->edition = "20th Anniversary";
        b2->location = "B5";
        b2->genre = "Programming";
        b2->price = 39.90;
        b2->maxLoanPeriodDays = 7;
        b2->mediaType = MediaType::EBook;
        b2->createdBy = "test";
        b2->lastModifiedBy = "test";
        b2->isAvailable = true;
    }

    // Borrower + Loans
    int m1 = lib.addMember("Alice", "a@x", "Street 1");
    int m2 = lib.addMember("Bob", "b@y", "Street 2");

    // Einen Loan anlegen und sofort zurückgeben, damit returnDate != 0 persistiert wird
    REQUIRE(lib.borrowBook(id2, m1) == true);
    REQUIRE(lib.returnBook(id2) == true);

    // Speichern
    REQUIRE(lib.saveData() == true);

    // Neu laden in frische Instanz
    Library lib2;
    lib2.setDataFilePath("test_v2_roundtrip.bin");
    REQUIRE(lib2.loadData() == true);

    // Bücher prüfen
    const auto& books = lib2.getBooks();
    REQUIRE(books.size() == 2);

    const Book* rb1 = nullptr; const Book* rb2 = nullptr;
    for (const auto& b : books) {
        if (b.inventoryID == id1) rb1 = &b;
        if (b.inventoryID == id2) rb2 = &b;
    }
    REQUIRE(rb1 != nullptr);
    REQUIRE(rb2 != nullptr);

    // Buch 1
    CHECK(rb1->isbn == "978-0-1");
    CHECK(rb1->title == "Refactoring");
    REQUIRE(rb1->authors.size() == 2);
    CHECK(rb1->authors[0].find("Martin") != std::string::npos);
    CHECK(rb1->authors[1].find("Beck") != std::string::npos);
    CHECK(rb1->edition == "2nd");
    CHECK(rb1->location == "A1");
    CHECK(rb1->genre == "Software Engineering");
    CHECK(std::fabs(rb1->price - 49.95) < 1e-6);
    CHECK(rb1->maxLoanPeriodDays == 21);
    CHECK(static_cast<int>(rb1->mediaType) == static_cast<int>(MediaType::Book));
    CHECK(rb1->createdBy == "test");
    CHECK(rb1->lastModifiedBy == "test");
    CHECK(rb1->isAvailable == false);

    // Buch 2
    CHECK(rb2->isbn == "978-0-2");
    CHECK(rb2->title == "The Pragmatic Programmer");
    REQUIRE(rb2->authors.size() == 2);
    CHECK(rb2->authors[0].find("Andrew") != std::string::npos);
    CHECK(rb2->authors[1].find("Thomas") != std::string::npos);
    CHECK(rb2->edition == "20th Anniversary");
    CHECK(rb2->location == "B5");
    CHECK(rb2->genre == "Programming");
    CHECK(std::fabs(rb2->price - 39.90) < 1e-6);
    CHECK(rb2->maxLoanPeriodDays == 7);
    CHECK(static_cast<int>(rb2->mediaType) == static_cast<int>(MediaType::EBook));
    CHECK(rb2->createdBy == "test");
    CHECK(rb2->lastModifiedBy == "test");
    CHECK(rb2->isAvailable == true);

    // Borrower prüfen
    const auto& borrowers = lib2.getBorrowers();
    REQUIRE(borrowers.size() == 2);
    CHECK(borrowers[0].name.size() > 0);

    // Loans prüfen (eins mit returnDate gesetzt)
    const auto& loans = lib2.getLoans();
    REQUIRE(loans.size() >= 1);
    bool foundReturned = false;
    for (const auto& l : loans) {
        if (l.bookInventoryID == id2) {
            // loanDate <= dueDate logisch gegeben; returnDate != 0
            CHECK(l.dueDate >= l.loanDate);
            CHECK(l.returnDate != 0);
            foundReturned = true;
        }
    }
    CHECK(foundReturned == true);

    // Zähler prüfen: neue Buch-ID sollte nachladen fortgeführt werden
    int newBookId = lib2.addBook("978-0-3", "Clean Code", std::vector<std::string>{"Robert C. Martin"}, "Prentice Hall",
                                 "GEN", "", "", 0.0, MediaType::Book);
    CHECK(newBookId != id1);
    CHECK(newBookId != id2);
}

TEST_CASE("Loading non-existent file returns false (v2)", "[persistence][v2][negative]") {
    Library lib;
    lib.setDataFilePath("idontexist_anywhere.bin");
    REQUIRE(lib.loadData() == false);
}
