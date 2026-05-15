#include <anvil/price_level.hpp>

#include <cassert>
#include <iterator>


namespace anvil {

    bool PriceLevel::empty() const noexcept {
        return orders_.empty();
    }

    std::size_t PriceLevel::size() const noexcept {
        return orders_.size();
    }

    Quantity PriceLevel::total_quantity() const noexcept {
        return total_quantity_;
    }

    const Order& PriceLevel::front() const noexcept {
        assert(!orders_.empty());
        return orders_.front();
    }

    Order& PriceLevel::front() noexcept {
        assert(!orders_.empty());
        return orders_.front();
    }

    PriceLevel::iterator PriceLevel::enqueue(Order order) {
        total_quantity_ += order.quantity;
        orders_.push_back(order);
        return std::prev(orders_.end());
    }

    void PriceLevel::erase(iterator it) noexcept {
        total_quantity_ -= it->quantity;
        orders_.erase(it);
    }

    void PriceLevel::pop_front() noexcept {
        assert(!orders_.empty());
        total_quantity_ -= orders_.front().quantity;
        orders_.pop_front();
    }

    void PriceLevel::reduce_front(Quantity qty) noexcept {
        assert(!orders_.empty());
        assert(qty < orders_.front().quantity);
        orders_.front().quantity -= qty;
        total_quantity_ -= qty;
    }

    PriceLevel::const_iterator PriceLevel::begin() const noexcept {
        return orders_.begin();
    }

    PriceLevel::const_iterator PriceLevel::end() const noexcept {
        return orders_.end();
    }
}   // namespace anvil
