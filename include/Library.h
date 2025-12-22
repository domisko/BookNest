#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "Book.h"
#include "Borrower.h"
#include "Loan.h"
#include "Employee.h"

class Library {
private:
    std::vector<Book> books;
    std::vector<Borrower> borrowers;
    std::vector<Loan> loans;
    std::vector<Employee> employees;

    int nextBookID = 1000;
    int nextMemberID = 1;
    int nextLoanID = 1;
    int nextEmployeeID = 1;

    // einfacher Session-Status
    int currentUserID = -1; // -1 = niemand eingeloggt

    // Pfad zur Persistenzdatei (für Tests konfigurierbar)
    std::string dataFilePath = "library.bin";

public:
    Library() = default;
    // Import-Funktionen (CSV)
    bool importBooksFromCSV(const std::string& directoryPath);
    bool importMembersFromCSV(const std::string& directoryPath);
    // Datei-basierte Import-Funktionen (für Bulk-Import mit Dateiauswahl)
    bool importBooksFromCSVFile(const std::string& filePath);
    bool importMembersFromCSVFile(const std::string& filePath);
    // Fügt ein Buch hinzu (vollständige Parameter) und gibt die vergebene Inventar-ID zurück
    int addBook(const std::string& isbn,
                const std::string& title,
                const std::vector<std::string>& authors,
                const std::string& publisher,
                const std::string& location,
                const std::string& edition,
                const std::string& genre,
                double price,
                MediaType mediaType,
                int maxLoanPeriodDays = 14,
                std::time_t createdAt = std::time(nullptr),
                const std::string& createdBy = "system",
                const std::string& lastModifiedBy = "system",
                bool isAvailable = true,
                bool doSave = true);
    // Fügt ein Mitglied hinzu und gibt die vergebene Member-ID zurück
    int addMember(const std::string& name, const std::string& email, const std::string& address, bool doSave = true);

    Book* findBookByID(int id);
    Borrower* findBorrowerByID(int id);

    // Versucht ein Buch zu verleihen. Liefert true bei Erfolg, false bei Fehler (nicht gefunden/ bereits verliehen)
    bool borrowBook(int bookID, int memberID);
    // Versucht, ein Buch zurückzunehmen. Liefert true bei Erfolg, false wenn kein offener Loan gefunden wird
    bool returnBook(int bookID);

    // Sucht nach Büchern und gibt die gefundenen Inventar-IDs zurück (zus. Konsolen-Output für die App)
    [[nodiscard]] std::vector<int> searchBooks(const std::string& query) const;
    // Reine ID-Suche ohne Konsolen-Ausgabe (für Paging im UI)
    [[nodiscard]] std::vector<int> searchBooksIDs(const std::string& query) const;

    // Mitgliedersuche (IDs zurück)
    // Vereinheitlichte Suche: ID (exakt, wenn numerisch) ODER E-Mail/Name (Teilstring, case-insensitive)
    [[nodiscard]] std::vector<int> searchBorrowersIDs(const std::string& query) const;
    [[nodiscard]] std::vector<int> searchBorrowersByEmail(const std::string& query) const;
    [[nodiscard]] std::vector<int> searchBorrowersByName(const std::string& query) const;
    void printAllBooks() const;
    void printAllMembers() const;

    // Persistenz: true bei Erfolg
    bool saveData();
    bool loadData();
    void generateDummyData();

    // Reporting
    void showDailyReport(std::time_t day = std::time(nullptr)) const; // Tagesbericht: Ausleihen/Rückgaben
    std::vector<Loan> getDueReport(size_t limit = 20) const;           // Offene Ausleihen nach Dringlichkeit

    // Test- & App-Unterstützung: Dateipfad setzen/lesen (z. B. für isolierte Tests)
    void setDataFilePath(std::string path) { dataFilePath = std::move(path); }
    [[nodiscard]] const std::string& getDataFilePath() const { return dataFilePath; }

    // Test-unterstützende Read-Only-Zugänge
    [[nodiscard]] const std::vector<Book>& getBooks() const { return books; }
    [[nodiscard]] const std::vector<Borrower>& getBorrowers() const { return borrowers; }
    [[nodiscard]] const std::vector<Loan>& getLoans() const { return loans; }
    [[nodiscard]] const std::vector<Employee>& getEmployees() const { return employees; }

    // Mitglieder-Management (für UI)
    bool editMember(int memberID, const std::string& name, const std::string& email, const std::string& address);
    bool setMemberStatus(int memberID, BorrowerStatus status);

    // Mitarbeiter-Verwaltung & Auth
    int addEmployee(const std::string& username,
                    const std::string& fullName,
                    Role role,
                    const std::string& rawPassword);
    bool deactivateEmployee(int employeeID);
    bool reactivateEmployee(int employeeID);
    bool resetEmployeePassword(int employeeID, const std::string& rawPassword);

    bool authenticate(const std::string& username, const std::string& rawPassword);
    void logout();
    [[nodiscard]] const Employee* getCurrentUser() const;
};