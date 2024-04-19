#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <set>
#include <shared_mutex>

#include "io.hpp"

struct Order {
  CommandType type;
  uint32_t order_id;
  uint32_t price;
  uint32_t count;
  uint32_t execution_id;
  size_t time_added;
};

auto comparePrice = [](const Order& a, const Order& b) {
  if (a.type != b.type)
    throw std::invalid_argument("Differing type in same orderbook");
  if (a.type == CommandType::input_cancel)
    throw std::invalid_argument("Cancel type in orderbook");
  if (a.price == b.price)
    return a.time_added < b.time_added;
  return a.type == CommandType::input_sell ? a.price < b.price : a.price > b.price;
};

class OrderBook {
  private:
    std::set<Order, decltype(comparePrice)> orderbook;
    mutable std::shared_mutex mut;
  public:
    using iterator = typename std::set<Order, decltype(comparePrice)>::iterator;
    using const_iterator = typename std::set<Order, decltype(comparePrice)>::const_iterator;

    void insert(const Order& key) {
      std::lock_guard l{mut};
      orderbook.insert(key);
    }

    void erase(const Order& key) {
      std::lock_guard l{mut};
      orderbook.erase(key);
    }

    iterator find(const Order& key) {
      std::lock_guard l{mut};
      return orderbook.find(key);
    }

    size_t size() const {
      std::shared_lock l{mut};
      return orderbook.size();
    }

    size_t count(const Order& key) const {
      std::shared_lock l{mut};
      return orderbook.count(key);
    }

    const_iterator begin() {
      std::shared_lock l{mut};
      return orderbook.begin();
    }

    const_iterator end() {
      std::shared_lock l{mut};
      return orderbook.end();
    }
    OrderBook() = default;
    OrderBook& operator=(const OrderBook&) = delete;
    OrderBook(const OrderBook&) = delete;
};

#endif
