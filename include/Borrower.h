#pragma once
#include <string>
#include <ctime>

// Status eines Mitglieds: darf ausleihen (Active) oder ist gesperrt (Blocked)
enum class BorrowerStatus {
    Active = 0,
    Blocked = 1
};

class Borrower {
public:
    int memberID;
    std::string name;
    std::string email;
    std::string address;
    std::time_t registrationDate; // Zeitpunkt der Registrierung
    BorrowerStatus status;        // Active/Blocked

    Borrower(int id, std::string name, std::string email, std::string address,
             std::time_t registrationDate = std::time(nullptr),
             BorrowerStatus status = BorrowerStatus::Active);
};