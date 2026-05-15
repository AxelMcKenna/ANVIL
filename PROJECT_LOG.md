# ANVIL — Project Log

A limit order book matching engine and market-making backtester in C++20.
Portfolio project targeting quant developer / quant trading internships.

CMake project name: `anvil`. GitHub repo name: TBD (likely `anvil`).

---

## Status

| Session | Date       | Where we ended                                   |
| ------- | ---------- | ------------------------------------------------ |
| 1       | 2026-05-15 | Repo initialized. Toolchain not yet bootstrapped. |

## Author profile

- **C++ experience:** none — learning as the project progresses.
- **Dev environment:** macOS (Apple Silicon).
- **Benchmark target:** Linux x86 box required before any perf numbers land in README. Apple Silicon results do not transfer and will not be credible in interview review.

## Build order (Phase 1 — Core matching engine)

- [ ] Toolchain bootstrap: CMake + vcpkg + GoogleTest hello-world that compiles and runs `make test`.
- [ ] `include/tape/side.hpp` — `Side` enum (Bid / Ask).
- [ ] `include/tape/order.hpp` — `Order` value type.
- [ ] `include/tape/price_level.hpp` — FIFO queue of resting orders at a single price.
- [ ] `include/tape/order_book.hpp` + `src/order_book.cpp` — naive `std::map<Price, PriceLevel>` engine. Deliberately not optimised.
- [ ] 50+ GoogleTest correctness cases (crosses, partial fills, self-trades, FIFO invariants, cancel-at-head, price improvement).
- [ ] Google Benchmark baseline (add / cancel / match at 1M random orders) — Linux only.

## Later phases (do not start)

- Phase 2: LOBSTER replay harness — parser + tick-by-tick state validation.
- Phase 3: Profile-driven optimisation. Only after `perf` / flamegraphs identify hot paths. No speculative rewriting.
- Phase 4: Avellaneda–Stoikov market maker with inventory risk, realistic fill model (queue position, adverse selection, exchange latency). Sharpe / drawdown / turnover, sensitivity sweep over γ and κ.
- Phase 5: Writeup. Honest assumptions section. Post-mortem.

## Decisions log

- **2026-05-15** — Starting Phase 1 with naive `std::map<Price, std::list<Order>>`. Cache-friendly alternatives (intrusive lists, flat maps, custom allocators) come in Phase 3 only when profiling justifies them. Premature optimisation here destroys the narrative that we measured before we optimised.
- **2026-05-15** — macOS used for development only. Benchmark numbers in the README must come from a Linux x86 host. Will provision a cloud VM (~c7i.large) before Phase 1 benchmark commit.
- **2026-05-15** — Header layout: public API in `include/tape/`, implementation in `src/`. Library target compiled separately from tests and benchmarks so the build graph is clean.

## Open questions

- None yet.

## Next session pickup

- Confirm toolchain plan and prerequisites (Xcode Command Line Tools, Homebrew CMake, vcpkg bootstrap).
- Write `CMakeLists.txt`, `vcpkg.json`, and a trivial `src/main.cpp` plus `tests/smoke_test.cpp` that links GoogleTest and passes.
- Verify `cmake --build build && ctest --test-dir build` succeeds end-to-end on macOS.

## Next file

Toolchain bootstrap. Specifically the top-level `CMakeLists.txt`. Justification: with zero C++ experience, no source file is meaningful until you can compile and run a trivial test. Once the toolchain works on your machine, every subsequent file is a self-contained step.
