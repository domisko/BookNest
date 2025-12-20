#include <catch2/catch_test_macros.hpp>
#include "Book.h"

TEST_CASE("Book constructor sets fields and availability", "[book]") {
    Book b(1234, "ISBN-1", "Titel", std::vector<std::string>{"Autor"}, "Verlag",
            "GEN", "", "", 0.0, MediaType::Book);
    REQUIRE(b.inventoryID == 1234);
    REQUIRE(b.isbn == "ISBN-1");
    REQUIRE(b.title == "Titel");
    REQUIRE_FALSE(b.authors.empty());
    REQUIRE(b.authors[0] == "Autor");
    REQUIRE(b.publisher == "Verlag");
    REQUIRE(b.isAvailable == true);
}
