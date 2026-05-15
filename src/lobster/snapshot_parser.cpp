#include <anvil/lobster/snapshot_parser.hpp>

#include <charconv>
#include <istream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace anvil::lobster {

    namespace {

    constexpr Price kAskSentinel =  9'999'999'999LL;
    constexpr Price kBidSentinel = -9'999'999'999LL;

    template <typename T>
    T parse_int(std::string_view sv) {
        T value{};
        const auto* first = sv.data();
        const auto* last  = sv.data() + sv.size();
        const auto result = std::from_chars(first, last, value);
        if (result.ec != std::errc{} || result.ptr != last) {
            throw std::runtime_error("bad integer field: '" + std::string(sv) + "'");
        }
        return value;
    }

    // Split a line into exactly `expected` fields. Views point into `line`.
    void split_n(std::string_view line, std::size_t expected,
                 std::vector<std::string_view>& out) {
        out.clear();
        out.reserve(expected);
        std::size_t start = 0;
        for (std::size_t i = 0; i <= line.size(); ++i) {
            if (i == line.size() || line[i] == ',') {
                out.push_back(line.substr(start, i - start));
                start = i + 1;
            }
        }
        if (out.size() != expected) {
            throw std::runtime_error("expected " + std::to_string(expected) +
                                     " fields, got " + std::to_string(out.size()));
        }
    }

    }  // namespace

    std::vector<Snapshot> parse_snapshots(std::istream& in, std::size_t n) {
        const std::size_t expected_cols = 4 * n;
        std::vector<Snapshot> out;
        std::vector<std::string_view> fields;
        std::string line;
        std::size_t line_no = 0;

        while (std::getline(in, line)) {
            ++line_no;
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (line.empty()) {
                continue;
            }

            try {
                split_n(line, expected_cols, fields);

                Snapshot snap;
                snap.asks.reserve(n);
                snap.bids.reserve(n);

                for (std::size_t i = 0; i < n; ++i) {
                    const auto ask_price = parse_int<Price>(fields[i * 4 + 0]);
                    const auto ask_size  = parse_int<Quantity>(fields[i * 4 + 1]);
                    const auto bid_price = parse_int<Price>(fields[i * 4 + 2]);
                    const auto bid_size  = parse_int<Quantity>(fields[i * 4 + 3]);

                    if (ask_price != kAskSentinel && ask_size != 0) {
                        snap.asks.push_back({ask_price, ask_size});
                    }
                    if (bid_price != kBidSentinel && bid_size != 0) {
                        snap.bids.push_back({bid_price, bid_size});
                    }
                }
                out.push_back(std::move(snap));
            } catch (const std::exception& e) {
                throw std::runtime_error("LOBSTER snapshot line " + std::to_string(line_no) +
                                         ": " + e.what());
            }
        }
        return out;
    }

}  // namespace anvil::lobster
