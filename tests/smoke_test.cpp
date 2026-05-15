#include <gtest/gtest.h>

#include "anvil/version.hpp"

TEST(Smoke, ArithmeticHolds) {
    EXPECT_EQ(1 + 1, 2);
}

TEST(Smoke, LibraryLinks) {
    EXPECT_FALSE(anvil::version().empty());
}
