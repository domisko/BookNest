#include "Utils.h"
#include <iomanip>
#include <sstream>
#include <iostream>
#include <limits>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#endif

void writeString(std::ostream& out, const std::string& str) {
    size_t size = str.size();
    out.write(reinterpret_cast<char *>(&size), sizeof(size));
    out.write(str.data(), size);
}

std::string readString(std::istream& in) {
    size_t size;
    in.read(reinterpret_cast<char *>(&size), sizeof(size));
    std::string str(size, ' ');
    in.read(&str[0], size);
    return str;
}

time_t addDays(const time_t startTime, int days) {
    return startTime + (days * 24 * 60 * 60);
}

std::string dateToString(const time_t t) {
    char buf[20];
    // ISO-Format YYYY-MM-DD (entspricht Erwartung der Tests und ist sortierbar)
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&t));
    return {buf};
}

// Plattformübergreifendes Bildschirm-Löschen für angeheftetes Menü
// Hinweis: Viele Terminals behalten einen Scrollback-Puffer. ESC[3J löscht diesen.
// Reihenfolge: 3J (Scrollback), 2J (Screen), H (Cursor Home)
void clearScreen() {
    // 1) Versuche ANSI (funktioniert auf macOS/Linux und modernen Windows-Terminals)
    std::cout << "\x1b[3J\x1b[2J\x1b[H" << std::flush; // Scrollback + Screen + Home

#ifdef _WIN32
    // 2) Falls klassisches Windows ohne VT: versuche Virtual Terminal Processing zu aktivieren
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
                DWORD newMode = mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                if (SetConsoleMode(hOut, newMode)) {
                    // Nochmal ANSI senden (inkl. Scrollback leeren)
                    std::cout << "\x1b[3J\x1b[2J\x1b[H" << std::flush;
                    return;
                }
            } else {
                // ANSI war bereits aktiv
                return;
            }
        }
    }
    // 3) Fallback
    std::system("cls");
#else
    // Unix-Fallback wäre system("clear"), meist nicht nötig, daher optional
    // std::system("clear");
#endif
}

// Sehr einfacher Demo-Hash: FNV-1a 64-bit Variante übergeben als hex-String
// Hinweis: Nur Demo-Zwecke; NICHT für echte Passwörter verwenden.
std::string simpleHash(const std::string& input) {
    const uint64_t FNV_OFFSET = 1469598103934665603ull;
    const uint64_t FNV_PRIME  = 1099511628211ull;
    uint64_t hash = FNV_OFFSET;
    for (unsigned char c : input) {
        hash ^= c;
        hash *= FNV_PRIME;
    }
    std::ostringstream oss;
    oss << std::hex << std::nouppercase << hash;
    return oss.str();
}

// Einfache Enter-Pause (zentral genutzt in Untermenüs, um doppelte Pausen zu vermeiden)
void pauseForEnter() {
    std::cout << "\nDruecken Sie Enter um fortzufahren...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void printMenuFooter(const std::string& zeroLabel) {
    std::cout << "\n[0] " << zeroLabel << "\n";
    std::cout << "------------------------------------\n";
}

std::string padOrEllipsize(const std::string& s, size_t width) {
    if (width == 0) return "";
    if (s.size() <= width) {
        std::string out = s;
        if (out.size() < width) out.append(width - out.size(), ' ');
        return out;
    }
    if (width <= 1) return s.substr(0, width);
    // Kürzen und am Ende mit … ersetzen (ASCII Punkt '.' dreimal)
    if (width <= 3) return s.substr(0, width);
    std::string out = s.substr(0, width - 3);
    out += "...";
    return out;
}

// -------------------- Benchmark-Utilities --------------------
namespace {
    bool g_benchmarkEnabled = false;
}

void setBenchmarkEnabled(bool enabled) { g_benchmarkEnabled = enabled; }
bool isBenchmarkEnabled() { return g_benchmarkEnabled; }

ScopedTimer::ScopedTimer(const std::string& label)
    : label_(label), start_(std::chrono::steady_clock::now()) {}

ScopedTimer::~ScopedTimer() {
    if (!isBenchmarkEnabled()) return;
    const auto end = std::chrono::steady_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start_).count();
    std::cout.setf(std::ios::fixed); std::cout.precision(1);
    std::cout << "(" << label_ << " in " << ms << " ms)\n";
    std::cout.unsetf(std::ios::floatfield);
}