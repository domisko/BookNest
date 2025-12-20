#include "Loan.h"
#include "Utils.h"

Loan::Loan(int id, int bID, int mID)
    : loanID(id), bookInventoryID(bID), borrowerID(mID), returnDate(0), performedBy("") {
    loanDate = std::time(nullptr);
    // Default-Fall: 14 Tage, bleibt für bestehenden Code erhalten
    dueDate = addDays(loanDate, 14);
}

// Neuer Konstruktor: nutzt eine vom Buch vorgegebene Ausleihdauer
Loan::Loan(int id, int bID, int mID, int loanPeriodDays)
    : loanID(id), bookInventoryID(bID), borrowerID(mID), returnDate(0), performedBy("") {
    loanDate = std::time(nullptr);
    if (loanPeriodDays <= 0) loanPeriodDays = 14;
    dueDate = addDays(loanDate, loanPeriodDays);
}

bool Loan::isReturned() const {
    return returnDate != 0;
}