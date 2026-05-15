#pragma once

#include <compare>
#include <vector>

#include <anvil/order.hpp>

namespace anvil {

    // One level of the top of the book ladder. Price in engine ticks, quantity in shares
    struct PriceLevelSnapshot {
        Price price;
        Quantity quantity;

        [[nodiscard]] friend constexpr auto operator<=>(const PriceLevelSnapshot&, const PriceLevelSnapshot&) = default;
        [[nodiscard]] friend constexpr bool operator==(const PriceLevelSnapshot&, const PriceLevelSnapshot&) = default;
    };

    // Top-N book snapshot. 'bids' ordered high-to-low (best bid first).
    // 'asks' ordered low-to-high (best ask first.) Each side contains only
    // real levels - sentinel/no-entry placeholders are not stored.
    //
    // Produced by 'OrderBook::top_n_snapshot(n)' and by 'lobster::parse_snapshot'.
    // Compared field-by-field via defaulted operators during replay validation.
    struct Snapshot {
        std::vector<PriceLevelSnapshot> bids;
        std::vector<PriceLevelSnapshot> asks;

        [[nodiscard]] auto operator<=>(const Snapshot&) const = default;
        [[nodiscard]] bool operator==(const Snapshot&) const = default;
    };
}   // namespace anvil
