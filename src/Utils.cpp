#include "Utils.h"
#include <iomanip>
#include <sstream>
#include <iostream>
#include <limits>
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
    // Ausgabeformat auf TT-MM-YYYY umgestellt (deutsches Datumsformat)
    std::strftime(buf, sizeof(buf), "%d-%m-%Y", std::localtime(&t));
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