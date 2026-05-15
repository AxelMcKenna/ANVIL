 #include <gtest/gtest.h>

  #include <anvil/price_level.hpp>

  namespace {

  constexpr anvil::Order make_order(anvil::OrderId id,
                                    anvil::Quantity qty,
                                    anvil::Price price = 100,
                                    anvil::Timestamp ts = 0,
                                    anvil::Side side = anvil::Side::Bid) {
      return {id, price, ts, qty, side};
  }

  }  // namespace

  TEST(PriceLevel, EmptyByDefault) {
      anvil::PriceLevel level;
      EXPECT_TRUE(level.empty());
      EXPECT_EQ(level.size(), 0U);
      EXPECT_EQ(level.total_quantity(), 0U);
  }

  TEST(PriceLevel, EnqueueIncreasesSizeAndTotalQuantity) {
      anvil::PriceLevel level;
      level.enqueue(make_order(1, 10));

      EXPECT_FALSE(level.empty());
      EXPECT_EQ(level.size(), 1U);
      EXPECT_EQ(level.total_quantity(), 10U);
      EXPECT_EQ(level.front().id, 1U);
  }

  TEST(PriceLevel, FifoOrderingOnPopFront) {
      anvil::PriceLevel level;
      level.enqueue(make_order(1, 10));
      level.enqueue(make_order(2, 20));
      level.enqueue(make_order(3, 30));

      EXPECT_EQ(level.front().id, 1U);
      level.pop_front();
      EXPECT_EQ(level.front().id, 2U);
      level.pop_front();
      EXPECT_EQ(level.front().id, 3U);
      level.pop_front();
      EXPECT_TRUE(level.empty());
  }

  TEST(PriceLevel, EraseMiddlePreservesFifo) {
      anvil::PriceLevel level;
      level.enqueue(make_order(1, 10));
      auto mid = level.enqueue(make_order(2, 20));
      level.enqueue(make_order(3, 30));

      level.erase(mid);

      ASSERT_EQ(level.size(), 2U);
      EXPECT_EQ(level.front().id, 1U);
      level.pop_front();
      EXPECT_EQ(level.front().id, 3U);
  }

  TEST(PriceLevel, EraseHeadAdvancesFront) {
      anvil::PriceLevel level;
      auto head = level.enqueue(make_order(1, 10));
      level.enqueue(make_order(2, 20));

      level.erase(head);

      EXPECT_EQ(level.size(), 1U);
      EXPECT_EQ(level.front().id, 2U);
  }

  TEST(PriceLevel, EraseTailKeepsHead) {
      anvil::PriceLevel level;
      level.enqueue(make_order(1, 10));
      auto tail = level.enqueue(make_order(2, 20));

      level.erase(tail);

      EXPECT_EQ(level.size(), 1U);
      EXPECT_EQ(level.front().id, 1U);
  }

  TEST(PriceLevel, EraseUpdatesTotalQuantity) {
      anvil::PriceLevel level;
      level.enqueue(make_order(1, 10));
      auto mid = level.enqueue(make_order(2, 20));
      level.enqueue(make_order(3, 30));
      ASSERT_EQ(level.total_quantity(), 60U);

      level.erase(mid);

      EXPECT_EQ(level.total_quantity(), 40U);
  }

  TEST(PriceLevel, ReduceFrontUpdatesQuantityAndTotal) {
      anvil::PriceLevel level;
      level.enqueue(make_order(1, 10));
      level.enqueue(make_order(2, 20));
      ASSERT_EQ(level.total_quantity(), 30U);

      level.reduce_front(3);

      EXPECT_EQ(level.front().quantity, 7U);
      EXPECT_EQ(level.total_quantity(), 27U);
      EXPECT_EQ(level.size(), 2U);
  }

  TEST(PriceLevel, PopFrontUpdatesTotal) {
      anvil::PriceLevel level;
      level.enqueue(make_order(1, 10));
      level.enqueue(make_order(2, 20));

      level.pop_front();

      EXPECT_EQ(level.total_quantity(), 20U);
      EXPECT_EQ(level.size(), 1U);
  }

  TEST(PriceLevel, RangeBasedForYieldsFifoOrder) {
      anvil::PriceLevel level;
      level.enqueue(make_order(1, 10));
      level.enqueue(make_order(2, 20));
      level.enqueue(make_order(3, 30));

      std::vector<anvil::OrderId> ids;
      for (const auto& order : level) {
          ids.push_back(order.id);
      }

      EXPECT_EQ(ids, (std::vector<anvil::OrderId>{1, 2, 3}));
  }