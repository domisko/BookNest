# BookNest – Anwenderhandbuch

Dieses Handbuch erklärt die Nutzung der Konsolen‑Anwendung „BookNest“.

## Start & Anmeldung
1. Programm starten (CLion: Target „BookNest“; oder im Terminal `./cmake-build-debug/BookNest`).
2. Beim ersten Start existiert ein Default‑Admin: Benutzername `admin`, Passwort `admin`.
3. Nach erfolgreichem Login wird oben der angemeldete Benutzer und die Rolle angezeigt.

## Hauptmenü
- [1] Buch suchen: Sucht nach Titel, ISBN oder Autor(en). Ausgabe enthält Inventar‑ID zum weiteren Arbeiten.
- [2] Buch ausleihen: Benötigt Buch‑ID und Mitglieds‑ID. Die Fälligkeit richtet sich nach `Book::maxLoanPeriodDays`.
- [3] Buch zurückgeben: Benötigt Buch‑ID. Historie bleibt erhalten.
- [4] Alle Bücher anzeigen: Kurzübersicht mit Verfügbarkeitsstatus.
- [5] Alle Mitglieder anzeigen: Liste aller Borrower. (Status wird angezeigt, sofern blockiert.)
- [6] Mitarbeiter verwalten (nur Admin): Mitarbeitende anlegen/deaktivieren/reaktivieren, Passwort zurücksetzen.
- [7] Abmelden: Zurück zum Login.
- [8] Testdaten generieren: Fügt einige Bücher/Mitglieder zum schnellen Testen hinzu.
- [9] Beenden: Speichert Daten nach `library.bin` und beendet die App.
- [10] Reports: Berichte anzeigen (Tagesbericht, Fälligkeitsliste).

Hinweis: Die Konsole wird vor jeder Aktion geleert (angeheftetes Menü). In einigen IDE‑Konsolen kann der Scroll‑Verlauf sichtbar bleiben; im System‑Terminal (macOS Terminal, iTerm2, Windows Terminal) ist die Darstellung sauber.

## Ausleihe & Rückgabe
- Eine Ausleihe erzeugt einen `Loan`‑Eintrag mit `loanDate` und `dueDate` (Fälligkeit). Der/die aktuell angemeldete Mitarbeiter:in wird als `performedBy` vermerkt.
- Eine Rückgabe setzt `returnDate` und aktualisiert `performedBy`.
- Blockierte Mitglieder können nicht ausleihen (Status `Blocked`).

## Reports
- Tagesbericht: Zeigt alle Ausleihen und Rückgaben des gewählten Tages, inklusive Uhrzeit, beteiligten Personen, Fälligkeit und Ausführendem (`performedBy`). Außerdem werden Summen pro `MediaType` ausgegeben.
- Fälligkeitsliste: Zeigt offene Ausleihen sortiert nach verbleibender Zeit (überfällige zuerst). Nützlich, um Mahnungen vorzubereiten.

## Mitarbeiterverwaltung
- Nur Admins dürfen Mitarbeitende anlegen, deaktivieren/reaktivieren und Passwörter zurücksetzen.
- Passwörter werden nicht im Klartext gespeichert, sondern über einen einfachen Demo‑Hash (nicht sicherheitsgeeignet für Produktion).

## Dateien & Persistenz
- Alle Daten werden binär in `library.bin` gespeichert. Beim Laden wird eine Format‑Version geprüft. In der Entwicklung kann die Datei bei Schema‑Änderungen gelöscht werden.

## FAQ
- „Bildschirm leert sich nicht vollständig“: Bitte im System‑Terminal starten. IDE‑Run‑Konsolen puffern häufig den Verlauf.
- „Login schlägt fehl“: Beim ersten Start mit `admin/admin` anmelden. Danach ggf. Passwort im Admin‑Menü ändern.
