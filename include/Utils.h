#pragma once
#include <string>
#include <fstream>
#include <ctime>
#include <chrono>

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

// Hilfsformatierung: auf Breite auffüllen oder mit … kürzen
std::string padOrEllipsize(const std::string& s, size_t width);

// Benchmark-Schalter (global) und Helfer
void setBenchmarkEnabled(bool enabled);
bool isBenchmarkEnabled();

// RAII-Timer, der bei Zerstörung die verstrichene Zeit in ms ausgibt,
// wenn Benchmark aktiviert ist. Ausgabeformat: "(Label in X.Y ms)".
class ScopedTimer {
public:
    explicit ScopedTimer(const std::string& label);
    ~ScopedTimer();
private:
    std::string label_;
    std::chrono::steady_clock::time_point start_;
};