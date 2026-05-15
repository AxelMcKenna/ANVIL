#include <anvil/order_book.hpp>

#include <algorithm>
#include <cassert>

namespace anvil {

    std::vector<Trade> OrderBook::add_limit_order(Order order) {
        assert(by_id_.find(order.id) == by_id_.end());  // ids must be unique
        std::vector<Trade> trades;

        if (order.side == Side::Bid) {
            trades = match_buy(order);
        } else {
            trades = match_sell(order);
        }

        if (order.quantity > 0) {
            rest_order(order);
        }
        return trades;
    }

    bool OrderBook::cancel(OrderId id) {
        auto it = by_id_.find(id);

        if (it == by_id_.end()) {
            return false;
        }

        const OrderLocation loc = it->second;

        if (loc.side == Side::Bid) {
            auto level_it = bids_.find(loc.price);
            assert(level_it != bids_.end());
            level_it->second.erase(loc.iter);
            if (level_it->second.empty()) {
                bids_.erase(level_it);
            }
        } else {
            auto level_it = asks_.find(loc.price);
            assert(level_it != asks_.end());
            level_it->second.erase(loc.iter);

            if (level_it->second.empty()) {
                asks_.erase(level_it);
            }
        }

        by_id_.erase(id);
        return true;
    }

    std::optional<Price> OrderBook::best_bid() const noexcept {
        if (bids_.empty()) {
            return std::nullopt;
        }
        return bids_.begin()->first;
    }

    std::optional<Price> OrderBook::best_ask() const noexcept {
        if (asks_.empty()) {
            return std::nullopt;
        }
        return asks_.begin()->first;
    }

    Quantity OrderBook::bid_quantity_at(Price price) const noexcept {
        auto it = bids_.find(price);
        return it == bids_.end() ? 0U : it->second.total_quantity();
    }

    Quantity OrderBook::ask_quantity_at(Price price) const noexcept {
        auto it = asks_.find(price);
        return it == asks_.end() ? 0U : it->second.total_quantity();
    }

    bool OrderBook::empty() const noexcept {
        return bids_.empty() && asks_.empty();
    }

    std::vector<Trade> OrderBook::match_buy(Order& incoming) {
        std::vector<Trade> trades;

        while (incoming.quantity > 0 && !asks_.empty()) {
            auto best_it = asks_.begin();
            const Price level_price = best_it->first;

            if (incoming.price < level_price) {
                break;  // no longer crosses
            }

            PriceLevel& level = best_it->second;

            while (incoming.quantity > 0 && !level.empty()) {
                const Order& resting = level.front();
                const Quantity fill_qty = std::min(incoming.quantity, resting.quantity);

                trades.push_back({
                    .aggressive_id = incoming.id,
                    .resting_id = resting.id,
                    .price = level_price,
                    .timestamp = incoming.timestamp,
                    .quantity = fill_qty,
                });

                if (fill_qty == resting.quantity) {
                    by_id_.erase(resting.id);
                    level.pop_front();
                } else {
                    level.reduce_front(fill_qty);
                }
                incoming.quantity -= fill_qty;
            }
            if (level.empty()) {
                asks_.erase(best_it);
            }
        }
        return trades;
    }

    std::vector<Trade> OrderBook::match_sell(Order& incoming) {
        std::vector<Trade> trades;

        while (incoming.quantity > 0 && !bids_.empty())
        {
            auto best_it = bids_.begin();
            const Price level_price = best_it->first;

            if (incoming.price > level_price) {
                break;  // no longer crosses
            }

            PriceLevel& level = best_it->second;
            while (incoming.quantity > 0 && !level.empty())
            {
                const Order& resting = level.front();
                const Quantity fill_qty = std::min(incoming.quantity, resting.quantity);
                trades.push_back({
                    .aggressive_id = incoming.id,
                    .resting_id = resting.id,
                    .price = level_price,
                    .timestamp = incoming.timestamp,
                    .quantity = fill_qty
                });

                if (fill_qty == resting.quantity) {
                    by_id_.erase(resting.id);
                    level.pop_front();
                } else {
                    level.reduce_front(fill_qty);
                }
                incoming.quantity -= fill_qty;
            }
            if (level.empty()) {
                bids_.erase(best_it);
            }
        }
        return trades;
    }

    void OrderBook::rest_order(Order order) {
        if (order.side == Side::Bid) {
            PriceLevel& level = bids_[order.price];
            const auto iter = level.enqueue(order);
            by_id_[order.id] = {Side::Bid, order.price, iter};
        } else {
            PriceLevel& level = asks_[order.price];
            const auto iter = level.enqueue(order);
            by_id_[order.id] = {Side::Ask, order.price, iter};
        }
    }
}      // namespace anvil
