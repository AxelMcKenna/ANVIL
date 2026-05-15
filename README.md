# ANVIL

[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C.svg?style=flat&logo=cplusplus&logoColor=white)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.25%2B-064F8C.svg?style=flat&logo=cmake&logoColor=white)](https://cmake.org/)
[![vcpkg](https://img.shields.io/badge/vcpkg-manifest-0078D4.svg?style=flat)](https://vcpkg.io/)
[![Tests](https://img.shields.io/badge/tests-33%2F33%20passing-success.svg)](#build--test)

A limit order book matching engine and market-making backtester in C++20. Built file-by-file as a portfolio project; design decisions recorded in `PROJECT_LOG.md`.

**Status:** Phase 1 (core matching engine) functionally complete. No benchmarks landed yet. Phases 2–5 unstarted.

## What's in here

A price-time-priority matching engine for limit orders:

- `add_limit_order` — matches any crossing quantity immediately, rests the remainder.
- `cancel(order_id)` — O(1) via an order-id index.
- Best bid / best ask / per-level depth queries.
- 33 unit tests covering: cross at equal price, price improvement, partial / full fills, multi-level walks, FIFO within a price level, four cancel paths, and full sell-side symmetry.

The engine is **deliberately naive**: `std::map<Price, std::list<Order>>` per side, plus a `std::unordered_map<OrderId, OrderLocation>` for cancel. Profiling-driven optimisation (intrusive lists, flat maps, custom allocators) is Phase 3 work and explicitly out of scope here.

## Layout

```
include/anvil/    public API headers
src/              implementation, compiled into libanvil.a
tests/            GoogleTest, one TU per component
CMakeLists.txt    build graph
vcpkg.json        dependency manifest
CMakePresets.json configure / build / test presets
PROJECT_LOG.md    session log + decisions
```

## Build & test

Requires CMake ≥ 3.25, Ninja, a C++20 compiler, and vcpkg (default location `~/vcpkg`).

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

Tests are auto-discovered via `gtest_discover_tests`.

## Tech stack

- C++20 (designated initializers, defaulted `operator<=>`, `std::optional`)
- CMake 3.25+ / Ninja
- vcpkg in manifest mode (GoogleTest; Google Benchmark to be added in Phase 1 baseline)
- GoogleTest for correctness
- Google Benchmark for perf (pending Linux run)

## Roadmap

| Phase | Scope                                                              | Status                          |
| ----- | ------------------------------------------------------------------ | ------------------------------- |
| 1     | Core matching engine + correctness tests                            | functional ✓, benchmarks pending |
| 2     | LOBSTER replay harness with tick-by-tick state validation           | not started                     |
| 3     | Profile-driven optimisation (`perf`, flamegraphs)                   | not started                     |
| 4     | Avellaneda–Stoikov market maker with realistic fill model           | not started                     |
| 5     | Writeup with measured numbers and honest assumptions                | not started                     |

## Known limitations (Phase 1)

- **Limit orders only.** No market, IOC, FOK, post-only, or other time-in-force types.
- **No `modify_order`.** Callers must `cancel(id)` then `add_limit_order(new)`. Price changes lose queue priority either way; modify-in-place is only meaningful for quantity-down, which we defer.
- **No self-trade prevention.** The engine has no concept of participant or account; duplicate ids trip a debug `assert`.
- **Single-threaded.** No locking. Real engines either single-thread the matcher per symbol or shard; we're not yet at the point where this matters.
- **Asserts, not exceptions,** for precondition violations (e.g. pop-front on empty). Hot path allocates only for `std::vector<Trade>` returns and `std::list` / `std::map` node allocation.
- **Performance has not been measured.** There are no "fast" or "low-latency" claims in this repo until a Google Benchmark number backs them up.

## Design decisions worth flagging

- **Price as integer ticks (`std::int64_t`).** Floating point is forbidden: the order book is a price-keyed map, cross checks like `bid >= ask` break under IEEE 754 rounding, and some markets (e.g. oil futures, April 2020) trade at legitimately negative prices — hence the signed type.
- **`Order` is a 32-byte aggregate.** Fits two per cache line. A `static_assert(sizeof(Order) == 32)` guards against accidental field bloat.
- **Cancel by iterator inside `PriceLevel`.** The id-to-location map lives in `OrderBook`; `PriceLevel` is dumb storage. Keeps the two structures composable.
- **Two `match_*` helpers instead of one templated `match`.** The bid map (`std::greater`) and ask map (default) differ only by comparator. Templating dedupes ~25 lines but adds template instantiation in the header. The duplication is deliberate; consolidation is a Phase 3 item.
- **`std::map` + `std::list`, deliberately.** Both are textbook-naive and known cache-unfriendly. Replacement is gated on Phase 3 profiling.

## Project log

See `PROJECT_LOG.md` for the file-by-file build trail and decisions log — including what was considered and rejected at each step.
