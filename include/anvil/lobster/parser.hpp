#pragma once

#include <iosfwd>
#include <vector>

#include <anvil/lobster/message.hpp>

namespace anvil::lobster {

    // Parse a LOBSTER message-file CSV stream into a vector of Messages.
    // Throws std::runtime_error on malformed input with a line-numbered message.
    // No header row is expected (LOBSTER's format has none).
    //
    // For very large files, a streaming reader (Phase 2 follow-up) would avoid
    // loading the entire file into memory.
    [[nodiscard]] std::vector<Message> parse_messages(std::istream& in);

}  // namespace anvil::lobster
