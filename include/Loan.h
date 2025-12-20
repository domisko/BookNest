#pragma once
#include <ctime>
#include <string>

struct Loan {
    int loanID;
    int bookInventoryID;
    int borrowerID;

    time_t loanDate;    // Wann ausgeliehen?
    time_t dueDate;     // Wann fällig?
    time_t returnDate;  // Wann zurückgegeben? (0 = noch offen)

    // Wer hat die Aktion durchgeführt (Ausleihe oder Rückgabe)?
    // Für einfache Nachvollziehbarkeit; wird bei Borrow gesetzt und bei Return aktualisiert.
    std::string performedBy;

    // Nur noch die Deklaration des Konstruktors und der Methode
    Loan(int id, int bID, int mID);
    // Überladener Konstruktor: setzt Fälligkeit anhand einer vorgegebenen Ausleihdauer (Tage)
    Loan(int id, int bID, int mID, int loanPeriodDays);
    [[nodiscard]] bool isReturned() const;
};