#include <iostream>
#include <limits> // Für cin.ignore
#include <sstream>
#include <filesystem>
#include "Library.h"
#include "Utils.h"

// Menü-Rendering und Aktions-Wrapper für „angeheftete“ Darstellung
static void drawMainMenu(bool showEmployeeMenu) {
    std::cout << "====================================\n";
    std::cout << "       BOOKNEST LIBRARY v1.0        \n";
    std::cout << "====================================\n";
    std::cout << "[1] Books\n";
    std::cout << "[2] Members\n";
    std::cout << "[3] Reports\n";
    if (showEmployeeMenu) {
        std::cout << "[4] Staff (Admin)\n";
        std::cout << "[5] Import (Admin)\n";
    }
    std::cout << "[6] Settings\n";
    printMenuFooter("Logout");
}

template<typename F>
static void runAction(const std::string& title, F body) {
    clearScreen();
    std::cout << "--- " << title << " ---\n";
    body();
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

int main() {
    Library myLib;
    int choice = 0;

    // Zum Start Daten laden – bevorzugt in ./data (für Docker-Volumes),
    // dann Fallback ../data, anschließend klassisch ./ und ../
    // data/-Verzeichnis anlegen falls es noch nicht existiert (erster Programmstart)
    std::filesystem::create_directories("data");
    myLib.setDataFilePath("data/library.bin");
    bool loaded = myLib.loadData();
    if (!loaded) {
        myLib.setDataFilePath("../data/library.bin");
        loaded = myLib.loadData();
    }
    if (!loaded) {
        myLib.setDataFilePath("library.bin");
        loaded = myLib.loadData();
    }
    if (!loaded) {
        myLib.setDataFilePath("../library.bin");
        loaded = myLib.loadData();
        if (!loaded) {
            // wieder auf Standard für späteres Speichern
            myLib.setDataFilePath("data/library.bin");
        }
    }
    // Kein Onboarding: Wenn nichts geladen wurde, starten wir leer weiter.

    // Login-Flow: bei erster Nutzung existiert Default-Admin (admin/admin)
    while (true) {
        clearScreen();
        const Employee* user = myLib.getCurrentUser();
        if (user) break; // bereits eingeloggt (z. B. aus vorheriger Session nicht gewünscht, aber belassen)
        std::cout << "=== Login Required ===\n";
        std::cout << "Username: ";
        std::string uname; std::cin >> uname;
        std::cout << "Password: ";
        std::string pwd; std::cin >> pwd;
        if (myLib.authenticate(uname, pwd)) {
            std::cout << "Login successful.\n";
            break;
        } else {
            std::cout << "Login failed. Hint: Default is admin/admin.\n";
            std::cout << "Press Enter to try again...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
        }
    }

    while (true) {
        clearScreen();
        const Employee* user = myLib.getCurrentUser();
        if (user) {
            std::cout << "Logged in as: " << user->username << " (" << (user->role == Role::Admin ? "Admin" : "Staff") << ")\n";
        } else {
            std::cout << "[Not logged in]\n";
        }
        drawMainMenu(user && user->role == Role::Admin);
        std::cout << "Your choice: ";

        // Read and validate input
        if (!(std::cin >> choice)) {
            std::cout << "Invalid input! Please enter a number.\n";
            std::cin.clear(); // Fehlerstatus löschen
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Puffer leeren
            continue;
        }

        switch (choice) {
            case 0: { // Logout (von 5 auf 0 verlegt)
                myLib.logout();
                // Zurück zum Login
                while (true) {
                    clearScreen();
                    std::cout << "=== Login Required ===\nUsername: ";
                    std::string un; std::cin >> un; std::cout << "Password: "; std::string pw; std::cin >> pw;
                    if (myLib.authenticate(un, pw)) break;
                    std::cout << "Login failed. Press Enter...";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cin.get();
                }
                break;
            }
            case 1: { // BUECHER Submenü (ohne runAction, um doppelte Enter-Pausen zu vermeiden)
                bool back = false;
                while (!back) {
                    clearScreen();
                    std::cout << "--- Books ---\n"
                              << "[1] Search\n[2] Borrow\n[3] Return\n";
                    printMenuFooter("Back");
                    std::cout << "Choice: ";
                    int bc=0; std::cin >> bc;
                    if (bc==1) {
                        // Bücher: Suche mit Paging (10er Schritte)
                        clearScreen();
                        std::cout << "--- Books: Search ---\n";
                        std::cout << "Search (Title/ISBN/Author): ";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::string query; std::getline(std::cin, query);
                        // Zeitmessung manuell, damit der Wert die Clear-Screen-Phase überlebt
                        auto t0_search = std::chrono::steady_clock::now();
                        std::vector<int> matches = myLib.searchBooksIDs(query);
                        double ms_search = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::steady_clock::now() - t0_search).count();
                        size_t shown = 0;
                        while (true) {
                            clearScreen();
                            std::cout.setf(std::ios::fixed); std::cout.precision(1);
                            std::cout << "--- Results (" << matches.size() << ")";
                            if (isBenchmarkEnabled()) std::cout << " — " << ms_search << " ms";
                            std::cout << " ---\n";
                            std::cout.unsetf(std::ios::floatfield);

                            // Tabellarische Ausgabe
                            auto mtToStr = [](MediaType mt){
                                switch (mt) {
                                    case MediaType::Book: return std::string("Book");
                                    case MediaType::Magazine: return std::string("Magazine");
                                    case MediaType::DVD: return std::string("DVD");
                                    case MediaType::EBook: return std::string("EBook");
                                    default: return std::string("Other");
                                }
                            };
                            const size_t W_ID=6, W_TITLE=30, W_AUTH=24, W_MT=10, W_AV=11, W_LOC=10, W_AB=12;
                            std::cout << padOrEllipsize("ID", W_ID) << " "
                                      << padOrEllipsize("Title", W_TITLE) << " "
                                      << padOrEllipsize("Authors", W_AUTH) << " "
                                      << padOrEllipsize("Type", W_MT) << " "
                                      << padOrEllipsize("Available", W_AV) << " "
                                      << padOrEllipsize("Location", W_LOC) << " "
                                      << padOrEllipsize("Avail. from", W_AB) << "\n";
                            std::cout << std::string(W_ID+1+W_TITLE+1+W_AUTH+1+W_MT+1+W_AV+1+W_LOC+1+W_AB, '-') << "\n";

                            size_t toShow = std::min<size_t>(10, matches.size() - shown);
                            for (size_t i = 0; i < toShow; ++i) {
                                int bid = matches[shown + i];
                                const Book* b = nullptr; for (const auto& x : myLib.getBooks()) if (x.inventoryID==bid){ b=&x; break; }
                                if (b) {
                                    std::ostringstream oss; for (size_t k=0;k<b->authors.size();++k){ if(k) oss<<", "; oss<<b->authors[k]; }
                                    std::string avail = b->isAvailable ? "Yes" : "No";
                                    std::string availFrom;
                                    if (!b->isAvailable) {
                                        // Offenen Loan suchen
                                        for (const auto& l : myLib.getLoans()) {
                                            if (l.bookInventoryID == b->inventoryID && l.returnDate == 0) {
                                                availFrom = dateToString(l.dueDate);
                                                break;
                                            }
                                        }
                                    }
                                    std::cout << padOrEllipsize(std::to_string(b->inventoryID), W_ID) << " "
                                              << padOrEllipsize(b->title, W_TITLE) << " "
                                              << padOrEllipsize(oss.str(), W_AUTH) << " "
                                              << padOrEllipsize(mtToStr(b->mediaType), W_MT) << " "
                                              << padOrEllipsize(avail, W_AV) << " "
                                              << padOrEllipsize(b->location, W_LOC) << " "
                                              << padOrEllipsize(availFrom, W_AB) << "\n";
                                }
                            }
                            shown += toShow;
                            if (shown >= matches.size()) {
                                std::cout << "\n[Enter] Back";
                                std::cin.get();
                                break;
                            } else {
                                std::cout << "\n[m] Load more  |  [Enter] Back: ";
                                std::string cmd; std::getline(std::cin, cmd);
                                if (cmd != "m" && cmd != "M" && cmd != "+") break;
                            }
                        }
                    } else if (bc==2) {
                        int bookID, memberID;
                        clearScreen();
                        std::cout << "--- Books: Borrow ---\n";
                        std::cout << "Book ID: "; std::cin >> bookID;
                        std::cout << "Member ID: "; std::cin >> memberID;
                        clearScreen();
                        std::cout << "--- Result ---\n";
                        myLib.borrowBook(bookID, memberID);
                        std::cout << "\n[Enter] Back";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (bc==3) {
                        int bookID;
                        clearScreen();
                        std::cout << "--- Books: Return ---\n";
                        std::cout << "Book ID: "; std::cin >> bookID;
                        clearScreen();
                        std::cout << "--- Result ---\n";
                        myLib.returnBook(bookID);
                        std::cout << "\n[Enter] Back";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (bc==0) {
                        back = true; // keine zusätzliche Enter-Pause hier
                    }
                }
                break;
            }
            case 2: { // MITGLIEDER Submenü (ohne runAction, um doppelte Enter-Pausen zu vermeiden)
                bool back = false;
                while (!back) {
                    clearScreen();
                    std::cout << "--- Members ---\n"
                              << "[1] Add\n[2] Edit\n[3] Activate/Deactivate\n[4] Search (ID/Email/Name)\n";
                    printMenuFooter("Back");
                    std::cout << "Choice: ";
                    int mc=0; std::cin >> mc;
                    if (mc==1) {
                        // Nur Formular + Ergebnis, kein Untermenü im Hintergrund
                        clearScreen();
                        std::cout << "--- Members: Add ---\n";
                        std::string name, email, addr;
                        std::cout << "Name: "; std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::getline(std::cin, name);
                        std::cout << "Email: "; std::getline(std::cin, email);
                        std::cout << "Address: "; std::getline(std::cin, addr);
                        clearScreen();
                        std::cout << "--- Result ---\n";
                        int id = -1;
                        {
                            ScopedTimer t("Add member");
                            id = myLib.addMember(name, email, addr);
                        }
                        std::cout << "Member added with ID " << id << ".";
                        std::cout << "\n[Enter] Back";
                        std::cin.get();
                    } else if (mc==2) {
                        clearScreen();
                        std::cout << "--- Members: Edit ---\n";
                        int id; std::cout << "Member ID: "; std::cin >> id;
                        const Borrower* found=nullptr; for (const auto& b: myLib.getBorrowers()) if (b.memberID==id){found=&b;break;}
                        clearScreen();
                        std::cout << "--- Result ---\n";
                        if (!found) { std::cout << "Not found."; }
                        else {
                            std::string name,email,addr;
                            std::cout << "New name (Enter = keep, current: " << found->name << "): ";
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::getline(std::cin, name);
                            std::cout << "New email (current: " << found->email << "): "; std::getline(std::cin, email);
                            std::cout << "New address (current: " << found->address << "): "; std::getline(std::cin, addr);
                            myLib.editMember(id, name, email, addr);
                            std::cout << "Saved.";
                        }
                        std::cout << "\n[Enter] Back";
                        std::cin.get();
                    } else if (mc==3) {
                        clearScreen();
                        std::cout << "--- Members: Status ---\n";
                        int id; std::cout << "Member ID: "; std::cin >> id;
                        BorrowerStatus cur = BorrowerStatus::Active; bool ok=false; for (const auto& b: myLib.getBorrowers()) if (b.memberID==id){cur=b.status; ok=true; break;}
                        clearScreen();
                        std::cout << "--- Result ---\n";
                        if (!ok) { std::cout << "Not found."; }
                        else {
                            std::cout << "Current status: " << (cur==BorrowerStatus::Active?"Active":"Blocked") << "\n";
                            std::cout << "[1] Activate\n[2] Deactivate\nChoice: "; int t=0; std::cin >> t;
                            myLib.setMemberStatus(id, (t==1?BorrowerStatus::Active:BorrowerStatus::Blocked));
                            std::cout << "Status updated.";
                        }
                        std::cout << "\n[Enter] Back";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (mc==4) {
                        // Mitglieder: Einheitliche Suche (ID exakt, Email/Name Teilstring), Paging 10er
                        clearScreen();
                        std::cout << "--- Members: Search ---\n";
                        std::cout << "Search (ID/Email/Name): ";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::string q; std::getline(std::cin, q);
                        auto t0_m = std::chrono::steady_clock::now();
                        std::vector<int> ids = myLib.searchBorrowersIDs(q);
                        double ms_m = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::steady_clock::now() - t0_m).count();
                        size_t shown=0;
                        while (true) {
                            clearScreen();
                            std::cout.setf(std::ios::fixed); std::cout.precision(1);
                            std::cout << "--- Results (" << ids.size() << ")";
                            if (isBenchmarkEnabled()) std::cout << " — " << ms_m << " ms";
                            std::cout << " ---\n";
                            std::cout.unsetf(std::ios::floatfield);

                            const size_t W_ID=6, W_NAME=24, W_MAIL=28, W_STATUS=8, W_REG=12, W_OPEN=6;
                            std::cout << padOrEllipsize("ID", W_ID) << " "
                                      << padOrEllipsize("Name", W_NAME) << " "
                                      << padOrEllipsize("Email", W_MAIL) << " "
                                      << padOrEllipsize("Status", W_STATUS) << " "
                                      << padOrEllipsize("Registered", W_REG) << " "
                                      << padOrEllipsize("Open", W_OPEN) << "\n";
                            std::cout << std::string(W_ID+1+W_NAME+1+W_MAIL+1+W_STATUS+1+W_REG+1+W_OPEN, '-') << "\n";

                            size_t toShow = std::min<size_t>(10, ids.size()-shown);
                            for (size_t i=0;i<toShow;++i) {
                                int mid = ids[shown+i];
                                const Borrower* m=nullptr; for (const auto& b: myLib.getBorrowers()) if (b.memberID==mid){ m=&b; break; }
                                if (m) {
                                    // Offene Ausleihen zählen
                                    int open = 0; for (const auto& l : myLib.getLoans()) if (l.borrowerID==m->memberID && l.returnDate==0) ++open;
                                    std::cout << padOrEllipsize(std::to_string(m->memberID), W_ID) << " "
                                              << padOrEllipsize(m->name, W_NAME) << " "
                                              << padOrEllipsize(m->email, W_MAIL) << " "
                                              << padOrEllipsize(m->status==BorrowerStatus::Active?"Active":"Blocked", W_STATUS) << " "
                                              << padOrEllipsize(dateToString(m->registrationDate), W_REG) << " "
                                              << padOrEllipsize(std::to_string(open), W_OPEN) << "\n";
                                }
                            }
                            shown += toShow;
                            if (shown >= ids.size()) {
                                std::cout << "\n[Enter] Back";
                                std::cin.get();
                                break;
                            } else {
                                std::cout << "\n[m] Load more  |  [Enter] Back: ";
                                std::string cmd; std::getline(std::cin, cmd);
                                if (cmd != "m" && cmd != "M" && cmd != "+") break;
                            }
                        }
                    } else if (mc==0) {
                        back = true; // keine zusätzliche Enter-Pause hier
                    }
                }
                break;
            }
            case 3: { // REPORTS (persistentes Untermenü ohne runAction)
                bool back = false;
                while (!back) {
                    clearScreen();
                    std::cout << "--- Reports ---\n"
                              << "[1] Daily report (today)\n[2] Daily report (enter date)\n[3] Due list (overdue)\n";
                    printMenuFooter("Back");
                    std::cout << "Choice: ";
                    int rc=0; std::cin >> rc;
                    if (rc==1) {
                        clearScreen();
                        std::cout << "--- Reports: Daily Report (today) ---\n";
                        {
                            ScopedTimer t("Daily report (today)");
                            myLib.showDailyReport();
                        }
                        std::cout << "\n[Enter] Zurueck";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (rc==2) {
                        clearScreen();
                        std::cout << "--- Reports: Daily Report (date) ---\n";
                        std::cout << "Date (DD-MM-YYYY): ";
                        std::string ds; std::cin >> ds;
                        std::tm t{}; t.tm_isdst = -1;
                        if (ds.size()>=10) {
                            // Erwartetes Format: TT-MM-YYYY
                            t.tm_mday = std::stoi(ds.substr(0,2));
                            t.tm_mon  = std::stoi(ds.substr(3,2)) - 1;
                            t.tm_year = std::stoi(ds.substr(6,4)) - 1900;
                            auto day = std::mktime(&t);
                            clearScreen();
                            std::cout << "--- Reports: Daily Report for " << ds << " ---\n";
                            {
                                ScopedTimer t("Daily report (date)");
                                myLib.showDailyReport(day);
                            }
                        } else {
                            std::cout << "Invalid format. Please enter DD-MM-YYYY.";
                        }
                        std::cout << "\n[Enter] Zurueck";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (rc==3) {
                        std::cout << "How many entries to show? ";
                        size_t n=20; std::cin >> n;
                        std::vector<Loan> list;
                        {
                            ScopedTimer t("Due list");
                            list = myLib.getDueReport(n);
                        }
                        clearScreen();
                        std::cout << "=== Due List (open loans) ===\n";
                        auto now = std::time(nullptr);
                        for (const auto& l : list) {
                            const Book* b=nullptr; const Borrower* m=nullptr;
                            for (const auto& bk : myLib.getBooks()) if (bk.inventoryID==l.bookInventoryID) { b=&bk; break; }
                            for (const auto& br : myLib.getBorrowers()) if (br.memberID==l.borrowerID) { m=&br; break; }
                            long daysLeft = (long)((long long)l.dueDate - (long long)now) / (24*60*60);
                            std::cout << "  " << (b?b->title:("Book#"+std::to_string(l.bookInventoryID)))
                                      << " -> " << (m?m->name:("Member#"+std::to_string(l.borrowerID)))
                                      << " | due: " << dateToString(l.dueDate)
                                      << " (" << daysLeft << " days)\n";
                        }
                        std::cout << "\n[Enter] Zurueck";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (rc==0) {
                        back = true; // keine Enter-Pause
                    }
                }
                break;
            }
            case 4: { // Mitarbeiter verwalten (nur Admin) – persistentes Untermenü
                const Employee* user = myLib.getCurrentUser();
                if (!user || user->role != Role::Admin) {
                    runAction("Staff", [&]{ std::cout << "Only admins can access this area."; });
                    break;
                }
                bool back = false;
                while (!back) {
                    clearScreen();
                    std::cout << "--- Staff Management ---\n"
                              << "[1] List\n[2] Add\n[3] Deactivate\n[4] Reactivate\n[5] Reset password\n";
                    printMenuFooter("Back");
                    std::cout << "Choice: ";
                    int c=0; std::cin >> c;
                    if (c==1) {
                        clearScreen();
                        const auto& emps = myLib.getEmployees();
                        std::cout << "--- Staff (" << emps.size() << ") ---\n";
                        for (const auto& e : emps) {
                            std::cout << "[ID:" << e.employeeID << "] " << e.username << " - " << e.fullName
                                      << " | Role: " << (e.role==Role::Admin?"Admin":"Staff")
                                      << " | " << (e.isActive?"Active":"Inactive") << "\n";
                        }
                        std::cout << "\n[Enter] Back";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (c==2) {
                        clearScreen();
                        std::cout << "--- Staff: Add ---\n";
                        std::string un, fn, pw; int r;
                        std::cout << "Username: "; std::cin >> un;
                        std::cout << "Full name: "; std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::getline(std::cin, fn);
                        std::cout << "Role (0=Admin,1=Staff): "; std::cin >> r;
                        std::cout << "Password: "; std::cin >> pw;
                        clearScreen();
                        std::cout << "--- Result ---\n";
                        int id = myLib.addEmployee(un, fn, r==0?Role::Admin:Role::Staff, pw);
                        if (id>0) std::cout << "Created with ID " << id << ".";
                        std::cout << "\n[Enter] Back";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (c==3) {
                        clearScreen();
                        std::cout << "--- Staff: Deactivate ---\n";
                        int id; std::cout << "ID: "; std::cin >> id;
                        clearScreen();
                        std::cout << "--- Result ---\n";
                        std::cout << (myLib.deactivateEmployee(id)?"Deactivated":"Not found/Error");
                        std::cout << "\n[Enter] Back";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (c==4) {
                        clearScreen();
                        std::cout << "--- Staff: Reactivate ---\n";
                        int id; std::cout << "ID: "; std::cin >> id;
                        clearScreen();
                        std::cout << "--- Result ---\n";
                        std::cout << (myLib.reactivateEmployee(id)?"Reactivated":"Not found/Error");
                        std::cout << "\n[Enter] Back";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (c==5) {
                        clearScreen();
                        std::cout << "--- Staff: Reset Password ---\n";
                        int id; std::string pw; std::cout << "ID: "; std::cin >> id; std::cout << "New password: "; std::cin >> pw;
                        clearScreen();
                        std::cout << "--- Result ---\n";
                        std::cout << (myLib.resetEmployeePassword(id, pw)?"Reset":"Not found/Error");
                        std::cout << "\n[Enter] Back";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (c==0) {
                        back = true;
                        // kleine Pause optional vermeiden, damit schneller zurück
                    }
                }
                break;
            }
            case 5: { // Import (Admin) – eigener Menüpunkt im Hauptmenü
                const Employee* user = myLib.getCurrentUser();
                if (!user || user->role != Role::Admin) {
                    runAction("Import", [&]{ std::cout << "Only admins can import."; });
                    break;
                }
                namespace fs = std::filesystem;
                bool back = false;
                while (!back) {
                    clearScreen();
                    std::cout << "--- Import (CSV) ---\n"
                                 "[1] Import books (books.csv)\n"
                                 "[2] Import members (members.csv)\n";
                    printMenuFooter("Back");
                    std::cout << "Choice: ";
                    int ic = 0; std::cin >> ic;
                    if (ic == 0) { back = true; break; }

                    // Wir suchen den "import" Ordner relativ zum aktuellen Verzeichnis
                    fs::path importDir = "import";
                    
                    // Fallback für lokale Entwicklung, falls man aus cmake-build-debug startet
                    if (!fs::exists(importDir) && fs::exists("../import")) {
                        importDir = "../import";
                    }

                    fs::path filePath;
                    if (ic == 1) filePath = importDir / "books.csv";
                    else if (ic == 2) filePath = importDir / "members.csv";
                    else { continue; }

                    clearScreen();
                    std::cout << "--- Ergebnis: Import ---\n";
                    if (!fs::exists(filePath)) {
                        std::cout << "CSV not found at path: " << filePath.string() << "\n";
                        std::cout << "\n[Enter] Back";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                        continue;
                    }

                    bool ok=false;
                    if (ic==1) {
                        ok = myLib.importBooksFromCSVFile(filePath.string());
                    } else if (ic==2) {
                        ok = myLib.importMembersFromCSVFile(filePath.string());
                    }

                    if (ok) {
                        myLib.saveData();
                        std::cout << "Import successful from '" << filePath.filename().string() << "'.";
                    } else {
                        std::cout << "Import failed (invalid file?).";
                    }
                    std::cout << "\n[Enter] Back";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cin.get();
                }
                break;
            }
            case 6: { // Einstellungen
                bool back = false;
                while (!back) {
                    clearScreen();
                    std::cout << "--- Settings ---\n";
                    std::cout << "Benchmark mode: " << (isBenchmarkEnabled()?"ON":"OFF") << "\n";
                    std::cout << "[1] Toggle benchmark\n";
                    printMenuFooter("Back");
                    std::cout << "Choice: ";
                    int ec=0; std::cin >> ec;
                    if (ec==0) { back=true; break; }
                    if (ec==1) {
                        setBenchmarkEnabled(!isBenchmarkEnabled());
                    }
                }
                break;
            }
            default:
                runAction("Notice", [&]{ std::cout << "Unknown option."; });
        }
    }
}