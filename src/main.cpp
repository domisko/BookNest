#include <iostream>
#include <limits> // Für cin.ignore
#include <sstream>
#include <filesystem>
#include "Library.h"
#include "Utils.h"

// Menü-Rendering und Aktions-Wrapper für „angeheftete“ Darstellung
static void drawMainMenu(bool showEmployeeMenu) {
    std::cout << "====================================\n";
    std::cout << "      BOOKNEST BIBLIOTHEK v1.0      \n";
    std::cout << "====================================\n";
    std::cout << "[1] Buecher\n";
    std::cout << "[2] Mitglieder\n";
    std::cout << "[3] Reports\n";
    if (showEmployeeMenu) {
        std::cout << "[4] Mitarbeiter (Admin)\n";
        std::cout << "[5] Import (Admin)\n";
    }
    std::cout << "[6] Einstellungen\n";
    printMenuFooter("Abmelden");
}

template<typename F>
static void runAction(const std::string& title, F body) {
    clearScreen();
    std::cout << "--- " << title << " ---\n";
    body();
    std::cout << "\nDruecken Sie Enter um fortzufahren...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

int main() {
    Library myLib;
    int choice = 0;

    // Zum Start Daten laden – bevorzugt in ./data (für Docker-Volumes),
    // dann Fallback ../data, anschließend klassisch ./ und ../
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
        std::cout << "=== Anmeldung erforderlich ===\n";
        std::cout << "Benutzername: ";
        std::string uname; std::cin >> uname;
        std::cout << "Passwort: ";
        std::string pwd; std::cin >> pwd;
        if (myLib.authenticate(uname, pwd)) {
            std::cout << "Anmeldung erfolgreich.\n";
            break;
        } else {
            std::cout << "Anmeldung fehlgeschlagen. Tipp: Default ist admin/admin.\n";
            std::cout << "Druecken Sie Enter um es erneut zu versuchen...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
        }
    }

    while (true) {
        clearScreen();
        const Employee* user = myLib.getCurrentUser();
        if (user) {
            std::cout << "Angemeldet als: " << user->username << " (" << (user->role == Role::Admin ? "Admin" : "Mitarbeiter") << ")\n";
        } else {
            std::cout << "[Nicht angemeldet]\n";
        }
        drawMainMenu(user && user->role == Role::Admin);
        std::cout << "Ihre Wahl: ";

        // Eingabe lesen und validieren
        if (!(std::cin >> choice)) {
            std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben.\n";
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
                    std::cout << "=== Anmeldung erforderlich ===\nBenutzername: ";
                    std::string un; std::cin >> un; std::cout << "Passwort: "; std::string pw; std::cin >> pw;
                    if (myLib.authenticate(un, pw)) break;
                    std::cout << "Anmeldung fehlgeschlagen. Druecken Sie Enter...";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cin.get();
                }
                break;
            }
            case 1: { // BUECHER Submenü (ohne runAction, um doppelte Enter-Pausen zu vermeiden)
                bool back = false;
                while (!back) {
                    clearScreen();
                    std::cout << "--- Buecher ---\n"
                              << "[1] Suchen\n[2] Ausleihen\n[3] Zurueckgeben\n";
                    printMenuFooter("Zurueck");
                    std::cout << "Auswahl: ";
                    int bc=0; std::cin >> bc;
                    if (bc==1) {
                        // Bücher: Suche mit Paging (10er Schritte)
                        clearScreen();
                        std::cout << "--- Buecher: Suche ---\n";
                        std::cout << "Suchbegriff (Titel/ISBN/Autor): ";
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
                            std::cout << "--- Suchergebnisse (" << matches.size() << ")";
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
                                      << padOrEllipsize("Titel", W_TITLE) << " "
                                      << padOrEllipsize("Autoren", W_AUTH) << " "
                                      << padOrEllipsize("Medienart", W_MT) << " "
                                      << padOrEllipsize("Verfuegbar", W_AV) << " "
                                      << padOrEllipsize("Location", W_LOC) << " "
                                      << padOrEllipsize("Verf. ab", W_AB) << "\n";
                            std::cout << std::string(W_ID+1+W_TITLE+1+W_AUTH+1+W_MT+1+W_AV+1+W_LOC+1+W_AB, '-') << "\n";

                            size_t toShow = std::min<size_t>(10, matches.size() - shown);
                            for (size_t i = 0; i < toShow; ++i) {
                                int bid = matches[shown + i];
                                const Book* b = nullptr; for (const auto& x : myLib.getBooks()) if (x.inventoryID==bid){ b=&x; break; }
                                if (b) {
                                    std::ostringstream oss; for (size_t k=0;k<b->authors.size();++k){ if(k) oss<<", "; oss<<b->authors[k]; }
                                    std::string avail = b->isAvailable ? "Ja" : "Nein";
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
                                std::cout << "\n[Enter] Zurueck";
                                std::cin.get();
                                break;
                            } else {
                                std::cout << "\n[m] Mehr anzeigen  |  [Enter] Zurueck: ";
                                std::string cmd; std::getline(std::cin, cmd);
                                if (cmd != "m" && cmd != "M" && cmd != "+") break;
                            }
                        }
                    } else if (bc==2) {
                        int bookID, memberID;
                        clearScreen();
                        std::cout << "--- Buecher: Ausleihen ---\n";
                        std::cout << "Buch-ID: "; std::cin >> bookID;
                        std::cout << "Mitglieds-ID: "; std::cin >> memberID;
                        clearScreen();
                        std::cout << "--- Ergebnis ---\n";
                        myLib.borrowBook(bookID, memberID);
                        std::cout << "\n[Enter] Zurueck";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (bc==3) {
                        int bookID;
                        clearScreen();
                        std::cout << "--- Buecher: Rueckgabe ---\n";
                        std::cout << "Buch-ID: "; std::cin >> bookID;
                        clearScreen();
                        std::cout << "--- Ergebnis ---\n";
                        myLib.returnBook(bookID);
                        std::cout << "\n[Enter] Zurueck";
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
                    std::cout << "--- Mitglieder ---\n"
                              << "[1] Anlegen\n[2] Bearbeiten\n[3] Aktivieren/Deaktivieren\n[4] Suchen (ID/Email/Name)\n";
                    printMenuFooter("Zurueck");
                    std::cout << "Auswahl: ";
                    int mc=0; std::cin >> mc;
                    if (mc==1) {
                        // Nur Formular + Ergebnis, kein Untermenü im Hintergrund
                        clearScreen();
                        std::cout << "--- Mitglieder: Anlegen ---\n";
                        std::string name, email, addr;
                        std::cout << "Name: "; std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::getline(std::cin, name);
                        std::cout << "Email: "; std::getline(std::cin, email);
                        std::cout << "Adresse: "; std::getline(std::cin, addr);
                        clearScreen();
                        std::cout << "--- Ergebnis ---\n";
                        int id = -1;
                        {
                            ScopedTimer t("Mitglied anlegen");
                            id = myLib.addMember(name, email, addr);
                        }
                        std::cout << "Mitglied angelegt mit ID " << id << ".";
                        std::cout << "\n[Enter] Zurueck";
                        std::cin.get();
                    } else if (mc==2) {
                        clearScreen();
                        std::cout << "--- Mitglieder: Bearbeiten ---\n";
                        int id; std::cout << "Member-ID: "; std::cin >> id;
                        const Borrower* found=nullptr; for (const auto& b: myLib.getBorrowers()) if (b.memberID==id){found=&b;break;}
                        clearScreen();
                        std::cout << "--- Ergebnis ---\n";
                        if (!found) { std::cout << "Nicht gefunden."; }
                        else {
                            std::string name,email,addr;
                            std::cout << "Neuer Name (Enter = unveraendert, aktuell: " << found->name << "): ";
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::getline(std::cin, name);
                            std::cout << "Neue Email (aktuell: " << found->email << "): "; std::getline(std::cin, email);
                            std::cout << "Neue Adresse (aktuell: " << found->address << "): "; std::getline(std::cin, addr);
                            myLib.editMember(id, name, email, addr);
                            std::cout << "Gespeichert.";
                        }
                        std::cout << "\n[Enter] Zurueck";
                        std::cin.get();
                    } else if (mc==3) {
                        clearScreen();
                        std::cout << "--- Mitglieder: Status ---\n";
                        int id; std::cout << "Member-ID: "; std::cin >> id;
                        BorrowerStatus cur = BorrowerStatus::Active; bool ok=false; for (const auto& b: myLib.getBorrowers()) if (b.memberID==id){cur=b.status; ok=true; break;}
                        clearScreen();
                        std::cout << "--- Ergebnis ---\n";
                        if (!ok) { std::cout << "Nicht gefunden."; }
                        else {
                            std::cout << "Aktueller Status: " << (cur==BorrowerStatus::Active?"Active":"Blocked") << "\n";
                            std::cout << "[1] Aktivieren\n[2] Deaktivieren\nAuswahl: "; int t=0; std::cin >> t;
                            myLib.setMemberStatus(id, (t==1?BorrowerStatus::Active:BorrowerStatus::Blocked));
                            std::cout << "Status aktualisiert.";
                        }
                        std::cout << "\n[Enter] Zurueck";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (mc==4) {
                        // Mitglieder: Einheitliche Suche (ID exakt, Email/Name Teilstring), Paging 10er
                        clearScreen();
                        std::cout << "--- Mitglieder: Suche ---\n";
                        std::cout << "Suchbegriff (ID/Email/Name): ";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::string q; std::getline(std::cin, q);
                        auto t0_m = std::chrono::steady_clock::now();
                        std::vector<int> ids = myLib.searchBorrowersIDs(q);
                        double ms_m = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::steady_clock::now() - t0_m).count();
                        size_t shown=0;
                        while (true) {
                            clearScreen();
                            std::cout.setf(std::ios::fixed); std::cout.precision(1);
                            std::cout << "--- Treffer (" << ids.size() << ")";
                            if (isBenchmarkEnabled()) std::cout << " — " << ms_m << " ms";
                            std::cout << " ---\n";
                            std::cout.unsetf(std::ios::floatfield);

                            const size_t W_ID=6, W_NAME=24, W_MAIL=28, W_STATUS=8, W_REG=12, W_OPEN=6;
                            std::cout << padOrEllipsize("ID", W_ID) << " "
                                      << padOrEllipsize("Name", W_NAME) << " "
                                      << padOrEllipsize("Email", W_MAIL) << " "
                                      << padOrEllipsize("Status", W_STATUS) << " "
                                      << padOrEllipsize("Registriert", W_REG) << " "
                                      << padOrEllipsize("Offen", W_OPEN) << "\n";
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
                                std::cout << "\n[Enter] Zurueck";
                                std::cin.get();
                                break;
                            } else {
                                std::cout << "\n[m] Mehr anzeigen  |  [Enter] Zurueck: ";
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
                              << "[1] Tagesbericht (heute)\n[2] Tagesbericht (Datum eingeben)\n[3] Rueckgabeliste (Faelligkeiten)\n";
                    printMenuFooter("Zurueck");
                    std::cout << "Auswahl: ";
                    int rc=0; std::cin >> rc;
                    if (rc==1) {
                        clearScreen();
                        std::cout << "--- Reports: Tagesbericht (heute) ---\n";
                        {
                            ScopedTimer t("Tagesbericht (heute)");
                            myLib.showDailyReport();
                        }
                        std::cout << "\n[Enter] Zurueck";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (rc==2) {
                        clearScreen();
                        std::cout << "--- Reports: Tagesbericht (Datum) ---\n";
                        std::cout << "Datum (TT-MM-YYYY): ";
                        std::string ds; std::cin >> ds;
                        std::tm t{}; t.tm_isdst = -1;
                        if (ds.size()>=10) {
                            // Erwartetes Format: TT-MM-YYYY
                            t.tm_mday = std::stoi(ds.substr(0,2));
                            t.tm_mon  = std::stoi(ds.substr(3,2)) - 1;
                            t.tm_year = std::stoi(ds.substr(6,4)) - 1900;
                            auto day = std::mktime(&t);
                            clearScreen();
                            std::cout << "--- Reports: Tagesbericht fuer " << ds << " ---\n";
                            {
                                ScopedTimer t("Tagesbericht (Datum)");
                                myLib.showDailyReport(day);
                            }
                        } else {
                            std::cout << "Ungueltiges Format. Bitte TT-MM-YYYY eingeben.";
                        }
                        std::cout << "\n[Enter] Zurueck";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (rc==3) {
                        std::cout << "Wie viele Eintraege anzeigen? ";
                        size_t n=20; std::cin >> n;
                        std::vector<Loan> list;
                        {
                            ScopedTimer t("Faelligkeitsliste");
                            list = myLib.getDueReport(n);
                        }
                        clearScreen();
                        std::cout << "=== Rueckgabeliste (offene Ausleihen) ===\n";
                        auto now = std::time(nullptr);
                        for (const auto& l : list) {
                            const Book* b=nullptr; const Borrower* m=nullptr;
                            for (const auto& bk : myLib.getBooks()) if (bk.inventoryID==l.bookInventoryID) { b=&bk; break; }
                            for (const auto& br : myLib.getBorrowers()) if (br.memberID==l.borrowerID) { m=&br; break; }
                            long daysLeft = (long)((long long)l.dueDate - (long long)now) / (24*60*60);
                            std::cout << "  " << (b?b->title:("Buch#"+std::to_string(l.bookInventoryID)))
                                      << " -> " << (m?m->name:("Mitglied#"+std::to_string(l.borrowerID)))
                                      << " | faellig am " << dateToString(l.dueDate)
                                      << " (" << daysLeft << " Tage)\n";
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
                    runAction("Mitarbeiter", [&]{ std::cout << "Nur Admins duerfen diesen Bereich aufrufen."; });
                    break;
                }
                bool back = false;
                while (!back) {
                    // Wichtig: Hier KEIN runAction um doppelte Enter-Pausen zu vermeiden,
                    // da wir in einigen Fällen selbst eine Enter-Pause anzeigen.
                    clearScreen();
                    std::cout << "--- Mitarbeiter verwalten ---\n"
                              << "[1] Liste\n[2] Anlegen\n[3] Deaktivieren\n[4] Reaktivieren\n[5] Passwort zuruecksetzen\n";
                    printMenuFooter("Zurueck");
                    std::cout << "Auswahl: ";
                    int c=0; std::cin >> c;
                    if (c==1) {
                        // Spezielle Anzeige: Nur Liste + eigene Enter-Pause
                        clearScreen();
                        const auto& emps = myLib.getEmployees();
                        std::cout << "--- Mitarbeiter (" << emps.size() << ") ---\n";
                        for (const auto& e : emps) {
                            std::cout << "[ID:" << e.employeeID << "] " << e.username << " - " << e.fullName
                                      << " | Rolle: " << (e.role==Role::Admin?"Admin":"Mitarbeiter")
                                      << " | " << (e.isActive?"Aktiv":"Inaktiv") << "\n";
                        }
                        std::cout << "\n[Enter] Zurueck";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                        // Danach wird durch die while-Schleife das Untermenü erneut angezeigt
                    } else if (c==2) { // Anlegen
                        clearScreen();
                        std::cout << "--- Mitarbeiter: Anlegen ---\n";
                        std::string un, fn, pw; int r;
                        std::cout << "Username: "; std::cin >> un;
                        std::cout << "Voller Name: "; std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::getline(std::cin, fn);
                        std::cout << "Rolle (0=Admin,1=Staff): "; std::cin >> r;
                        std::cout << "Passwort: "; std::cin >> pw;
                        clearScreen();
                        std::cout << "--- Ergebnis ---\n";
                        int id = myLib.addEmployee(un, fn, r==0?Role::Admin:Role::Staff, pw);
                        if (id>0) std::cout << "Angelegt mit ID " << id << ".";
                        std::cout << "\n[Enter] Zurueck";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (c==3) { // Deaktivieren
                        clearScreen();
                        std::cout << "--- Mitarbeiter: Deaktivieren ---\n";
                        int id; std::cout << "ID: "; std::cin >> id;
                        clearScreen();
                        std::cout << "--- Ergebnis ---\n";
                        std::cout << (myLib.deactivateEmployee(id)?"Deaktiviert":"Nicht gefunden/Fehler");
                        std::cout << "\n[Enter] Zurueck";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (c==4) { // Reaktivieren
                        clearScreen();
                        std::cout << "--- Mitarbeiter: Reaktivieren ---\n";
                        int id; std::cout << "ID: "; std::cin >> id;
                        clearScreen();
                        std::cout << "--- Ergebnis ---\n";
                        std::cout << (myLib.reactivateEmployee(id)?"Reaktiviert":"Nicht gefunden/Fehler");
                        std::cout << "\n[Enter] Zurueck";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (c==5) { // Passwort zurücksetzen
                        clearScreen();
                        std::cout << "--- Mitarbeiter: Passwort zuruecksetzen ---\n";
                        int id; std::string pw; std::cout << "ID: "; std::cin >> id; std::cout << "Neues Passwort: "; std::cin >> pw;
                        clearScreen();
                        std::cout << "--- Ergebnis ---\n";
                        std::cout << (myLib.resetEmployeePassword(id, pw)?"Zurueckgesetzt":"Nicht gefunden/Fehler");
                        std::cout << "\n[Enter] Zurueck";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    } else if (c==0) {
                        back = true;
                         std::cout << "Zurueck zum Hauptmenue";
                        // kleine Pause optional vermeiden, damit schneller zurück
                    }
                }
                break;
            }
            case 5: { // Import (Admin) – eigener Menüpunkt im Hauptmenü
                const Employee* user = myLib.getCurrentUser();
                if (!user || user->role != Role::Admin) {
                    runAction("Import", [&]{ std::cout << "Nur Admins duerfen importieren."; });
                    break;
                }
                namespace fs = std::filesystem;
                bool back = false;
                while (!back) {
                    clearScreen();
                    std::cout << "--- Import (CSV) ---\n"
                                 "[1] Buecher importieren (books.csv)\n"
                                 "[2] Mitglieder importieren (members.csv)\n";
                    printMenuFooter("Zurueck");
                    std::cout << "Auswahl: ";
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
                        std::cout << "CSV nicht gefunden unter Pfad: " << filePath.string() << "\n";
                        std::cout << "\n[Enter] Zurueck";
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
                        std::cout << "Import erfolgreich aus '" << filePath.filename().string() << "'.";
                    } else {
                        std::cout << "Import fehlgeschlagen (Datei ungueltig?).";
                    }
                    std::cout << "\n[Enter] Zurueck";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cin.get();
                }
                break;
            }
            case 6: { // Einstellungen
                bool back = false;
                while (!back) {
                    clearScreen();
                    std::cout << "--- Einstellungen ---\n";
                    std::cout << "Benchmark-Modus: " << (isBenchmarkEnabled()?"AN":"AUS") << "\n";
                    std::cout << "[1] Benchmark umschalten\n";
                    printMenuFooter("Zurueck");
                    std::cout << "Auswahl: ";
                    int ec=0; std::cin >> ec;
                    if (ec==0) { back=true; break; }
                    if (ec==1) {
                        setBenchmarkEnabled(!isBenchmarkEnabled());
                    }
                }
                break;
            }
            default:
                runAction("Hinweis", [&]{ std::cout << "Unbekannte Option."; });
        }
    }
}