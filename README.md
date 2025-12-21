# BookNest – Console Library Management System (C++17)

BookNest ist ein textbasiertes Bibliotheksverwaltungssystem, das Lernziele aus OOP, Binär‑Persistenz, Datum/Fristen, Suche, Reporting, CSV‑Import und einfacher Rollenverwaltung (Admin/Mitarbeitende) vereint.

## Features
- Medienmodell: `Book` mit Autorenliste, Publisher, Edition, Standort, Genre, Preis, `MediaType`, `maxLoanPeriodDays`, `createdAt`, `createdBy/lastModifiedBy`.
- Physische Exemplare: Eindeutige `inventoryID` je Exemplar; mehrere Exemplare pro ISBN sind möglich.
- Mitglieder (Borrower): Name, Email, Adresse, `registrationDate`, `status` (Active/Blocked).
- Historie: `Loan` speichert Ausleihe, Fälligkeit und Rückgabe – inkl. `performedBy` (Mitarbeiter:in, die die Aktion durchgeführt hat).
- Mitarbeiterverwaltung: Rollen `Admin`/`Staff`, Login/Logout, Anlegen/Deaktivieren/Reaktivieren, Passwort zurücksetzen (Demo‑Hash).
- Suche: Bücher (Titel/ISBN/Autor) und Mitglieder (ID/Email/Name) – beide mit paginierter Ausgabe (10er‑Schritte, „m/+“ für mehr).
- Reporting: Tagesbericht (Ausleihen/Rückgaben, sortiert, inkl. Summen pro `MediaType`), Fälligkeitsliste („Due Report“).
- CSV‑Import: Bücher und Mitglieder per `import/books.csv` bzw. `import/members.csv`.
- Persistenz & Autosave: Binärdatei `library.bin` (Version 5). Nach jeder mutierenden Aktion wird automatisch gespeichert.
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
2) Optional unter „Mitarbeiter (Admin)“ weitere Konten anlegen.
3) Import: Als Admin im Hauptmenü `[5] Import (Admin)` wählen und `books.csv`/`members.csv` aus `import/` importieren.
4) Danach Suche/Ausleihe/Rückgabe/Reports ausprobieren.

## Menüüberblick (vereinfacht)
- Hauptmenü:
  - [1] Buecher
  - [2] Mitglieder
  - [3] Reports
  - [4] Mitarbeiter (Admin)
  - [5] Import (Admin)
  - [0] Abmelden
  
  In allen Menüs wird eine einheitliche Fußzeile mit `[0] Zurueck` bzw. `[0] Abmelden` angezeigt.

Hinweis: In manchen IDE‑Run‑Konsolen bleibt der Scroll‑Verlauf sichtbar. Für eine „saubere“ angeheftete Darstellung bitte das System‑Terminal (macOS Terminal, iTerm2, Windows Terminal) nutzen.

## Datenmodell & Regeln
- Book: `maxLoanPeriodDays` steuert das Fälligkeitsdatum bei Ausleihe. `isAvailable` kennzeichnet den Status des Exemplars.
- Borrower: `status == Active` ist Voraussetzung für Ausleihe. `registrationDate` wird bei Anlage gesetzt.
- Loan: Enthält `loanDate`, `dueDate`, `returnDate` (0 = offen) und `performedBy`.
- Employee: `Role::Admin` vs. `Role::Staff`. Nur Admins verwalten Mitarbeiter.

## CSV‑Import – Schemas
Ort der Dateien (Standard): `import/books.csv` und `import/members.csv` (im Projektroot). Fallback: `../import/...`.

### Bücher (`books.csv`)
- Pflicht: `ISBN`; `Title`
- Optional: `Authors` (Trennung per `|`), `Publisher`, `Edition`, `Location`, `Genre`, `Price`, `MaxLoanDays`, `MediaType` (`Book/Buch`, `Magazine/Magazin/Zeitschrift`, `DVD`, `EBook/E-Book/E_Buch`, `Other/Sonstiges`), `CreatedBy`

Beispiel:
```
ISBN;Title;Authors;Publisher;Edition;Location;Genre;Price;MaxLoanDays;MediaType;CreatedBy
978-3-16-148410-0;Clean Code;Robert C. Martin;Prentice Hall;;REG-A12;Programming;39.90;14;Book;domi
```

### Mitglieder (`members.csv`)
- Pflicht: `Name`; `Email`; `Address`
- Optional (derzeit ignoriert): `RegistrationDate` (`YYYY-MM-DD`), `Status` (`Active/Blocked`)

Beispiel:
```
Name;Email;Address
Max Mustermann;max@test.de;Musterweg 1
```

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

## Docker
Mit Docker kannst du BookNest schnell testen – inklusive persistenter Daten und CSV‑Import per Volume.

### Voraussetzungen
- Docker und optional Docker Compose v2

### Image bauen
```
docker build -t booknest:latest .
```

### Starten via Docker Compose (empfohlen)
```
mkdir -p data import
docker compose up --build
```
Dies startet die App interaktiv. Volumes:
- `./data` → `/app/data` (enthält `library.bin`)
- `./import` → `/app/import` (lege hier `books.csv`/`members.csv` ab)

Im Programm als Admin anmelden (`admin/admin`) → Hauptmenü `[5] Import (Admin)`.

### Direktstart ohne Compose
```
mkdir -p data import
docker run --rm -it \
  -v "$(pwd)/data:/app/data" \
  -v "$(pwd)/import:/app/import" \
  --name booknest booknest:latest
```

## Anwender‑Doku
- Siehe `docs/UserGuide.md` für einen Schritt‑für‑Schritt‑Leitfaden.

## Lizenz / Hinweis
Dieses Projekt ist Teil eines Studien‑Portfolios (DLBMINPAPCC01). Der Code dient Demonstrations‑ und Lernzwecken.