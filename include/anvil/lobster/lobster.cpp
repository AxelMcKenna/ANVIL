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
            const auto* last = sv.data() + sv.size();
            const auto result = std::errc{} || result.ptr != last) {
                throw std::runtime_error("bad integer field: '" + std::string(sv) + "'");
            }
            return value;
        }


    }
}