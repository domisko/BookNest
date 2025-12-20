#include <catch2/catch_test_macros.hpp>
#include "Library.h"

TEST_CASE("Blocked borrower cannot borrow; persistence keeps registrationDate/status", "[borrower][status][persistence]") {
    Library lib;
    lib.setDataFilePath("test_library.bin");

    int b = lib.addBook("X", "Titel", std::vector<std::string>{"Autor"}, "P", "GEN", "", "", 0.0, MediaType::Book);
    int m = lib.addMember("Blocky", "b@x", "Addr");

    // Borrower zuerst blockieren, indem wir direkt auf das Modell zugreifen (kein Admin-UI nötig)
    Borrower* br = lib.findBorrowerByID(m);
    REQUIRE(br != nullptr);
    br->status = BorrowerStatus::Blocked;

    // Versuch zu leihen muss fehlschlagen
    REQUIRE(lib.borrowBook(b, m) == false);

    // Registrierung sollte gesetzt sein (ungefähr jetzt)
    REQUIRE(br->registrationDate > 0);

    // Persistenz-Roundtrip
    REQUIRE(lib.saveData() == true);
    Library lib2;
    lib2.setDataFilePath("test_library.bin");
    REQUIRE(lib2.loadData() == true);

    const auto& borrowers = lib2.getBorrowers();
    REQUIRE(borrowers.size() >= 1);
    bool found = false;
    for (const auto& it : borrowers) {
        if (it.memberID == m) {
            found = true;
            REQUIRE(static_cast<int>(it.status) == static_cast<int>(BorrowerStatus::Blocked));
            REQUIRE(it.registrationDate > 0);
        }
    }
    REQUIRE(found == true);
}
