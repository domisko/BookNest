# BookNest – Console Library Management System (C++17)

BookNest ist ein textbasiertes Bibliotheksverwaltungssystem, das Lernziele aus OOP, Binär‑Persistenz, Datum/Fristen, Suche, Reporting und einfacher Rollenverwaltung (Admin/Mitarbeitende) vereint.

## Features
- Medienmodell: `Book` mit Autorenliste, Publisher, Edition, Standort, Genre, Preis, `MediaType`, `maxLoanPeriodDays`, `createdAt`, `createdBy/lastModifiedBy`.
- Physische Exemplare: Eindeutige `inventoryID` je Exemplar; mehrere Exemplare pro ISBN sind möglich.
- Mitglieder (Borrower): Name, Email, Adresse, `registrationDate`, `status` (Active/Blocked).
- Historie: `Loan` speichert Ausleihe, Fälligkeit und Rückgabe – inkl. `performedBy` (Mitarbeiter:in, die die Aktion durchgeführt hat).
- Mitarbeiterverwaltung: Rollen `Admin`/`Staff`, Login/Logout, Anlegen/Deaktivieren/Reaktivieren, Passwort zurücksetzen (Demo‑Hash).
- Suche: Nach Titel, ISBN oder Autor(en).
- Reporting: Tagesbericht (Ausleihen/Rückgaben, sortiert, inkl. Summen pro `MediaType`), Fälligkeitsliste („Due Report“).
- Persistenz: Eine Binärdatei `library.bin` mit versioniertem Header und klaren Sektionen.
- UI: Reines Konsolenmenü mit „angeheftetem“ Bildschirm (Clear‑Screen zwischen Aktionen).

## Build & Tests (CLion / CMake)
- Profile: Debug (lokale Toolchain). Targets: `BookNest` (App), `unit_tests` (Catch2‑basierte Tests).
- Build & Tests (Beispiel):
  ```bash
  cmake --build cmake-build-debug --target unit_tests && ./cmake-build-debug/unit_tests
  ```
- Start der App:
  ```bash
  cmake --build cmake-build-debug --target BookNest && ./cmake-build-debug/BookNest
  ```

## Erste Schritte
1) App starten → Login: Standard‑Admin `admin`/`admin`.
2) Optional unter „Mitarbeiter verwalten“ weitere Konten anlegen.
3) Testdaten erzeugen über Menüpunkt `[8]`, danach Suche/Ausleihe/Rückgabe/Reports ausprobieren.

## Menüüberblick
- [1] Buch suchen (Titel/ISBN/Autor)
- [2] Buch ausleihen (Buch‑ID + Mitglieds‑ID)
- [3] Buch zurückgeben (Buch‑ID)
- [4] Alle Bücher anzeigen
- [5] Alle Mitglieder anzeigen (Status sichtbar)
- [6] Mitarbeiter verwalten (nur Admin)
- [7] Abmelden
- [8] Testdaten generieren
- [9] Beenden (Speichern)
- [10] Reports (Tagesbericht, Fälligkeitsliste)

Hinweis: In manchen IDE‑Run‑Konsolen bleibt der Scroll‑Verlauf sichtbar. Für eine „saubere“ angeheftete Darstellung bitte das System‑Terminal (macOS Terminal, iTerm2, Windows Terminal) nutzen.

## Datenmodell & Regeln
- Book: `maxLoanPeriodDays` steuert das Fälligkeitsdatum bei Ausleihe. `isAvailable` kennzeichnet den Status des Exemplars.
- Borrower: `status == Active` ist Voraussetzung für Ausleihe. `registrationDate` wird bei Anlage gesetzt.
- Loan: Enthält `loanDate`, `dueDate`, `returnDate` (0 = offen) und `performedBy`.
- Employee: `Role::Admin` vs. `Role::Staff`. Nur Admins verwalten Mitarbeiter.

## Reporting
- Tagesbericht (`Library::showDailyReport`):
  - Listet Ausleihen und Rückgaben eines Tages (Zeit, Medium, Borrower, `performedBy`, Fälligkeit).
  - Zusätzliche Summen pro `MediaType` und Gesamtsummen.
- Fälligkeitsliste (`Library::getDueReport`):
  - Offene Ausleihen, sortiert nach verbleibender Zeit (überfällige zuerst), limitierbar.

## Persistenzformat (`library.bin`)
Header:
- Magic: `BNES` (4 Bytes)
- Version: `uint32_t` (derzeit 5)

Sektionen (in Reihenfolge):
1. Books (Anzahl, dann Felder je Buch inkl. Autoren‑Vektor)
2. Borrowers (Anzahl, dann Felder inkl. `registrationDate` und `status`)
3. Employees (Anzahl, dann Felder inkl. Rolle, Aktiv‑Flag)
4. Loans (Anzahl, dann Felder inkl. `performedBy`)
5. Zähler (next IDs)

Alle Strings/Vektoren werden mit Längenfeld vorangeschrieben, wie in `Utils::writeString` implementiert.

## Sicherheit (Hinweis)
- Passwörter werden aus Demo‑Gründen nur mit einem sehr einfachen Hash gespeichert (`Utils::simpleHash`). Für Produktivsysteme unbedingt eine echte Passwort‑Hashing‑Bibliothek (z. B. bcrypt, argon2) verwenden.
- In der Entwicklung kann die Datei `library.bin` bei Schema‑Änderungen gelöscht werden (keine Abwärtskompatibilität notwendig).

## Tests
- Catch2‑basierte Unit‑Tests decken u. a. ab: Ausleihe/Rückgabe‑Flow, Fälligkeitsberechnung, Suche, Persistenz‑Roundtrip, Mitarbeiter‑Auth/RBAC, `performedBy`.

## Anwender‑Doku
- Siehe `docs/UserGuide.md` für einen Schritt‑für‑Schritt‑Leitfaden.

## Lizenz / Hinweis
Dieses Projekt ist Teil eines Studien‑Portfolios (DLBMINPAPCC01). Der Code dient Demonstrations‑ und Lernzwecken.