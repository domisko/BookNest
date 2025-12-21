#pragma once
#include <string>
#include <fstream>
#include <ctime>

void writeString(std::ostream& out, const std::string& str);
std::string readString(std::istream& in);
time_t addDays(time_t startTime, int days);
std::string dateToString(time_t t);

// Konsole leeren (plattformübergreifend, für „angeheftetes“ Menü)
void clearScreen();

// Sehr einfacher Demo-Hash (kein echter Sicherheitsschutz!)
// Zweck: Passwörter nicht im Klartext speichern.
std::string simpleHash(const std::string& input);

// Kleine Eingabe-Pause nur mit Enter (ohne zusätzliches Clear o. ä.)
void pauseForEnter();

// Einheitliche Menü-Fußzeile für Abmelden/Zurück mit Abstand und Trenner
void printMenuFooter(const std::string& zeroLabel = "Abmelden");