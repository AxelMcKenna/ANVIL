# ANVIL — Project Log

A limit order book matching engine and market-making backtester in C++20.
Portfolio project targeting quant developer / quant trading internships.

CMake project name: `anvil`. GitHub repo name: TBD (likely `anvil`).

---

## Status

| Session | Date       | Where we ended                                                          |
| ------- | ---------- | ----------------------------------------------------------------------- |
| 1       | 2026-05-15 | Toolchain bootstrapped. CMake + vcpkg + GoogleTest hello-world is green. |

## Author profile

- **C++ experience:** none — learning as the project progresses.
- **Dev environment:** macOS (Apple Silicon).
- **Benchmark target:** Linux x86 box required before any perf numbers land in README. Apple Silicon results do not transfer and will not be credible in interview review.

## Build order (Phase 1 — Core matching engine)

- [x] Toolchain bootstrap: CMake + vcpkg + GoogleTest hello-world. `ctest --preset dev` green (2026-05-15).
- [x] `include/anvil/side.hpp` — `Side` enum (Bid / Ask). Header-only, `enum class : std::uint8_t`, plus `constexpr opposite()`. Verified via `static_assert` (2026-05-15).
- [x] `include/anvil/order.hpp` — `Order` value type. 32-byte aggregate (id/price/timestamp/quantity/side). Integer ticks for price (signed int64). Defaulted `<=>` and `==`. Type aliases not strong typedefs (decision flagged for revisit). Verified via `static_assert` (2026-05-15).
- [x] `include/anvil/price_level.hpp` + `src/price_level.cpp` — FIFO queue with cancel-by-iterator, cached `total_quantity_`, `std::list<Order>` underlying. Asserts (not throws) for preconditions (2026-05-15).
- [x] `tests/price_level_test.cpp` — 10 cases covering FIFO order, cancel-at-head/middle/tail, total-quantity accounting, partial fill, iteration order. 12/12 passing (2026-05-15).
- [x] `include/anvil/trade.hpp` — `Trade` POD (aggressive_id, resting_id, price, ts, qty). 40 bytes. Defaulted `<=>` and `==`. Verified via `static_assert` (2026-05-15).
- [x] `include/anvil/order_book.hpp` — class declaration. Two-map layout (`std::greater` for bids, default for asks). `unordered_map<OrderId, OrderLocation>` for O(1) cancel. Header parses clean (2026-05-15).
- [x] `src/order_book.cpp` — `add_limit_order`, `cancel`, queries, `match_buy` / `match_sell` (near-duplicates), `rest_order`. Compiles clean under full warning regime (2026-05-15).
- [x] `tests/order_book.cpp` — 22 cases: empty book, resting, crossing at equal/improved/non-cross prices, partial + full fills, multi-level walks, FIFO at a level, four cancel paths, sell-side symmetry. 33/33 passing across all components (2026-05-15). _Filename should arguably be `order_book_test.cpp` to match siblings; deferred for now._
- [x] `benchmarks/order_book_bench.cpp` + `benchmarks/CMakeLists.txt` + `bench` CMake preset. Google Benchmark via vcpkg. Three benchmarks (`BM_AddOrder`, `BM_CancelOrder`, `BM_MatchOneLevel`). Match benchmark rewritten to pre-load resting orders outside the timed loop — removed ~2000 ns of pause/resume bias. macOS Intel numbers (loaded machine, **NOT README material**): add ~396, cancel ~142, match ~154 ns/op. Plausible Phase 3 hypothesis: add cost is allocator-bound (two-to-three heap allocs per call), not algorithm-bound (2026-05-15).
- [ ] Google Benchmark baseline (add / cancel / match at 1M random orders) — Linux only.

## Phase 2 — LOBSTER replay

- [x] `include/anvil/lobster/message.hpp` — `Message` struct (32 bytes), `MessageType` and `Direction` enums, `to_side` helper. Kept LOBSTER's ×10000 price scale and `+1/-1` direction encoding on the wire; conversion to `anvil::Side` happens at the engine boundary (2026-05-15).
- [x] `include/anvil/lobster/parser.hpp` + `src/lobster/parser.cpp` — CSV message parser. `std::from_chars` (locale-independent), int64-ns time parsed via dot-split (no `double`), stack-allocated field views (no per-line vector alloc), bounds-checked enum casts, line-numbered errors. Type-7 halt messages skipped silently (`-1` sentinel fields are incompatible with unsigned parsing) (2026-05-15).
- [x] `tests/lobster_parser_test.cpp` — 13 cases: happy path for all 6 supported types, sell direction, fractional time padding, CR-stripping, halt-skipping, four error paths, line-number in error message. 46/46 passing (2026-05-15).
- [ ] `include/anvil/lobster/snapshot.hpp` + `src/lobster/snapshot.cpp` — top-N snapshot type + LOBSTER orderbook-file parser.
- [ ] `OrderBook::reduce_order` (Type 2 partial cancel) + `OrderBook::top_n_snapshot(n)`.
- [ ] `include/anvil/lobster/replay.hpp` + `src/lobster/replay.cpp` — driver: stream messages → engine → verify snapshots.
- [ ] `tests/lobster_parser_test.cpp` + tiny fixture CSVs.
- [ ] `tests/lobster_replay_test.cpp` — end-to-end replay against fixture.
- [ ] Run against real LOBSTER sample (AAPL one-day free download).

## Later phases (do not start)

- Phase 3: Profile-driven optimisation. Only after `perf` / flamegraphs identify hot paths. No speculative rewriting.
- Phase 4: Avellaneda–Stoikov market maker with inventory risk, realistic fill model (queue position, adverse selection, exchange latency). Sharpe / drawdown / turnover, sensitivity sweep over γ and κ.
- Phase 5: Writeup. Honest assumptions section. Post-mortem.

## Decisions log

- **2026-05-15** — `Order` is a plain aggregate, 32 bytes by design (2 per cache line). Integer ticks for price (`std::int64_t` — signed for negative-price products). Type aliases (`OrderId`, `Price`, `Quantity`, `Timestamp`) instead of strong typedefs; revisit if type-mixup bugs surface. Defaulted `operator<=>` + `operator==` so tests can compare orders directly. `static_assert(sizeof(Order) == 32)` guards against accidental bloat from future field additions.
- **2026-05-15** — Starting Phase 1 with naive `std::map<Price, std::list<Order>>`. Cache-friendly alternatives (intrusive lists, flat maps, custom allocators) come in Phase 3 only when profiling justifies them. Premature optimisation here destroys the narrative that we measured before we optimised.
- **2026-05-15** — macOS used for development only. Benchmark numbers in the README must come from a Linux x86 host. Will provision a cloud VM (~c7i.large) before Phase 1 benchmark commit.
- **2026-05-15** — Header layout: public API in `include/anvil/`, implementation in `src/`. Library target compiled separately from tests and benchmarks so the build graph is clean.
- **2026-05-15** — Build system: CMake ≥ 3.25 + Ninja + vcpkg (manifest mode at `~/vcpkg`) + `CMakePresets.json`. Workflow is `cmake --preset dev` → `cmake --build --preset dev` → `ctest --preset dev`.
- **2026-05-15** — Warnings-as-errors enabled (`-Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion -Wsign-conversion -Wold-style-cast -Wnull-dereference -Wdouble-promotion -Wformat=2 -Wcast-align`). Applied via the `anvil_warnings` INTERFACE target so every component compiles under the same regime.
- **2026-05-15** — One residual linker warning on macOS (duplicate `libgtest.a` debug+release). Cosmetic vcpkg quirk; left untouched.

## Open questions

- None yet.

## Next session pickup

- Next file: `src/order_book.cpp`. Implementations for `add_limit_order`, `cancel`, the two match helpers, `rest_order`, and the queries.
- Add `src/order_book.cpp` to the `anvil` library target in root `CMakeLists.txt`.
- Style in force: K&R braces, indented namespace body, include-what-you-use, `<anvil/...>` only (never bare quoted names).

## Next file

`src/order_book.cpp`. The matching engine itself. Justification: this is the file the interviewer cares most about. The `add_limit_order → match → rest` flow is the canonical "limit order book" interview question; getting the cross check, partial-fill loop, level cleanup, and id-index maintenance all correct is the bar. Two near-identical match helpers (`match_buy`, `match_sell`) deliberately duplicate ~25 lines each — clearer than a template helper for now. Explicitly *not* doing yet: market orders, IOC / FOK / etc., self-trade prevention, modify-in-place. Limit orders only, naive `std::map` storage.
