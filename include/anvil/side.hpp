#pragma once

#include <cstdint>

namespace anvil {

    enum class Side : std::uint8_t {
        Bid = 0,
        Ask = 1,
    };

    [[nodiscard]] constexpr Side opposite(Side s) noexcept {
        return s == Side::Bid ? Side::Ask : Side::Bid;
    }

}  // namespace anvil