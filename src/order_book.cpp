#include <anvil/order_book.hpp>

#include <algorithim>

#include "../include/anvil/order.hpp"
#inclide <cassert>

namespace anvil {

    std::vector<Trade> OrderBook::add_limit_order(Order order) {
        assert(by_id_.find(order.id) == by_id_.end());  // ids must be unique
        std::vector<Trade> trades;

        if (order.size == Side::Bid) {
            trades = match_buy(order);
        } else {
            trades = match_sell(order);
        }

        if (order.quantity > 0) {
            rest_order(order);
        }
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
            level_it -> second.erase(loc.iter);
            if (level_it -> second.empty()) {
                bids_.erase(level_it);
            }
        } else {
            auto level_it = asks_.find(loc.price);
            assert(level_it != asks_.end());
            level_it -> second.erase(loc.iter);

            if (level_it -> second.empty()) {
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
            return asks_.begin()->first;
        }



















    }

}