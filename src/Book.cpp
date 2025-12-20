#include "Book.h"

#include <utility>

// Einheitlicher Konstruktor
Book::Book(const int id,
           std::string isbn,
           std::string title,
           std::vector<std::string> authors,
           std::string publisher,
           std::string location,
           std::string edition,
           std::string genre,
           double price,
           MediaType mediaType,
           int maxLoanPeriodDays,
           std::time_t createdAt,
           std::string createdBy,
           std::string lastModifiedBy,
           bool isAvailable)
    : inventoryID(id),
      isbn(std::move(isbn)),
      title(std::move(title)),
      authors(std::move(authors)),
      publisher(std::move(publisher)),
      edition(std::move(edition)),
      location(std::move(location)),
      genre(std::move(genre)),
      price(price),
      maxLoanPeriodDays(maxLoanPeriodDays),
      mediaType(mediaType),
      createdAt(createdAt),
      createdBy(std::move(createdBy)),
      lastModifiedBy(std::move(lastModifiedBy)),
      isAvailable(isAvailable) {}