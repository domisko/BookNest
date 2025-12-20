#include "Employee.h"

Employee::Employee(int id,
                   std::string uname,
                   std::string fname,
                   std::string pwh,
                   Role r,
                   std::time_t created,
                   bool active)
        : employeeID(id), username(std::move(uname)), fullName(std::move(fname)),
          passwordHash(std::move(pwh)), role(r), createdAt(created), isActive(active) {}
