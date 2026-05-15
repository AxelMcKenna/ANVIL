#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

#include <anvil/order.hpp>
#include <anvil/price_level.hpp>
#include <anvil/side.hpp>
#include <anvil/trade.hpp>

namespace anvil {

    class OrderBook {
    public:

        // Insert a limit order. Any crossing quantity matches immediately and
        // produces trades; the remainder (if any) rests on the book.
        std::vector<Trade> add_limit_order(Order order);

        // Cancel an order by id. Returns true if the id was on the book
        bool cancel(OrderId id);

        // --- Queries ---
        [[nodiscard]] std::optional<Price> best_bid() const noexcept;
        [[nodiscard]] std::optional<Price> best_ask() const noexcept;
        [[nodiscard]] Quantity bid_quantity_at(Price price) const noexcept;
        [[nodiscard]] Quantity ask_quantity_at(Price price) const noexcept;
        [[nodiscard]] bool empty() const noexcept;

    private:
        struct OrderLocation {
            Side side;
            Price price;
            PriceLevel::iterator iter;
        };

        // Bids: descending (best == highest price == first map entry).
        std::map<Price, PriceLevel, std::greater<>> bids_;
        // Asks: ascending (best == lowest price == first map entry).
        std::map<Price, PriceLevel> asks_;
        // Id -> location index for O(1) cancel
        std::unordered_map<OrderId, OrderLocation> by_id_;

        // Matching helpers. Consume the opposite until incoming.quantity
        // hits zero or the next level no longer crosses
        std::vector<Trade> match_buy(Order& incoming);
        std::vector<Trade> match_sell(Order& incoming);

        // Append remaining quantity to the appropriate side. Updates by_id_.
        void rest_order(Order order);
    };
}   // namespace anvil
