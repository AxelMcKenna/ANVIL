#pragma once

#include <cstdint>

#include <anvil/side.hpp>

namespace anvil::lobster {

    // LOBSTER message types. Values match the "Type" column in LOBSTER CSV.

    enum class MessageType : std::uint8_t {
        NewLimitOrder = 1,  // submission of a new visible limit order
        PartialCancel = 2,  // reduce quantity of a resting order (preserves queue priority)
        TotalCancel = 3,    // delete a resting order entirely
        VisibleExecution = 4, // a visible limit order was executed
        HiddenExecution = 5, // hidden order executed (not on visible book)
        CrossTrade = 6, // auction trade
        TradingHalt = 7, // exchange halt
    };

    // LOBSTER's "Direction" column. Encodes the side of the *limit order*
    // referenced by the message - for Type 4 (execution), that's the resting
    // order being hit. Kept distinct from anvil::Side because the wire encoding
    // is different (+1/-1 vs Bid/Ask) and we convert only at the engine boundary
    enum class Direction : std::int8_t {
        Buy = 1,
        Sell = -1
    };

    [[nodiscard]] constexpr Side to_side(Direction d) noexcept {
        return d == Direction::Buy ? Side::Bid : Side::Ask;
    }

    // One row from a LOBSTER message file.
    //
    // Field semantics depend on `type`:
    //   - Type 1/2/3: order_id, size, price, direction describe the order itself.
    //   - Type 4/5:   order_id is the executed (resting) order; size is the
    //                 executed quantity; price is the execution price;
    //                 direction is the resting order's side.
    //   - Type 7:     most fields are -1 / not meaningful.
    //
    // time_ns: nanoseconds since exchange-day midnight. LOBSTER provides decimal
    // seconds in the CSV; the parser will multiply by 1e9.
    //
    // price: LOBSTER's native ×10000 fixed-point scale (1234500 == $123.45).
    // No rescaling at parse time — the engine uses LOBSTER's tick scale directly.
    struct Message {
        std::int64_t time_ns;
        std::uint64_t order_id;
        std::int64_t price;
        std::uint32_t size;
        MessageType type;
        Direction direction;
    };

    static_assert(sizeof(Message) == 32, "Message is 32 bytes by design (8+8+8+4+1+1+2 pad)");

}   // namespace anvil::lobster