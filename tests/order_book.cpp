#include <gtest/gtest.h>

  #include <anvil/order_book.hpp>

  namespace {

  constexpr anvil::Order make_order(anvil::OrderId id,
                                    anvil::Side side,
                                    anvil::Price price,
                                    anvil::Quantity qty,
                                    anvil::Timestamp ts = 0) {
      return {id, price, ts, qty, side};
  }

  }  // namespace

  // === A. Empty book =========================================================

  TEST(OrderBook, EmptyByDefault) {
      anvil::OrderBook book;
      EXPECT_TRUE(book.empty());
      EXPECT_FALSE(book.best_bid().has_value());
      EXPECT_FALSE(book.best_ask().has_value());
      EXPECT_EQ(book.bid_quantity_at(100), 0U);
      EXPECT_EQ(book.ask_quantity_at(100), 0U);
  }

  // === B. Resting (non-crossing) =============================================

  TEST(OrderBook, RestingBidUpdatesBestBid) {
      anvil::OrderBook book;
      auto trades = book.add_limit_order(make_order(1, anvil::Side::Bid, 100, 10));

      EXPECT_TRUE(trades.empty());
      EXPECT_FALSE(book.empty());
      ASSERT_TRUE(book.best_bid().has_value());
      EXPECT_EQ(*book.best_bid(), 100);
      EXPECT_FALSE(book.best_ask().has_value());
      EXPECT_EQ(book.bid_quantity_at(100), 10U);
  }

  TEST(OrderBook, RestingAskUpdatesBestAsk) {
      anvil::OrderBook book;
      auto trades = book.add_limit_order(make_order(1, anvil::Side::Ask, 101, 10));

      EXPECT_TRUE(trades.empty());
      ASSERT_TRUE(book.best_ask().has_value());
      EXPECT_EQ(*book.best_ask(), 101);
      EXPECT_FALSE(book.best_bid().has_value());
  }

  TEST(OrderBook, NonCrossingBidAndAskCoexist) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Bid, 99, 10));
      book.add_limit_order(make_order(2, anvil::Side::Ask, 101, 10));

      EXPECT_EQ(*book.best_bid(), 99);
      EXPECT_EQ(*book.best_ask(), 101);
  }

  TEST(OrderBook, MultipleBidsBestBidIsHighest) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Bid, 100, 5));
      book.add_limit_order(make_order(2, anvil::Side::Bid, 102, 5));
      book.add_limit_order(make_order(3, anvil::Side::Bid, 101, 5));

      EXPECT_EQ(*book.best_bid(), 102);
  }

  TEST(OrderBook, MultipleAsksBestAskIsLowest) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Ask, 105, 5));
      book.add_limit_order(make_order(2, anvil::Side::Ask, 103, 5));
      book.add_limit_order(make_order(3, anvil::Side::Ask, 104, 5));

      EXPECT_EQ(*book.best_ask(), 103);
  }

  TEST(OrderBook, BidQuantityAggregatesAtLevel) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Bid, 100, 5));
      book.add_limit_order(make_order(2, anvil::Side::Bid, 100, 7));

      EXPECT_EQ(book.bid_quantity_at(100), 12U);
  }

  // === C. Matching ===========================================================

  TEST(OrderBook, AggressiveBuyCrossesAtRestingAskPrice) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Ask, 100, 10));

      auto trades = book.add_limit_order(make_order(2, anvil::Side::Bid, 105, 10));

      ASSERT_EQ(trades.size(), 1U);
      EXPECT_EQ(trades[0].aggressive_id, 2U);
      EXPECT_EQ(trades[0].resting_id, 1U);
      EXPECT_EQ(trades[0].price, 100);  // resting price, not aggressor's 105
      EXPECT_EQ(trades[0].quantity, 10U);
      EXPECT_TRUE(book.empty());
  }

  TEST(OrderBook, AggressiveBuyAtEqualPriceCrosses) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Ask, 100, 10));

      auto trades = book.add_limit_order(make_order(2, anvil::Side::Bid, 100, 10));

      ASSERT_EQ(trades.size(), 1U);
      EXPECT_EQ(trades[0].price, 100);
      EXPECT_TRUE(book.empty());
  }

  TEST(OrderBook, JustOneTickBelowDoesNotCross) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Ask, 100, 10));

      auto trades = book.add_limit_order(make_order(2, anvil::Side::Bid, 99, 10));

      EXPECT_TRUE(trades.empty());
      EXPECT_EQ(*book.best_bid(), 99);
      EXPECT_EQ(*book.best_ask(), 100);
  }

  TEST(OrderBook, PartialFillLeavesRestingOrderReduced) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Ask, 100, 10));

      auto trades = book.add_limit_order(make_order(2, anvil::Side::Bid, 100, 3));

      ASSERT_EQ(trades.size(), 1U);
      EXPECT_EQ(trades[0].quantity, 3U);
      EXPECT_EQ(book.ask_quantity_at(100), 7U);
  }

  TEST(OrderBook, FullFillRemovesRestingOrderAndLevel) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Ask, 100, 10));

      auto trades = book.add_limit_order(make_order(2, anvil::Side::Bid, 100, 10));

      ASSERT_EQ(trades.size(), 1U);
      EXPECT_EQ(book.ask_quantity_at(100), 0U);
      EXPECT_FALSE(book.best_ask().has_value());
  }

  TEST(OrderBook, AggressiveBuyEatsAcrossMultipleLevels) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Ask, 100, 5));
      book.add_limit_order(make_order(2, anvil::Side::Ask, 101, 5));
      book.add_limit_order(make_order(3, anvil::Side::Ask, 102, 5));

      auto trades = book.add_limit_order(make_order(4, anvil::Side::Bid, 101, 8));

      ASSERT_EQ(trades.size(), 2U);
      EXPECT_EQ(trades[0].price, 100);
      EXPECT_EQ(trades[0].quantity, 5U);
      EXPECT_EQ(trades[1].price, 101);
      EXPECT_EQ(trades[1].quantity, 3U);
      EXPECT_EQ(*book.best_ask(), 101);
      EXPECT_EQ(book.ask_quantity_at(101), 2U);
  }

  TEST(OrderBook, AggressiveOrderWithLeftoverRests) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Ask, 100, 6));

      auto trades = book.add_limit_order(make_order(2, anvil::Side::Bid, 100, 10));

      ASSERT_EQ(trades.size(), 1U);
      EXPECT_EQ(trades[0].quantity, 6U);
      EXPECT_FALSE(book.best_ask().has_value());
      ASSERT_TRUE(book.best_bid().has_value());
      EXPECT_EQ(*book.best_bid(), 100);
      EXPECT_EQ(book.bid_quantity_at(100), 4U);
  }

  TEST(OrderBook, FifoPriorityWithinPriceLevel) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Ask, 100, 5));
      book.add_limit_order(make_order(2, anvil::Side::Ask, 100, 5));

      auto trades = book.add_limit_order(make_order(3, anvil::Side::Bid, 100, 5));

      ASSERT_EQ(trades.size(), 1U);
      EXPECT_EQ(trades[0].resting_id, 1U);  // first-in fills first
      EXPECT_EQ(book.ask_quantity_at(100), 5U);
  }

  // === D. Cancel =============================================================

  TEST(OrderBook, CancelRestingOrderRemovesIt) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Bid, 100, 10));

      EXPECT_TRUE(book.cancel(1));
      EXPECT_FALSE(book.best_bid().has_value());
  }

  TEST(OrderBook, CancelLastOrderAtLevelRemovesLevel) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Bid, 100, 5));
      book.add_limit_order(make_order(2, anvil::Side::Bid, 100, 5));

      EXPECT_TRUE(book.cancel(1));
      EXPECT_EQ(book.bid_quantity_at(100), 5U);
      EXPECT_TRUE(book.cancel(2));
      EXPECT_EQ(book.bid_quantity_at(100), 0U);
      EXPECT_FALSE(book.best_bid().has_value());
  }

  TEST(OrderBook, CancelReturnsFalseForUnknownId) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Bid, 100, 5));

      EXPECT_FALSE(book.cancel(999));
  }

  TEST(OrderBook, CancelReturnsFalseAfterFullFill) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Ask, 100, 5));
      book.add_limit_order(make_order(2, anvil::Side::Bid, 100, 5));  // fully fills #1

      EXPECT_FALSE(book.cancel(1));  // id should already be gone via fill
  }

  // === E. Symmetry (sell aggressor) =========================================

  TEST(OrderBook, AggressiveSellCrossesAtRestingBidPrice) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Bid, 100, 10));

      auto trades = book.add_limit_order(make_order(2, anvil::Side::Ask, 95, 10));

      ASSERT_EQ(trades.size(), 1U);
      EXPECT_EQ(trades[0].price, 100);  // seller gets price improvement up to resting bid
      EXPECT_TRUE(book.empty());
  }

  TEST(OrderBook, SellEatsBidsBestFirst) {
      anvil::OrderBook book;
      book.add_limit_order(make_order(1, anvil::Side::Bid, 98, 5));
      book.add_limit_order(make_order(2, anvil::Side::Bid, 100, 5));
      book.add_limit_order(make_order(3, anvil::Side::Bid, 99, 5));

      auto trades = book.add_limit_order(make_order(4, anvil::Side::Ask, 98, 12));

      ASSERT_EQ(trades.size(), 3U);
      EXPECT_EQ(trades[0].price, 100);  // highest bid eaten first
      EXPECT_EQ(trades[1].price, 99);
      EXPECT_EQ(trades[2].price, 98);
      EXPECT_EQ(trades[0].quantity, 5U);
      EXPECT_EQ(trades[1].quantity, 5U);
      EXPECT_EQ(trades[2].quantity, 2U);
      EXPECT_EQ(book.bid_quantity_at(98), 3U);
  }
