#pragma once

#include <compare>

#include <anvil/order.hpp>

namespace anvil {

    struct Trade {
        OrderId aggressive_id;
        OrderId resting_id;
        Price price;
        Timestamp timestamp;
        Quantity quantity;

        [[nodiscard]] friend constexpr auto operator<=>(const Trade&, const Trade&) = default;
        [[nodiscard]] friend constexpr bool operator==(const Trade&, const Trade&) = default;
    };

    static_assert(sizeof(Trade) == 40, "Trade is 40 bytes by design (2x 8-byte id + 8 + 8 + 4 + 4 pad");
}   // namespace anvil
