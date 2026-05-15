#include <gtest/gtest.h>

#include <anvil/lobster/parser.hpp>

#include <sstream>
#include <stdexcept>
#include <string>

namespace {

anvil::lobster::Message parse_one(const std::string& line) {
    std::istringstream in(line);
    auto messages = anvil::lobster::parse_messages(in);
    if (messages.size() != 1) {
        throw std::runtime_error("expected exactly one message");
    }
    return messages[0];
}

}  // namespace

// === Happy path ============================================================

TEST(LobsterParser, EmptyStream) {
    std::istringstream in("");
    auto messages = anvil::lobster::parse_messages(in);
    EXPECT_TRUE(messages.empty());
}

TEST(LobsterParser, BlankLinesSkipped) {
    std::istringstream in("\n\n34200.000000000,1,42,100,1234500,1\n\n");
    auto messages = anvil::lobster::parse_messages(in);
    EXPECT_EQ(messages.size(), 1U);
}

TEST(LobsterParser, NewLimitOrderParses) {
    auto m = parse_one("34200.123456789,1,42,100,1234500,1");
    EXPECT_EQ(m.time_ns, 34200'123'456'789LL);
    EXPECT_EQ(m.type, anvil::lobster::MessageType::NewLimitOrder);
    EXPECT_EQ(m.order_id, 42U);
    EXPECT_EQ(m.size, 100U);
    EXPECT_EQ(m.price, 1234500);
    EXPECT_EQ(m.direction, anvil::lobster::Direction::Buy);
}

TEST(LobsterParser, SellDirectionParses) {
    auto m = parse_one("34200.000000000,1,42,100,1234500,-1");
    EXPECT_EQ(m.direction, anvil::lobster::Direction::Sell);
}

TEST(LobsterParser, AllSupportedMessageTypes) {
    std::istringstream in(
        "34200.0,1,1,10,100,1\n"   // NewLimitOrder
        "34200.1,2,1,5,100,1\n"    // PartialCancel
        "34200.2,3,1,5,100,1\n"    // TotalCancel
        "34200.3,4,1,3,100,1\n"    // VisibleExecution
        "34200.4,5,1,3,100,1\n"    // HiddenExecution
        "34200.5,6,1,3,100,1\n");  // CrossTrade
    auto messages = anvil::lobster::parse_messages(in);
    ASSERT_EQ(messages.size(), 6U);
    EXPECT_EQ(messages[0].type, anvil::lobster::MessageType::NewLimitOrder);
    EXPECT_EQ(messages[1].type, anvil::lobster::MessageType::PartialCancel);
    EXPECT_EQ(messages[2].type, anvil::lobster::MessageType::TotalCancel);
    EXPECT_EQ(messages[3].type, anvil::lobster::MessageType::VisibleExecution);
    EXPECT_EQ(messages[4].type, anvil::lobster::MessageType::HiddenExecution);
    EXPECT_EQ(messages[5].type, anvil::lobster::MessageType::CrossTrade);
}

TEST(LobsterParser, TradingHaltSkipped) {
    std::istringstream in(
        "34200.0,1,1,10,100,1\n"
        "34200.1,7,-1,-1,-1,-1\n"  // halt — should be silently skipped
        "34200.2,3,1,10,100,1\n");
    auto messages = anvil::lobster::parse_messages(in);
    ASSERT_EQ(messages.size(), 2U);
    EXPECT_EQ(messages[0].type, anvil::lobster::MessageType::NewLimitOrder);
    EXPECT_EQ(messages[1].type, anvil::lobster::MessageType::TotalCancel);
}

TEST(LobsterParser, FractionalTimePaddedToNanoseconds) {
    auto m = parse_one("34200.5,1,42,100,1234500,1");
    EXPECT_EQ(m.time_ns, 34200'500'000'000LL);
}

TEST(LobsterParser, TrailingCarriageReturnStripped) {
    std::istringstream in("34200.0,1,42,100,1234500,1\r\n");
    auto messages = anvil::lobster::parse_messages(in);
    ASSERT_EQ(messages.size(), 1U);
    EXPECT_EQ(messages[0].order_id, 42U);
}

// === Error paths ===========================================================

TEST(LobsterParser, ThrowsOnTooFewFields) {
    std::istringstream in("34200.0,1,42,100,1234500");
    EXPECT_THROW({ [[maybe_unused]] auto _ = anvil::lobster::parse_messages(in); }, std::runtime_error);
}

TEST(LobsterParser, ThrowsOnTooManyFields) {
    std::istringstream in("34200.0,1,42,100,1234500,1,extra");
    EXPECT_THROW({ [[maybe_unused]] auto _ = anvil::lobster::parse_messages(in); }, std::runtime_error);
}

TEST(LobsterParser, ThrowsOnTypeOutOfRange) {
    std::istringstream in("34200.0,99,42,100,1234500,1");
    EXPECT_THROW({ [[maybe_unused]] auto _ = anvil::lobster::parse_messages(in); }, std::runtime_error);
}

TEST(LobsterParser, ThrowsOnInvalidDirection) {
    std::istringstream in("34200.0,1,42,100,1234500,2");
    EXPECT_THROW({ [[maybe_unused]] auto _ = anvil::lobster::parse_messages(in); }, std::runtime_error);
}

TEST(LobsterParser, ErrorMessageIncludesLineNumber) {
    std::istringstream in(
        "34200.0,1,42,100,1234500,1\n"
        "34200.1,99,42,100,1234500,1\n");
    try {
        [[maybe_unused]] auto _ = anvil::lobster::parse_messages(in);
        FAIL() << "expected throw";
    } catch (const std::runtime_error& e) {
        const std::string what = e.what();
        EXPECT_NE(what.find("line 2"), std::string::npos) << "got: " << what;
    }
}
