#pragma once

#include <vector>
#include <shared_mutex>
#include "io.hpp"
#include "orderbook.hpp"
#include "lockedmap.hpp"

// Encompasses buy and sell book, synchronization between buy and sell (and maybe cancel orders) of same instruments should be kept within this class
struct InstrBook
{
  private:
    OrderBook sellBook, buyBook;
    LockedMap<unsigned int, Order> lastAdded;
    std::shared_mutex sellBookMtx, buyBookMtx, counterMtx, buyOrderMtx, sellOrderMtx;
    struct CounterPair {
      unsigned int sellBookCtr = 0;
      unsigned int buyBookCtr = 0;
    } counters;
    // Returns 0 if fully matched
    uint32_t appendMatches(const ClientCommand&, OrderBook&, std::vector<Order>&);
    void processCancelSell(const Order& order);
    void processCancelBuy(const Order& order);
  public:
    void processSell(const ClientCommand& input);
    void processBuy(const ClientCommand& input);
    void processCancel(const ClientCommand& input);
    InstrBook() = default;
};

inline int64_t getTimestamp() noexcept
{
  static std::atomic_int64_t clock;
	return clock.fetch_add(1, std::memory_order_acq_rel);
}