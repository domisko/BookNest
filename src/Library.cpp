#include "Library.h"
#include "Utils.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <map>
#include <filesystem>
#include <cctype>

// Optional: Hilfsfunktionen intern
static const Employee* findEmployeeByUsername(const std::vector<Employee>& list, const std::string& uname) {
    for (const auto& e : list) if (e.username == uname) return &e;
    return nullptr;
}

// --- Konstruktor: sorgt für Default-Admin, wenn noch keiner existiert ---
// Hinweis: Persistenz kann Mitarbeiter überschreiben; der Default-Admin wird nur für frische Instanzen erzeugt
// und später durch loadData() ersetzt.

int Library::addBook(const std::string& isbn,
                     const std::string& title,
                     const std::vector<std::string>& authors,
                     const std::string& publisher,
                     const std::string& location,
                     const std::string& edition,
                     const std::string& genre,
                     double price,
                     MediaType mediaType,
                     int maxLoanPeriodDays,
                     std::time_t createdAt,
                     const std::string& createdBy,
                     const std::string& lastModifiedBy,
                     bool isAvailable) {
    // Stempelung mit aktuellem User (falls eingeloggt)
    std::string actor = createdBy;
    if (currentUserID != -1) {
        for (const auto& e : employees) if (e.employeeID == currentUserID) { actor = e.username; break; }
    }
    std::string actorMod = lastModifiedBy;
    if (currentUserID != -1) {
        for (const auto& e : employees) if (e.employeeID == currentUserID) { actorMod = e.username; break; }
    }
    int assignedId = nextBookID++;
    books.emplace_back(assignedId, isbn, title, authors, publisher, location, edition, genre,
                       price, mediaType, maxLoanPeriodDays, createdAt, actor, actorMod, isAvailable);
    // Autosave nach erfolgreichem Hinzufügen
    saveData();
    return assignedId;
}

    int Library::addMember(const std::string& name, const std::string& email, const std::string& address) {
    int assignedId = nextMemberID++;
    borrowers.emplace_back(assignedId, name, email, address);
    // Autosave nach erfolgreichem Hinzufügen
    saveData();
    return assignedId;
}

// --- Hilfsmethoden zum Suchen (Pointer zurückgeben) ---

Book* Library::findBookByID(const int id) {
    for (auto& book : books) {
        if (book.inventoryID == id) return &book;
    }
    return nullptr;
}

    // Suche für das Menü
    std::vector<int> Library::searchBooks(const std::string& query) const {
        std::cout << "\n--- Suchergebnisse fuer '" << query << "' ---\n";
        std::vector<int> matches;

        for (const auto& book : books) {
            // Wir suchen im Titel ODER in der ISBN ODER in einem der Autoren (einfache Teilstring-Suche)
            bool authorHit = false;
            for (const auto& a : book.authors) {
                if (a.find(query) != std::string::npos) { authorHit = true; break; }
            }
            if (book.title.find(query) != std::string::npos ||
                book.isbn.find(query) != std::string::npos ||
                authorHit) {

                // Autoren als kommagetrennte Liste ausgeben (falls leer, nichts anzeigen)
                std::ostringstream oss;
                for (size_t i = 0; i < book.authors.size(); ++i) {
                    if (i) oss << ", ";
                    oss << book.authors[i];
                }

                std::cout << "[ID: " << book.inventoryID << "] " << book.title;
                if (!book.authors.empty()) {
                    std::cout << " (" << oss.str() << ")";
                }
                std::cout << " - " << (book.isAvailable ? "Verfuegbar" : "AUSGELIEHEN") << "\n";
                matches.push_back(book.inventoryID);
            }
        }

        if (matches.empty()) std::cout << "Keine Buecher gefunden.\n";
        return matches;
    }

    // Reine ID-Suche ohne Konsolen-Ausgabe (für Paging im UI)
    std::vector<int> Library::searchBooksIDs(const std::string& query) const {
        std::vector<int> ids;
        if (query.empty()) return ids;
        auto containsIC = [](const std::string& hay, const std::string& needle){
            if (needle.empty()) return true;
            std::string H, N; H.reserve(hay.size()); N.reserve(needle.size());
            for (char c: hay) H.push_back((char)std::tolower((unsigned char)c));
            for (char c: needle) N.push_back((char)std::tolower((unsigned char)c));
            return H.find(N) != std::string::npos;
        };
        for (const auto& book : books) {
            bool authorHit = false;
            for (const auto& a : book.authors) { if (containsIC(a, query)) { authorHit = true; break; } }
            if (containsIC(book.title, query) || containsIC(book.isbn, query) || authorHit) {
                ids.push_back(book.inventoryID);
            }
        }
        return ids;
    }

    // Mitgliedersuche per E-Mail (Teilstring, case-insensitive) → Member-IDs
    std::vector<int> Library::searchBorrowersByEmail(const std::string& query) const {
        std::vector<int> ids;
        if (query.empty()) return ids;
        std::string q; q.reserve(query.size()); for (char c: query) q.push_back((char)std::tolower((unsigned char)c));
        for (const auto& m : borrowers) {
            std::string e; e.reserve(m.email.size()); for (char c: m.email) e.push_back((char)std::tolower((unsigned char)c));
            if (e.find(q) != std::string::npos) ids.push_back(m.memberID);
        }
        return ids;
    }

    // Mitgliedersuche per Name (Teilstring, case-insensitive) → Member-IDs
    std::vector<int> Library::searchBorrowersByName(const std::string& query) const {
        std::vector<int> ids;
        if (query.empty()) return ids;
        std::string q; q.reserve(query.size()); for (char c: query) q.push_back((char)std::tolower((unsigned char)c));
        for (const auto& m : borrowers) {
            std::string n; n.reserve(m.name.size()); for (char c: m.name) n.push_back((char)std::tolower((unsigned char)c));
            if (n.find(q) != std::string::npos) ids.push_back(m.memberID);
        }
        return ids;
    }

    // Vereinheitlichte Mitgliedersuche: ID (exakt, wenn numerisch) oder Teilstring über E‑Mail/Name (case-insensitive)
    std::vector<int> Library::searchBorrowersIDs(const std::string& query) const {
        std::vector<int> ids;
        if (query.empty()) return ids;
        // Prüfen, ob rein numerisch (optional führende/leerende Spaces ignorieren)
        auto isDigits = [](const std::string& s){
            if (s.empty()) return false;
            for (char c : s) if (!std::isdigit((unsigned char)c)) return false;
            return true;
        };
        if (isDigits(query)) {
            int wanted = 0;
            try { wanted = std::stoi(query); } catch(...) { wanted = -1; }
            for (const auto& m : borrowers) if (m.memberID == wanted) { ids.push_back(m.memberID); break; }
            return ids;
        }
        // ansonsten: E-Mail oder Name enthält query (case-insensitive)
        std::string q; q.reserve(query.size()); for (char c: query) q.push_back((char)std::tolower((unsigned char)c));
        for (const auto& m : borrowers) {
            std::string e; e.reserve(m.email.size()); for (char c: m.email) e.push_back((char)std::tolower((unsigned char)c));
            std::string n; n.reserve(m.name.size());  for (char c: m.name)  n.push_back((char)std::tolower((unsigned char)c));
            if (e.find(q) != std::string::npos || n.find(q) != std::string::npos) ids.push_back(m.memberID);
        }
        return ids;
    }

    // Hilfsmethode: Alle Bücher anzeigen
    void Library::printAllBooks() const {
        std::cout << "\n--- Buchbestand (" << books.size() << ") ---\n";
        for (const auto& book : books) {
            std::cout << "[ID: " << book.inventoryID << "] " << book.title
                      << " (" << (book.isAvailable ? "Verfuegbar" : "Verliehen") << ")\n";
        }
    }

    // Hilfsmethode: Alle Mitglieder anzeigen (fürs Ausleihen)
    void Library::printAllMembers() const {
        std::cout << "\n--- Mitgliederliste (" << borrowers.size() << ") ---\n";
        for (const auto& m : borrowers) {
            std::cout << "[ID: " << m.memberID << "] " << m.name << "\n";
        }
    }

    Borrower* Library::findBorrowerByID(const int id) {
        for (auto& borrower : borrowers) {
            if (borrower.memberID == id) return &borrower;
        }
        return nullptr;
    }

    // --- Mitglieder-Management ---
    bool Library::editMember(const int memberID, const std::string& name, const std::string& email, const std::string& address) {
        for (auto& m : borrowers) {
            if (m.memberID == memberID) {
                if (!name.empty()) m.name = name;
                if (!email.empty()) m.email = email;
                if (!address.empty()) m.address = address;
                // Autosave nach erfolgreichem Update
                saveData();
                return true;
            }
        }
        return false;
    }

    bool Library::setMemberStatus(const int memberID, const BorrowerStatus status) {
        for (auto& m : borrowers) {
            if (m.memberID == memberID) {
                m.status = status;
                // Autosave nach erfolgreichem Update
                saveData();
                return true;
            }
        }
        return false;
    }

    // --- Ausleih-Logik ---

    bool Library::borrowBook(int bookID, int memberID) {
        Book* book = findBookByID(bookID);
        Borrower* borrower = findBorrowerByID(memberID);

        // 1. Validierung
        if (!book) {
            std::cout << "Fehler: Buch mit ID " << bookID << " nicht gefunden.\n";
            return false;
        }
        if (!borrower) {
            std::cout << "Fehler: Mitglied mit ID " << memberID << " nicht gefunden.\n";
            return false;
        }
        // Borrower-Status prüfen: nur Active darf ausleihen
        if (borrower->status != BorrowerStatus::Active) {
            std::cout << "Fehler: Mitglied '" << borrower->name << "' ist gesperrt (Status: Blocked).\n";
            return false;
        }
        if (!book->isAvailable) {
            std::cout << "Fehler: Buch '" << book->title << "' ist bereits verliehen!\n";
            return false;
        }

        // 2. Status ändern + Modifikationsstempel
        book->isAvailable = false;
        if (currentUserID != -1) {
            for (const auto& e : employees) if (e.employeeID == currentUserID) { book->lastModifiedBy = e.username; break; }
        }

        // 3. Transaktion erstellen (Faelligkeit abhängig von maxLoanPeriodDays des Buches)
        loans.emplace_back(nextLoanID++, bookID, memberID, book->maxLoanPeriodDays);

        // performedBy setzen (aktueller Benutzer oder "system")
        {
            std::string actor = "system";
            if (currentUserID != -1) {
                for (const auto& e : employees) if (e.employeeID == currentUserID) { actor = e.username; break; }
            }
            loans.back().performedBy = actor;
        }

        // Info-Ausgabe mit Datum
        Loan& newLoan = loans.back();
        std::cout << "Erfolg: '" << book->title << "' wurde an " << borrower->name
                  << " verliehen. Faellig am: " << dateToString(newLoan.dueDate) << "\n";
            // Autosave nach erfolgreichem Ausleihen
            saveData();
            return true;
        }

    // --- Rückgabe-Logik ---

    bool Library::returnBook(int bookID) {
        // Wir suchen in den Loans nach einem Eintrag, der:
        // A) Die richtige Buch-ID hat
        // B) NOCH NICHT zurückgegeben wurde (returnDate == 0)

        auto it = std::find_if(loans.begin(), loans.end(), [bookID](const Loan& loan) {
            return loan.bookInventoryID == bookID && loan.returnDate == 0;
        });

        if (it != loans.end()) {
            // Treffer! Es gibt eine offene Ausleihe.

            // 1. Rückgabedatum setzen (Transaktion abschließen)
            it->returnDate = std::time(nullptr);
            // performedBy aktualisieren (Wer hat die Rückgabe verbucht?)
            if (currentUserID != -1) {
                for (const auto& e : employees) if (e.employeeID == currentUserID) { it->performedBy = e.username; break; }
            } else {
                it->performedBy = "system";
            }

            // 2. Buch wieder verfügbar machen
            if (Book* book = findBookByID(bookID)) {
                book->isAvailable = true;
                if (currentUserID != -1) {
                    for (const auto& e : employees) if (e.employeeID == currentUserID) { book->lastModifiedBy = e.username; break; }
                }
                std::cout << "Erfolg: Buch '" << book->title << "' wurde zurueckgegeben.\n";
            }
            // Autosave nach erfolgreicher Rückgabe
            saveData();
            return true;
        }
        std::cout << "Fehler: Dieses Buch ist aktuell gar nicht ausgeliehen.\n";
        return false;
    }

    // --- REPORTING ---
    static std::time_t startOfDay(std::time_t t) {
        std::tm lt = *std::localtime(&t);
        lt.tm_hour = 0; lt.tm_min = 0; lt.tm_sec = 0;
        return std::mktime(&lt);
    }
    static std::time_t endOfDay(std::time_t t) {
        return startOfDay(t) + (24 * 60 * 60) - 1;
    }

    void Library::showDailyReport(std::time_t day) const {
        auto begin = startOfDay(day);
        auto end = endOfDay(day);

        std::vector<const Loan*> borrowed;
        std::vector<const Loan*> returned;
        for (const auto& l : loans) {
            if (l.loanDate >= begin && l.loanDate <= end) borrowed.push_back(&l);
            if (l.returnDate != 0 && l.returnDate >= begin && l.returnDate <= end) returned.push_back(&l);
        }
        auto byTime = [](const Loan* a, const Loan* b){ return a->loanDate < b->loanDate; };
        std::sort(borrowed.begin(), borrowed.end(), byTime);
        auto byRet = [](const Loan* a, const Loan* b){ return a->returnDate < b->returnDate; };
        std::sort(returned.begin(), returned.end(), byRet);

        std::cout << "\n=== Tagesbericht (" << dateToString(day) << ") ===\n";
        std::cout << "Ausgeliehen: " << borrowed.size() << "\n";
        for (auto* l : borrowed) {
            const Book* b = nullptr; const Borrower* m = nullptr;
            for (const auto& bk : books) if (bk.inventoryID == l->bookInventoryID) { b = &bk; break; }
            for (const auto& br : borrowers) if (br.memberID == l->borrowerID) { m = &br; break; }
            std::cout << "  [" << std::put_time(std::localtime(&l->loanDate), "%H:%M") << "] ";
            std::cout << (b? b->title : "Buch#"+std::to_string(l->bookInventoryID))
                      << " -> " << (m? m->name : ("Mitglied#"+std::to_string(l->borrowerID)))
                      << " | durch: " << (l->performedBy.empty()?"n/a":l->performedBy)
                      << " | faellig: " << dateToString(l->dueDate) << "\n";
        }
        std::cout << "Zurueckgegeben: " << returned.size() << "\n";
        for (auto* l : returned) {
            const Book* b = nullptr; const Borrower* m = nullptr;
            for (const auto& bk : books) if (bk.inventoryID == l->bookInventoryID) { b = &bk; break; }
            for (const auto& br : borrowers) if (br.memberID == l->borrowerID) { m = &br; break; }
            std::cout << "  [" << std::put_time(std::localtime(&l->returnDate), "%H:%M") << "] ";
            std::cout << (b? b->title : "Buch#"+std::to_string(l->bookInventoryID))
                      << " <- " << (m? m->name : ("Mitglied#"+std::to_string(l->borrowerID)))
                      << " | durch: " << (l->performedBy.empty()?"n/a":l->performedBy)
                      << "\n";
        }

        // Summen/Statistiken pro MediaType
        auto mediaLabel = [](MediaType mt){
            switch (mt) {
                case MediaType::Book: return "Book";
                case MediaType::Magazine: return "Magazine";
                case MediaType::DVD: return "DVD";
                case MediaType::EBook: return "EBook";
                default: return "Other";
            }
        };

        std::map<MediaType, size_t> borrowCounts;
        std::map<MediaType, size_t> returnCounts;
        for (auto* l : borrowed) {
            for (const auto& bk : books) if (bk.inventoryID == l->bookInventoryID) { borrowCounts[bk.mediaType]++; break; }
        }
        for (auto* l : returned) {
            for (const auto& bk : books) if (bk.inventoryID == l->bookInventoryID) { returnCounts[bk.mediaType]++; break; }
        }

        std::cout << "\n--- Summen nach MediaType ---\n";
        std::cout << "Ausgeliehen gesamt: " << borrowed.size() << " | Zurueckgegeben gesamt: " << returned.size() << "\n";
        for (int mt = 0; mt <= static_cast<int>(MediaType::Other); ++mt) {
            MediaType key = static_cast<MediaType>(mt);
            size_t bc = borrowCounts[key];
            size_t rc = returnCounts[key];
            if (bc == 0 && rc == 0) continue;
            std::cout << "  " << mediaLabel(key) << ": ausgeliehen " << bc << ", zurueckgegeben " << rc << "\n";
        }
    }

    std::vector<Loan> Library::getDueReport(size_t limit) const {
        const auto now = std::time(nullptr);
        std::vector<Loan> open;
        for (const auto& l : loans) if (l.returnDate == 0) open.push_back(l);
        std::sort(open.begin(), open.end(), [now](const Loan& a, const Loan& b){
            auto da = (long long)a.dueDate - (long long)now;
            auto db = (long long)b.dueDate - (long long)now;
            return da < db; // überfällige zuerst, dann bald fällige
        });
        if (open.size() > limit) open.erase(open.begin() + static_cast<long>(limit), open.end());
        return open;
    }

    // --- PERSISTENZ: Speichern ---
    bool Library::saveData() {
        std::ofstream out(dataFilePath, std::ios::binary);
        if (!out) {
            std::cerr << "Fehler: Konnte Datei nicht zum Speichern oeffnen!\n";
            return false;
        }

        // Header: Magic + Version
        const char magic[4] = {'B','N','E','S'};
        out.write(magic, sizeof(magic));
        uint32_t version = 5;
        out.write(reinterpret_cast<char*>(&version), sizeof(version));

        // 1. BÜCHER speichern (v3 Layout)
        size_t bookCount = books.size();
        out.write(reinterpret_cast<char *>(&bookCount), sizeof(bookCount)); // Anzahl schreiben

        for (const auto& b : books) {
            // Grunddaten
            out.write((char*)&b.inventoryID, sizeof(b.inventoryID));
            writeString(out, b.isbn);
            writeString(out, b.title);

            // Autoren (vector<string>)
            size_t aCount = b.authors.size();
            out.write(reinterpret_cast<char*>(&aCount), sizeof(aCount));
            for (const auto& a : b.authors) writeString(out, a);

            // Erweiterte Felder
            writeString(out, b.publisher);
            writeString(out, b.edition);
            writeString(out, b.location);
            writeString(out, b.genre);
            out.write(reinterpret_cast<const char*>(&b.price), sizeof(b.price));
            out.write(reinterpret_cast<const char*>(&b.maxLoanPeriodDays), sizeof(b.maxLoanPeriodDays));
            // mediaType als int32
            int32_t mt = static_cast<int32_t>(b.mediaType);
            out.write(reinterpret_cast<char*>(&mt), sizeof(mt));
            out.write(reinterpret_cast<const char*>(&b.createdAt), sizeof(b.createdAt));
            writeString(out, b.createdBy);
            writeString(out, b.lastModifiedBy);
            out.write(reinterpret_cast<const char*>(&b.isAvailable), sizeof(b.isAvailable));
        }

        // 2. MITGLIEDER speichern (v5: inkl. registrationDate, status)
        size_t memberCount = borrowers.size();
        out.write(reinterpret_cast<char *>(&memberCount), sizeof(memberCount));

        for (const auto& m : borrowers) {
            out.write((char*)&m.memberID, sizeof(m.memberID));
            writeString(out, m.name);
            writeString(out, m.email);
            writeString(out, m.address);
            out.write(reinterpret_cast<const char*>(&m.registrationDate), sizeof(m.registrationDate));
            int32_t st = static_cast<int32_t>(m.status);
            out.write(reinterpret_cast<char*>(&st), sizeof(st));
        }

        // 3. MITARBEITER speichern
        size_t empCount = employees.size();
        out.write(reinterpret_cast<char *>(&empCount), sizeof(empCount));
        for (const auto& e : employees) {
            out.write((char*)&e.employeeID, sizeof(e.employeeID));
            writeString(out, e.username);
            writeString(out, e.fullName);
            writeString(out, e.passwordHash);
            int32_t r = static_cast<int32_t>(e.role);
            out.write(reinterpret_cast<char*>(&r), sizeof(r));
            out.write(reinterpret_cast<const char*>(&e.createdAt), sizeof(e.createdAt));
            out.write(reinterpret_cast<const char*>(&e.isActive), sizeof(e.isActive));
        }

        // 4. AUSLEIHEN (Historie) speichern (v4: feldweise inkl. performedBy)
        size_t loanCount = loans.size();
        out.write((char*)&loanCount, sizeof(loanCount));

        for (const auto& l : loans) {
            out.write((char*)&l.loanID, sizeof(l.loanID));
            out.write((char*)&l.bookInventoryID, sizeof(l.bookInventoryID));
            out.write((char*)&l.borrowerID, sizeof(l.borrowerID));
            out.write((char*)&l.loanDate, sizeof(l.loanDate));
            out.write((char*)&l.dueDate, sizeof(l.dueDate));
            out.write((char*)&l.returnDate, sizeof(l.returnDate));
            writeString(out, l.performedBy);
        }

        // 5. ID-Zähler speichern (Wichtig, damit IDs nach Neustart nicht doppelt vergeben werden)
        out.write(reinterpret_cast<char *>(&nextBookID), sizeof(nextBookID));
        out.write(reinterpret_cast<char *>(&nextMemberID), sizeof(nextMemberID));
        out.write(reinterpret_cast<char *>(&nextLoanID), sizeof(nextLoanID));
        out.write(reinterpret_cast<char *>(&nextEmployeeID), sizeof(nextEmployeeID));

        std::cout << "Daten erfolgreich in '" << dataFilePath << "' gespeichert.\n";
        out.close();
        return true;
    }

    // --- PERSISTENZ: Laden ---
    bool Library::loadData() {
        std::ifstream in(dataFilePath, std::ios::binary);
        if (!in) {
            std::cout << "Keine gespeicherten Daten gefunden in '" << dataFilePath << "'. Starte leer.\n";
            return false;
        }

        // Vor dem Laden alles löschen
        books.clear();
        borrowers.clear();
        loans.clear();

        // Header prüfen: Magic + Version
        char magic[4];
        in.read(magic, sizeof(magic));
        if (!in || magic[0] != 'B' || magic[1] != 'N' || magic[2] != 'E' || magic[3] != 'S') {
            std::cerr << "Fehler: Ungueltiges Datenformat (Magic).\n";
            return false;
        }
        uint32_t version = 0;
        in.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (!in || version != 5) {
            std::cerr << "Fehler: Nicht unterstuetzte Version: " << version << "\n";
            return false;
        }

        // 1. BÜCHER laden (v3 Layout)
        size_t bookCount;
        in.read(reinterpret_cast<char *>(&bookCount), sizeof(bookCount));
        for (size_t i = 0; i < bookCount; ++i) {
            int id;
            in.read(reinterpret_cast<char *>(&id), sizeof(id));
            std::string isbn = readString(in);
            std::string title = readString(in);

            size_t aCount = 0;
            in.read(reinterpret_cast<char*>(&aCount), sizeof(aCount));
            std::vector<std::string> authors;
            authors.reserve(aCount);
            for (size_t ai = 0; ai < aCount; ++ai) authors.push_back(readString(in));

            std::string publisher = readString(in);
            std::string edition = readString(in);
            std::string location = readString(in);
            std::string genre = readString(in);
            double price = 0.0;
            in.read(reinterpret_cast<char*>(&price), sizeof(price));
            int maxLoanPeriodDays = 14;
            in.read(reinterpret_cast<char*>(&maxLoanPeriodDays), sizeof(maxLoanPeriodDays));
            int32_t mtInt = 0;
            in.read(reinterpret_cast<char*>(&mtInt), sizeof(mtInt));
            std::time_t createdAt = 0;
            in.read(reinterpret_cast<char*>(&createdAt), sizeof(createdAt));
            std::string createdBy = readString(in);
            std::string lastModifiedBy = readString(in);
            bool avail = true;
            in.read(reinterpret_cast<char*>(&avail), sizeof(avail));

            Book b(id, std::move(isbn), std::move(title), std::move(authors), std::move(publisher),
                   std::move(location), std::move(edition), std::move(genre), price,
                   static_cast<MediaType>(mtInt), maxLoanPeriodDays, createdAt,
                   std::move(createdBy), std::move(lastModifiedBy), avail);
            books.push_back(std::move(b));
        }

        // 2. MITGLIEDER laden (v5)
        size_t memberCount;
        in.read(reinterpret_cast<char *>(&memberCount), sizeof(memberCount));
        for (size_t i = 0; i < memberCount; ++i) {
            int id;
            in.read(reinterpret_cast<char *>(&id), sizeof(id));
            std::string name = readString(in);
            std::string email = readString(in);
            std::string addr = readString(in);
            std::time_t reg = 0; in.read(reinterpret_cast<char *>(&reg), sizeof(reg));
            int32_t stInt = 0; in.read(reinterpret_cast<char *>(&stInt), sizeof(stInt));
            borrowers.emplace_back(id, name, email, addr, reg, static_cast<BorrowerStatus>(stInt));
        }

        // 3. MITARBEITER laden
        size_t empCount;
        in.read(reinterpret_cast<char *>(&empCount), sizeof(empCount));
        for (size_t i = 0; i < empCount; ++i) {
            int id; in.read(reinterpret_cast<char *>(&id), sizeof(id));
            std::string uname = readString(in);
            std::string fname = readString(in);
            std::string pwh = readString(in);
            int32_t rInt = 0; in.read(reinterpret_cast<char *>(&rInt), sizeof(rInt));
            std::time_t cat = 0; in.read(reinterpret_cast<char *>(&cat), sizeof(cat));
            bool active = true; in.read(reinterpret_cast<char *>(&active), sizeof(active));
            employees.emplace_back(id, uname, fname, pwh, static_cast<Role>(rInt), cat, active);
        }

        // 4. AUSLEIHEN laden (v4: feldweise inkl. performedBy)
        size_t loanCount;
        in.read(reinterpret_cast<char *>(&loanCount), sizeof(loanCount));
        for (size_t i = 0; i < loanCount; ++i) {
            int id; int bID; int mID;
            std::time_t ld, dd, rd;
            in.read(reinterpret_cast<char *>(&id), sizeof(id));
            in.read(reinterpret_cast<char *>(&bID), sizeof(bID));
            in.read(reinterpret_cast<char *>(&mID), sizeof(mID));
            in.read(reinterpret_cast<char *>(&ld), sizeof(ld));
            in.read(reinterpret_cast<char *>(&dd), sizeof(dd));
            in.read(reinterpret_cast<char *>(&rd), sizeof(rd));
            std::string perf = readString(in);

            Loan l(id, bID, mID);
            l.loanDate = ld;
            l.dueDate = dd;
            l.returnDate = rd;
            l.performedBy = std::move(perf);
            loans.push_back(std::move(l));
        }

        // 5. Zähler wiederherstellen
        in.read(reinterpret_cast<char *>(&nextBookID), sizeof(nextBookID));
        in.read(reinterpret_cast<char *>(&nextMemberID), sizeof(nextMemberID));
        in.read(reinterpret_cast<char *>(&nextLoanID), sizeof(nextLoanID));
        in.read(reinterpret_cast<char *>(&nextEmployeeID), sizeof(nextEmployeeID));

        std::cout << "Daten erfolgreich aus '" << dataFilePath << "' geladen (" << books.size() << " Buecher).\n";
        in.close();
        return true;
    }

// --- Mitarbeiter-API ---

int Library::addEmployee(const std::string& username,
                         const std::string& fullName,
                         Role role,
                         const std::string& rawPassword) {
    // Rechte: nur Admin darf
    const Employee* current = getCurrentUser();
    if (!current || current->role != Role::Admin) {
        std::cout << "Fehler: Nur Admins duerfen Mitarbeiter anlegen.\n";
        return -1;
    }
    if (findEmployeeByUsername(employees, username)) {
        std::cout << "Fehler: Benutzername bereits vergeben.\n";
        return -1;
    }
    int id = nextEmployeeID++;
    employees.emplace_back(id, username, fullName, simpleHash(rawPassword), role, std::time(nullptr), true);
    // Autosave nach erfolgreichem Anlegen
    saveData();
    return id;
}

bool Library::deactivateEmployee(int employeeID) {
    const Employee* current = getCurrentUser();
    if (!current || current->role != Role::Admin) { std::cout << "Fehler: Admin-Recht erforderlich.\n"; return false; }
    for (auto& e : employees) {
        if (e.employeeID == employeeID) { e.isActive = false; saveData(); return true; }
    }
    return false;
}

bool Library::reactivateEmployee(int employeeID) {
    const Employee* current = getCurrentUser();
    if (!current || current->role != Role::Admin) { std::cout << "Fehler: Admin-Recht erforderlich.\n"; return false; }
    for (auto& e : employees) {
        if (e.employeeID == employeeID) { e.isActive = true; saveData(); return true; }
    }
    return false;
}

bool Library::resetEmployeePassword(int employeeID, const std::string& rawPassword) {
    const Employee* current = getCurrentUser();
    if (!current || current->role != Role::Admin) { std::cout << "Fehler: Admin-Recht erforderlich.\n"; return false; }
    for (auto& e : employees) {
        if (e.employeeID == employeeID) { e.passwordHash = simpleHash(rawPassword); saveData(); return true; }
    }
    return false;
}

bool Library::authenticate(const std::string& username, const std::string& rawPassword) {
    for (const auto& e : employees) {
        if (e.username == username && e.isActive) {
            if (e.passwordHash == simpleHash(rawPassword)) { currentUserID = e.employeeID; return true; }
            return false;
        }
    }
    // falls keine Mitarbeiter existieren: Default-Admin erzeugen und versuchen
    if (employees.empty()) {
        int id = nextEmployeeID++;
        employees.emplace_back(id, "admin", "Administrator", simpleHash("admin"), Role::Admin, std::time(nullptr), true);
        if (username == "admin" && rawPassword == "admin") { currentUserID = id; return true; }
    }
    return false;
}

void Library::logout() { currentUserID = -1; }

const Employee* Library::getCurrentUser() const {
    if (currentUserID == -1) return nullptr;
    for (const auto& e : employees) if (e.employeeID == currentUserID) return &e;
    return nullptr;
}

    // --- Testdaten Generator ---
    void Library::generateDummyData() {
    std::cout << "Generiere Testdaten...\n";
    // 5 Bücher (mit Minimalfeldern + Defaults)
    addBook("978-3-1", "Der C++ Programmierer", {"Ulrich Breymann"}, "Hanser",
            "GEN", "", "", 0.0, MediaType::Book);
    addBook("978-3-2", "Clean Code", {"Robert C. Martin"}, "Prentice Hall",
            "GEN", "", "", 0.0, MediaType::Book);
    addBook("978-3-3", "Design Patterns", {"Erich Gamma"}, "Addison-Wesley",
            "GEN", "", "", 0.0, MediaType::Book);
    addBook("978-3-4", "Harry Potter 1", {"J.K. Rowling"}, "Carlsen",
            "GEN", "", "", 0.0, MediaType::Book);
    addBook("978-3-5", "Herr der Ringe", {"J.R.R. Tolkien"}, "Klett-Cotta",
            "GEN", "", "", 0.0, MediaType::Book);

    // 3 Mitglieder
    addMember("Max Mustermann", "max@test.de", "Musterweg 1");
    addMember("Erika Musterfrau", "erika@test.de", "Beispielstrasse 2");
    addMember("John Doe", "john@doe.com", "Unknown Road 42");

    std::cout << "Daten generiert.\n";
}

// -------------------- CSV-Import --------------------

namespace {
    static inline std::string trim(const std::string& s) {
        size_t b = 0, e = s.size();
        while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
        while (e > b && std::isspace(static_cast<unsigned char>(s[e-1]))) --e;
        return s.substr(b, e - b);
    }

    static std::vector<std::string> splitSimple(const std::string& line, char delim) {
        std::vector<std::string> out; std::string cur; std::istringstream iss(line);
        while (std::getline(iss, cur, delim)) out.push_back(cur);
        // Entferne CR am Feldende (Windows-Zeilenenden)
        for (auto& f : out) { if (!f.empty() && f.back()=='\r') f.pop_back(); f = trim(f); }
        return out;
    }

    static std::vector<std::string> splitAuthors(const std::string& field) {
        std::vector<std::string> v = splitSimple(field, '|');
        for (auto& s : v) s = trim(s);
        if (v.size()==1 && v[0].empty()) v.clear();
        return v;
    }

    static MediaType parseMedia(const std::string& s) {
        std::string t; t.reserve(s.size());
        for (char c : s) t.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        if (t=="book" || t=="buch") return MediaType::Book;
        if (t=="magazine" || t=="magazin" || t=="zeitschrift") return MediaType::Magazine;
        if (t=="dvd") return MediaType::DVD;
        if (t=="ebook" || t=="e-book" || t=="e_buch") return MediaType::EBook;
        if (t=="other" || t=="sonstiges") return MediaType::Other;
        return MediaType::Book;
    }
}

bool Library::importBooksFromCSV(const std::string& directoryPath) {
    namespace fs = std::filesystem;
    fs::path dir(directoryPath);
    fs::path file = dir / "books.csv";
    return importBooksFromCSVFile(file.string());
}

bool Library::importBooksFromCSVFile(const std::string& filePath) {
    std::ifstream in(filePath);
    if (!in) { std::cout << "Import: Datei '" << filePath << "' nicht gefunden.\n"; return false; }

    std::string header; if (!std::getline(in, header)) { std::cout << "Import: Leere books.csv.\n"; return false; }
    // Erwartete Spalten (mindestens):
    // ISBN;Title;Authors;Publisher;Edition;Location;Genre;Price;MaxLoanDays;MediaType;CreatedBy
    // Nicht jede ist Pflicht: Pflicht sind ISBN, Title. Der Rest optional.
    const auto h = splitSimple(header, ';');
    auto col = [&](const std::string& name)->int{
        for (size_t i=0;i<h.size();++i){ std::string x=h[i]; for(char& c: x) c= (char)std::tolower((unsigned char)c); if (x==name) return (int)i; }
        return -1;
    };
    int ci_isbn = col("isbn");
    int ci_title = col("title");
    if (ci_isbn==-1 || ci_title==-1) {
        std::cout << "Import: Header muss mindestens ISBN und Title enthalten.\n"; return false;
    }
    int ci_authors = col("authors");
    int ci_publisher = col("publisher");
    int ci_edition = col("edition");
    int ci_location = col("location");
    int ci_genre = col("genre");
    int ci_price = col("price");
    int ci_maxloan = col("maxloandays");
    int ci_media = col("mediatype");
    int ci_createdby = col("createdby");

    size_t imported = 0, skipped = 0; std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto f = splitSimple(line, ';');
        auto get = [&](int idx)->std::string{ return (idx>=0 && (size_t)idx<f.size()) ? f[(size_t)idx] : std::string(); };
        std::string isbn = get(ci_isbn);
        std::string title = get(ci_title);
        if (isbn.empty() || title.empty()) { ++skipped; continue; }
        std::vector<std::string> authors = splitAuthors(get(ci_authors));
        std::string publisher = get(ci_publisher);
        std::string edition = get(ci_edition);
        std::string location = get(ci_location);
        std::string genre = get(ci_genre);
        double price = 0.0; if (ci_price>=0) { try { price = std::stod(get(ci_price)); } catch(...){} }
        int maxLoanDays = 14; if (ci_maxloan>=0) { try { maxLoanDays = std::stoi(get(ci_maxloan)); } catch(...){} }
        MediaType mt = MediaType::Book; if (ci_media>=0) mt = parseMedia(get(ci_media));
        std::string createdBy = get(ci_createdby); if (createdBy.empty()) createdBy = "import";

        addBook(isbn, title, authors, publisher, location, edition, genre, price, mt, maxLoanDays, std::time(nullptr), createdBy, createdBy, true);
        ++imported;
    }

    std::cout << "Import Buecher: " << imported << " importiert, " << skipped << " uebersprungen.\n";
    return imported>0;
}

bool Library::importMembersFromCSV(const std::string& directoryPath) {
    namespace fs = std::filesystem;
    fs::path dir(directoryPath);
    fs::path file = dir / "members.csv";
    return importMembersFromCSVFile(file.string());
}

bool Library::importMembersFromCSVFile(const std::string& filePath) {
    std::ifstream in(filePath);
    if (!in) { std::cout << "Import: Datei '" << filePath << "' nicht gefunden.\n"; return false; }

    std::string header; if (!std::getline(in, header)) { std::cout << "Import: Leere members.csv.\n"; return false; }
    // Erwartete Spalten (mindestens): Name;Email;Address
    // Optional: RegistrationDate(YYYY-MM-DD);Status(Active|Blocked)
    const auto h = splitSimple(header, ';');
    auto col = [&](const std::string& name)->int{
        for (size_t i=0;i<h.size();++i){ std::string x=h[i]; for(char& c: x) c= (char)std::tolower((unsigned char)c); if (x==name) return (int)i; }
        return -1;
    };
    int ci_name = col("name");
    int ci_email = col("email");
    int ci_address = col("address");
    int ci_reg = col("registrationdate");
    int ci_status = col("status");
    if (ci_name==-1 || ci_email==-1 || ci_address==-1) { std::cout << "Import: Header members.csv unvollstaendig.\n"; return false; }

    size_t imported = 0, skipped = 0; std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto f = splitSimple(line, ';');
        auto get = [&](int idx)->std::string{ return (idx>=0 && (size_t)idx<f.size()) ? f[(size_t)idx] : std::string(); };
        std::string name = get(ci_name);
        std::string email = get(ci_email);
        std::string address = get(ci_address);
        if (name.empty() || email.empty()) { ++skipped; continue; }

        // Einfacher Weg: Wir nutzen die bestehende addMember-API und lassen RegistrationDate/Status auf Defaults.
        addMember(name, email, address);
        ++imported;
    }
    std::cout << "Import Mitglieder: " << imported << " importiert, " << skipped << " uebersprungen.\n";
    return imported>0;
}