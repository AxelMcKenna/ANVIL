#include <benchmark/benchmark.h>

#include <anvil/order_book.hpp>

#include <cstdint>
#include <random>
#include <vector>

namespace {

constexpr std::size_t kPoolSize = 1'000'000;

// Random mix of bids and asks across a +/-100 tick band around 1000.
// Some orders will cross immediately; that is representative of real flow.
std::vector<anvil::Order> make_random_orders(std::size_t n, std::uint64_t seed = 42) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<anvil::Price>    price_dist(900, 1100);
    std::uniform_int_distribution<anvil::Quantity> qty_dist(1, 100);
    std::uniform_int_distribution<int>             side_dist(0, 1);

    std::vector<anvil::Order> orders;
    orders.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        orders.push_back({
            .id        = static_cast<anvil::OrderId>(i + 1),
            .price     = price_dist(rng),
            .timestamp = static_cast<anvil::Timestamp>(i),
            .quantity  = qty_dist(rng),
            .side      = side_dist(rng) ? anvil::Side::Bid : anvil::Side::Ask,
        });
    }
    return orders;
}

// All bids strictly below 1000, all asks strictly above. Every order rests,
// so each id is later cancellable.
std::vector<anvil::Order> make_non_crossing_orders(std::size_t n, std::uint64_t seed = 42) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<anvil::Quantity> qty_dist(1, 100);
    std::uniform_int_distribution<int>             side_dist(0, 1);
    std::uniform_int_distribution<anvil::Price>    bid_dist(900, 999);
    std::uniform_int_distribution<anvil::Price>    ask_dist(1001, 1100);

    std::vector<anvil::Order> orders;
    orders.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        const bool is_bid = side_dist(rng) != 0;
        orders.push_back({
            .id        = static_cast<anvil::OrderId>(i + 1),
            .price     = is_bid ? bid_dist(rng) : ask_dist(rng),
            .timestamp = static_cast<anvil::Timestamp>(i),
            .quantity  = qty_dist(rng),
            .side      = is_bid ? anvil::Side::Bid : anvil::Side::Ask,
        });
    }
    return orders;
}

}  // namespace

// =============================================================================
// Add: empty book grows by random crossing/non-crossing mix.
// =============================================================================
static void BM_AddOrder(benchmark::State& state) {
    const auto orders = make_random_orders(kPoolSize);
    anvil::OrderBook book;
    std::size_t idx = 0;
    for (auto _ : state) {
        if (idx >= orders.size()) {
            state.SkipWithError("ran out of pre-generated orders");
            break;
        }
        auto trades = book.add_limit_order(orders[idx++]);
        benchmark::DoNotOptimize(trades);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));
}
BENCHMARK(BM_AddOrder)->Iterations(kPoolSize);

// =============================================================================
// Cancel: pre-warm book with N non-crossing orders, then time cancels.
// =============================================================================
static void BM_CancelOrder(benchmark::State& state) {
    const auto orders = make_non_crossing_orders(kPoolSize);
    anvil::OrderBook book;
    for (const auto& o : orders) {
        book.add_limit_order(o);
    }
    std::size_t idx = 0;
    for (auto _ : state) {
        if (idx >= orders.size()) {
            state.SkipWithError("ran out of orders to cancel");
            break;
        }
        bool ok = book.cancel(orders[idx++].id);  // non-const so DoNotOptimize takes the lvalue overload
        benchmark::DoNotOptimize(ok);
    }
    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));
}
BENCHMARK(BM_CancelOrder)->Iterations(kPoolSize);

// =============================================================================
// Match: pre-fill the book with N resting asks at price 1000, then time N
// aggressive bids that each consume exactly one resting order. The FIFO queue
// at the top-of-book drains by one per iteration; book is empty at the end.
//
// No PauseTiming/ResumeTiming — the resting orders are loaded once outside the
// timed loop, so each iteration measures one match in isolation.
// =============================================================================
constexpr std::size_t kMatchIters = 100'000;

static void BM_MatchOneLevel(benchmark::State& state) {
    anvil::OrderBook book;
    std::vector<anvil::Order> aggressors;
    aggressors.reserve(kMatchIters);

    for (std::size_t i = 0; i < kMatchIters; ++i) {
        const auto resting_id = static_cast<anvil::OrderId>(i + 1);
        book.add_limit_order({
            .id        = resting_id,
            .price     = 1000,
            .timestamp = 0,
            .quantity  = 10,
            .side      = anvil::Side::Ask,
        });
        aggressors.push_back({
            .id        = static_cast<anvil::OrderId>(kMatchIters + i + 1),
            .price     = 1000,
            .timestamp = 1,
            .quantity  = 10,
            .side      = anvil::Side::Bid,
        });
    }

    std::size_t idx = 0;
    for (auto _ : state) {
        if (idx >= aggressors.size()) {
            state.SkipWithError("ran out of aggressors");
            break;
        }
        auto trades = book.add_limit_order(aggressors[idx++]);
        benchmark::DoNotOptimize(trades);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));
}
BENCHMARK(BM_MatchOneLevel)->Iterations(kMatchIters);
