#pragma once

#include <cstddef>
#include <list>

#include <anvil/order.hpp>

namespace anvil {

    class PriceLevel {
    public:
        using container_type = std::list<Order>;
        using iterator = container_type::iterator;
        using const_iterator = container_type::const_iterator;

        [[nodiscard]] bool empty() const noexcept;
        [[nodiscard]] std::size_t size() const noexcept;
        [[nodiscard]] Quantity total_quantity() const noexcept;

        [[nodiscard]] const Order& front() const noexcept;
        [[nodiscard]] Order& front() noexcept;

        // Append to the back of the FIFO queue. Returns the iterator the
        // OrderBook stores so it cancel in O(1) later.
        iterator enqueue(Order order);

        // Cancel an order. O(1) Caller passes the iterator returned by enqueue.
        void erase(iterator it) noexcept;

        // Remove the front (oldest) order. Precondition !empty().
        void pop_front() noexcept;

        // Reduce the front order's quantity (partial fill)
        // Precondition: !empty() && qty < front().quantity.
        void reduce_front(Quantity quantity) noexcept;

        [[nodiscard]] const_iterator begin() const noexcept;
        [[nodiscard]] const_iterator end() const noexcept;

    private:
        container_type orders_;
        Quantity total_quantity_ = 0;
    };

}   // namespace anvil
