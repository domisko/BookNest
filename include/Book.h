#pragma once
#include <string>
#include <vector>
#include <ctime>

// Medienart des Mediums (für spätere Erweiterungen/Filter)
enum class MediaType {
    Book = 0,
    Magazine = 1,
    DVD = 2,
    EBook = 3,
    Other = 4
};

// Repräsentiert ein physisches Exemplar eines Mediums.
// WICHTIG: ISBN kennzeichnet die Ausgabe/Version eines Werks, NICHT das Exemplar.
// inventoryID ist die eindeutige Kennung des physischen Exemplars (mehrere Exemplare pro ISBN möglich).
class Book {
public:
    // Identität & Grunddaten
    int inventoryID;
    std::string isbn;
    std::string title;

    // Erweiterte Metadaten
    std::vector<std::string> authors;   // mehrere Autoren
    std::string publisher;              // Verlag
    std::string edition;                // Auflage/Ausgabe (frei als Text)
    std::string location;               // Standort/Signatur im Regal
    std::string genre;                  // Genre/Kategorie
    double price;                       // Preis (EUR)
    int maxLoanPeriodDays;              // maximale Ausleihdauer in Tagen (steuert Fälligkeitsdatum)
    MediaType mediaType;                // Medienart
    std::time_t createdAt;              // Erfassungszeitpunkt

    // Admin/User Tracking
    std::string createdBy;
    std::string lastModifiedBy;

    bool isAvailable;

    // Einheitlicher Konstruktor mit sinnvollen Defaults für optionale Felder
    Book(int id,
         std::string isbn,
         std::string title,
         std::vector<std::string> authors,
         std::string publisher,
         std::string location,
         std::string edition,
         std::string genre,
         double price,
         MediaType mediaType,
         int maxLoanPeriodDays = 14,
         std::time_t createdAt = std::time(nullptr),
         std::string createdBy = "system",
         std::string lastModifiedBy = "system",
         bool isAvailable = true);
};