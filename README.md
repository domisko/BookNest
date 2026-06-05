# BookNest – Console Library Management System (C++17)

[![CI](https://github.com/domisko/BookNest/actions/workflows/ci.yml/badge.svg)](https://github.com/domisko/BookNest/actions/workflows/ci.yml)
[![Release](https://github.com/domisko/BookNest/actions/workflows/release.yml/badge.svg)](https://github.com/domisko/BookNest/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](#license--note)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus)

BookNest is a text-based library management system that combines learning objectives from OOP, binary persistence, dates/deadlines, search, reporting, CSV import, and simple role management (Admin/Staff).

## Demo

> Run it yourself in seconds — no build required:
> ```bash
> docker run -it --rm ghcr.io/domisko/booknest:latest
> ```
> Default login: `admin` / `admin`

<!-- demo.gif: record a short terminal session and drop it here -->
<!-- ![BookNest demo](docs/demo.gif) -->

## Features
- Media Model: `Book` with author list, publisher, edition, location, genre, price, `MediaType`, `maxLoanPeriodDays`, `createdAt`, `createdBy/lastModifiedBy`.
- Physical Copies: Unique `inventoryID` per copy; multiple copies per ISBN are possible.
- Members (Borrower): Name, Email, Address, `registrationDate`, `status` (Active/Blocked).
- History: `Loan` stores loan, due date, and return – including `performedBy` (the employee who performed the action).
- Employee Management: Roles `Admin`/`Staff`, login/logout, create/deactivate/reactivate, reset password (demo hash).
- Search: Books (Title/ISBN/Author) and members (ID/Email/Name) – both with paginated output (increments of 10, "m/+" for more).
- Reporting: Daily report (loans/returns, sorted, including totals per `MediaType`), due report.
- CSV Import: Books and members via `import/books.csv` or `import/members.csv`.
- Persistence & Autosave: Binary file `library.bin` (Version 5). Data is automatically saved after every mutating action.
- UI: Pure console menu with "pinned" screen (clear screen between actions).
- Benchmark Mode: Optional UI timer that shows the duration after actions (in ms). Toggleable in the "Settings" menu.

## Build & Tests (CLion / CMake)
- Profile: Debug (local toolchain). Targets: `BookNest` (App), `unit_tests` (Catch2-based tests).
- Build & Tests (Example):
  ```bash
  cmake --build cmake-build-debug --target unit_tests && ./cmake-build-debug/unit_tests
  ```
- Run the App:
  ```bash
  cmake --build cmake-build-debug --target BookNest && ./cmake-build-debug/BookNest
  ```

## Getting Started
1) Start the App → Login: Default admin `admin`/`admin`.
2) Optional: Create further accounts under "Employee (Admin)".
3) Import: As admin, select `[5] Import (Admin)` in the main menu and import `books.csv`/`members.csv` from `import/`.
4) Then try out search/loan/return/reports.

## Menu Overview (Simplified)
- Main Menu:
  - [1] Books
  - [2] Members
  - [3] Reports
  - [4] Employee (Admin)
  - [5] Import (Admin)
  - [0] Logout
  
  All menus display a consistent footer with `[0] Back` or `[0] Logout`.

Note: In some IDE run consoles, the scroll history remains visible. For a "clean" pinned display, please use the system terminal (macOS Terminal, iTerm2, Windows Terminal).

## Data Model & Rules
- Book: `maxLoanPeriodDays` controls the due date upon loan. `isAvailable` indicates the status of the copy.
- Borrower: `status == Active` is a prerequisite for borrowing. `registrationDate` is set upon creation.
- Loan: Contains `loanDate`, `dueDate`, `returnDate` (0 = open) and `performedBy`.
- Employee: `Role::Admin` vs. `Role::Staff`. Only admins manage employees.

## CSV Import – Schemas
Location of files (default): `import/books.csv` and `import/members.csv` (in project root). Fallback: `../import/...`.

### Books (`books.csv`)
- Required: `ISBN`; `Title`
- Optional: `Authors` (separated by `|`), `Publisher`, `Edition`, `Location`, `Genre`, `Price`, `MaxLoanDays`, `MediaType` (`Book/Buch`, `Magazine/Magazin/Zeitschrift`, `DVD`, `EBook/E-Book/E_Buch`, `Other/Sonstiges`), `CreatedBy`

Example:
```
ISBN;Title;Authors;Publisher;Edition;Location;Genre;Price;MaxLoanDays;MediaType;CreatedBy
978-3-16-148410-0;Clean Code;Robert C. Martin;Prentice Hall;;REG-A12;Programming;39.90;14;Book;domi
```

### Members (`members.csv`)
- Required: `Name`; `Email`; `Address`
- Optional: `RegistrationDate` (`YYYY-MM-DD`), `Status` (`Active/Blocked`)
  - If these fields are missing, they are automatically set (Registration Date = now, Status = Active).
  - Invalid values are ignored; default values remain.

Example:
```
Name;Email;Address
Max Mustermann;max@test.de;Musterweg 1
```

## Reporting
- Daily Report (`Library::showDailyReport`):
  - Lists loans and returns of a day (time, medium, borrower, `performedBy`, due date).
  - Additional totals per `MediaType` and grand totals.
- Due Report (`Library::getDueReport`):
  - Open loans, sorted by remaining time (overdue first), limitable.

## Persistence Format (`library.bin`)
Header:
- Magic: `BNES` (4 bytes)
- Version: `uint32_t` (currently 5)

Sections (in order):
1. Books (count, then fields per book including authors vector)
2. Borrowers (count, then fields including `registrationDate` and `status`)
3. Employees (count, then fields including role, active flag)
4. Loans (count, then fields including `performedBy`)
5. Counters (next IDs)

All strings/vectors are prefixed with a length field, as implemented in `Utils::writeString`.

## Security (Note)
- Passwords are saved with a very simple hash for demo purposes (`Utils::simpleHash`). For production systems, definitely use a real password hashing library (e.g., bcrypt, argon2).
- During development, the `library.bin` file can be deleted in case of schema changes (no backward compatibility required).

## Tests
- Catch2-based unit tests cover, among others: loan/return flow, due date calculation, search, persistence roundtrip, employee auth/RBAC, `performedBy`.

## Docker
With Docker, you can quickly test BookNest – including persistent data and CSV import via volume.

### Prerequisites
- Docker and optionally Docker Compose v2

### Build Image
```
docker build -t booknest:latest .
```

### Start via Docker Compose (Recommended)
For the best interactive experience (clean terminal), use:
```bash
# Create folders and run the app
mkdir -p data import
docker compose run --rm booknest
```
The `--rm` flag ensures that the container is removed after you exit, keeping your system clean.

If you prefer the standard way:
```bash
docker compose up --build
```
Note: `docker compose up` might prefix lines with `booknest-1 |`. Use `run` for a native feel.

Volumes:
- `./data` → `/app/data` (contains `library.bin`)
- `./import` → `/app/import` (place `books.csv`/`members.csv` here)

Log in as admin in the program (`admin/admin`) → Main Menu `[5] Import (Admin)`.

### Resetting the Application (Delete Data)
To reset the database (delete all imported books and members):
1. Stop any running container.
2. Delete the binary file: `rm data/library.bin`
3. Restart the app: `docker compose run --rm booknest`

## Benchmark Mode
- Activation: Main Menu → `[6] Settings` → "Toggle Benchmark". Status is displayed (ON/OFF).
- Output format: After relevant actions, a line like `(Books-Search in 12.3 ms)` appears.
- Measured actions include: loading/saving, book/member search, loan/return, reports, CSV import.

## User Documentation
- See `docs/UserGuide.md` for a step-by-step guide.

## License / Note
This project is part of a study portfolio (DLBMINPAPCC01). The code serves demonstration and learning purposes.

## Scaling & System Limits
- Standard operation uses linear searches over vectors with paginated output. In this configuration, 50k–100k books and a comparable number of members are well usable on typical hardware.
- For larger datasets, optional in-memory indices can be added (e.g., `unordered_map` for ID/ISBN/Email), allowing exact lookups in O(1). Startup time then increases once due to index construction (O(n)).
- Memory and I/O scale largely linearly with the number of records. For reliable tests, the Benchmark Mode (Settings) and CSV import of large amounts of data are recommended.