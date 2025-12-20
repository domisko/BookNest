#pragma once
#include <string>
#include <ctime>

enum class Role {
    Admin = 0,
    Staff = 1
};

class Employee {
public:
    int employeeID{};
    std::string username;
    std::string fullName;
    std::string passwordHash; // Demo: einfacher Hash, kein echter Schutz
    Role role{Role::Staff};
    std::time_t createdAt{};
    bool isActive{true};

    Employee() = default;
    Employee(int id,
             std::string username,
             std::string fullName,
             std::string passwordHash,
             Role role,
             std::time_t createdAt,
             bool isActive);
};
