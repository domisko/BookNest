#include "Borrower.h"

Borrower::Borrower(int id, std::string name, std::string email, std::string address,
                   std::time_t registrationDate, BorrowerStatus status)
    : memberID(id),
      name(std::move(name)),
      email(std::move(email)),
      address(std::move(address)),
      registrationDate(registrationDate),
      status(status) {}