#include <anvil/lobster/parser.hpp>

#include <charconv>
#include <istream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace anvil::lobster {

    namespace {

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

    // "34200.123456789" -> 34'200'123'456'789 ns. Avoids double precision loss.
    std::int64_t parse_time_ns(std::string_view sv) {
        const auto dot = sv.find('.');
        if (dot == std::string_view::npos) {
            return parse_int<std::int64_t>(sv) * 1'000'000'000LL;
        }
        const std::int64_t whole = parse_int<std::int64_t>(sv.substr(0, dot));
        std::string_view frac = sv.substr(dot + 1);
        if (frac.size() > 9) {
            frac = frac.substr(0, 9);
        }
        std::int64_t nanos = frac.empty() ? 0 : parse_int<std::int64_t>(frac);
        for (std::size_t i = frac.size(); i < 9; ++i) {
            nanos *= 10;
        }
        return whole * 1'000'000'000LL + nanos;
    }

    // Split a line into exactly 6 fields. Views point into `line`; caller must
    // ensure the source string outlives the views.
    void split_six(std::string_view line, std::string_view (&out)[6]) {
        std::size_t field = 0;
        std::size_t start = 0;
        for (std::size_t i = 0; i <= line.size(); ++i) {
            if (i == line.size() || line[i] == ',') {
                if (field >= 6) {
                    throw std::runtime_error("too many fields");
                }
                out[field++] = line.substr(start, i - start);
                start = i + 1;
            }
        }
        if (field != 6) {
            throw std::runtime_error("expected 6 fields, got " + std::to_string(field));
        }
    }

    }  // namespace

    std::vector<Message> parse_messages(std::istream& in) {
        std::vector<Message> out;
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
                std::string_view fields[6];
                split_six(line, fields);

                const auto type_raw = parse_int<int>(fields[1]);
                if (type_raw < 1 || type_raw > 7) {
                    throw std::runtime_error("type out of range: " + std::to_string(type_raw));
                }
                if (type_raw == 7) {
                    // Halt messages carry -1 placeholders in order_id/size/price/direction.
                    // We don't model halts; skip rather than choke on the sentinel values.
                    continue;
                }
                const auto dir_raw = parse_int<int>(fields[5]);
                if (dir_raw != 1 && dir_raw != -1) {
                    throw std::runtime_error("direction must be +/-1, got " + std::to_string(dir_raw));
                }

                out.push_back({
                    .time_ns   = parse_time_ns(fields[0]),
                    .order_id  = parse_int<std::uint64_t>(fields[2]),
                    .price     = parse_int<std::int64_t>(fields[4]),
                    .size      = parse_int<std::uint32_t>(fields[3]),
                    .type      = static_cast<MessageType>(type_raw),
                    .direction = static_cast<Direction>(dir_raw),
                });
            } catch (const std::exception& e) {
                throw std::runtime_error("LOBSTER line " + std::to_string(line_no) + ": " + e.what());
            }
        }
        return out;
    }

}  // namespace anvil::lobster
