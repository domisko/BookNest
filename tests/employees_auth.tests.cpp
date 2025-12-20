// Tests: Mitarbeiter Auth/RBAC und Persistenz v3/v4 (aktuell v4)
#include <catch2/catch_test_macros.hpp>
#include "Library.h"

TEST_CASE("Employees auth, RBAC and persistence", "[employees][auth][persistence]") {
    Library lib;
    lib.setDataFilePath("test_employees_auth.bin");

    // Default-Admin sollte existieren: admin/admin
    REQUIRE(lib.authenticate("admin", "admin") == true);

    // Admin legt einen Mitarbeiter an
    int eid = lib.addEmployee("staff1", "Mitarbeiter Eins", Role::Staff, "pw1");
    REQUIRE(eid > 0);

    // Abmelden und als Mitarbeiter einloggen
    lib.logout();
    REQUIRE(lib.authenticate("staff1", "pw1") == true);

    // RBAC: Mitarbeiter darf keinen neuen Mitarbeiter anlegen
    int shouldFail = lib.addEmployee("hacker", "Hacker", Role::Staff, "x");
    REQUIRE(shouldFail <= 0);

    // Persistenz: Speichern, neu laden und prüfen, dass beide existieren
    REQUIRE(lib.saveData() == true);

    Library lib2;
    lib2.setDataFilePath("test_employees_auth.bin");
    REQUIRE(lib2.loadData() == true);

    // admin sollte sich wieder anmelden können
    REQUIRE(lib2.authenticate("admin", "admin") == true);

    // und staff1 ebenso
    lib2.logout();
    REQUIRE(lib2.authenticate("staff1", "pw1") == true);

    // Prüfen, dass mindestens 2 Mitarbeiter vorhanden sind
    REQUIRE(lib2.getEmployees().size() >= 2);
}
