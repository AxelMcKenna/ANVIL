#pragma once

#include <compare>
#include <cstdint>

#include <anvil/side.hpp>

namespace anvil {

    using OrderId = std::uint64_t;
    using Price = std::int64_t;
    using Quantity = std::uint32_t;
    using Timestamp = std::int64_t;

    struct Order {
        OrderId id;
        Price price;
        Timestamp timestamp;
        Quantity quantity;
        Side side;

        [[nodiscard]] friend constexpr auto operator<=>(const Order&, const Order&) = default;

        [[nodiscard]] friend constexpr bool operator==(const Order&, const Order&) = default;
    };

    static_assert(sizeof(Order) == 32, "Order should be 32 bytes (2 per cache line)");


}   // namespace anvil