#pragma once

#include <cstddef>
#include <iosfwd>
#include <vector>

#include <anvil/snapshot.hpp>

namespace anvil::lobster {

    // Parse a LOBSTER orderbook-file CSV. Each row has 4*n columns interleaved:
    //   ask1_price, ask1_size, bid1_price, bid1_size, ask2_price, ask2_size, ...
    // Sentinel values (price = +/-9'999'999'999, size = 0) mark empty levels
    // and are stripped from the resulting Snapshot.
    //
    // Throws std::runtime_error on malformed input with a line-numbered message.
    [[nodiscard]] std::vector<Snapshot> parse_snapshots(std::istream& in, std::size_t n);

}  // namespace anvil::lobster
