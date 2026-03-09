# BookNest – User Guide

This guide explains how to use the "BookNest" console application.

## Startup & Login
1. Start the program (CLion: Target "BookNest"; or in the terminal `./cmake-build-debug/BookNest`).
2. Upon the first start, a default admin exists: Username `admin`, Password `admin`.
3. After a successful login, the logged-in user and their role are displayed at the top.

## Main Menu
- [1] Search Book: Search by title, ISBN, or author(s). The output includes the Inventory ID for further operations.
- [2] Borrow Book: Requires Book ID and Member ID. The due date is determined by `Book::maxLoanPeriodDays`.
- [3] Return Book: Requires Book ID. History is preserved.
- [4] Show All Books: Brief overview with availability status.
- [5] Show All Members: List of all borrowers. (Status is shown if blocked.)
- [6] Manage Employees (Admin only): Create/deactivate/reactivate employees, reset passwords.
- [7] Logout: Return to the login screen.
- [8] Generate Test Data: Adds some books/members for quick testing.
- [9] Exit: Saves data to `library.bin` and exits the app.
- [10] Reports: View reports (Daily Report, Due Report).

Note: The console is cleared before each action (pinned menu). In some IDE run consoles, the scroll history may remain visible; the display is clean in system terminals (macOS Terminal, iTerm2, Windows Terminal).

## Borrowing & Returning
- A loan creates a `Loan` entry with `loanDate` and `dueDate`. The currently logged-in employee is recorded as `performedBy`.
- A return sets the `returnDate` and updates `performedBy`.
- Blocked members cannot borrow books (Status `Blocked`).

## Reports
- Daily Report: Shows all loans and returns of the selected day, including time, involved persons, due date, and the person who performed the action (`performedBy`). Totals per `MediaType` are also provided.
- Due Report: Shows open loans sorted by remaining time (overdue first). Useful for preparing reminders.

## Employee Management
- Only admins are allowed to create, deactivate/reactivate employees, and reset passwords.
- Passwords are not stored in plain text but via a simple demo hash (not suitable for production security).

## Files & Persistence
- All data is stored binary in `library.bin`. A format version is checked upon loading. During development, the file can be deleted in case of schema changes.

## FAQ
- "Screen does not clear completely": Please start in a system terminal. IDE run consoles often buffer the history.
- "Login fails": On the first start, log in with `admin/admin`. Then change the password in the Admin menu if necessary.
