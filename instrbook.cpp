#include <atomic>
#include "instrbook.hpp"

void InstrBook::processSell(const ClientCommand& input) {
    std::unique_lock sellOrderLock{sellOrderMtx};
    CounterPair cnts;
    {
        std::lock_guard counterLock{counterMtx};
        cnts = this->counters;
    }
    std::vector<Order> matchedBuys;
    uint32_t qty;
    {
        std::shared_lock buyBookSharedLock{buyBookMtx};
        qty = appendMatches(input, buyBook, matchedBuys);
    }
    unsigned int timestamp;
    {
        std::lock_guard counterLock{counterMtx};
        if(cnts.buyBookCtr != this->counters.buyBookCtr || cnts.sellBookCtr != this->counters.sellBookCtr) {
            cnts = this->counters;
            std::shared_lock buyBookSharedLock{buyBookMtx};
            matchedBuys.clear();
            qty = appendMatches(input, buyBook, matchedBuys);
        }
        timestamp = getTimestamp();
        if (qty) {
            std::lock_guard sellBookLock{sellBookMtx};
            Order order = {input.type, input.order_id, input.price, qty, 1, timestamp};
            sellBook.insert(order);
            lastAdded[input.order_id] = order;
            this->counters.sellBookCtr++;
        }
    }
    {
        std::lock_guard buyBookLock{buyBookMtx};
        for(const Order& buy : matchedBuys) {
            Order rec{*buyBook.find(buy)};
            buyBook.erase(buy);
            if (rec.count -= buy.count) {
                ++rec.execution_id;
                buyBook.insert(rec);
            }
            Output::OrderExecuted(buy.order_id, input.order_id, buy.execution_id, buy.price, buy.count, timestamp);
        }
    }
    // output add to order book message
    if(qty)
        Output::OrderAdded(input.order_id, input.instrument, input.price, qty, true, timestamp);
    return;
}

void InstrBook::processBuy(const ClientCommand& input) {
    std::unique_lock buyOrderLock{buyOrderMtx};
    CounterPair cnts;
    {
        std::lock_guard counterLock{counterMtx};
        cnts = this->counters;
    }
    std::vector<Order> matchedSells;
    uint32_t qty;
    {
        std::shared_lock sellBookSharedLock{sellBookMtx};
        qty = appendMatches(input, sellBook, matchedSells);
    }
    unsigned int timestamp;
    {
        std::lock_guard counterLock{counterMtx};
        if(cnts.buyBookCtr != this->counters.buyBookCtr || cnts.sellBookCtr != this->counters.sellBookCtr) {
            cnts = this->counters;
            std::shared_lock sellBookSharedLock{sellBookMtx};
            matchedSells.clear();
            qty = appendMatches(input, sellBook, matchedSells);
        }
        timestamp = getTimestamp();
        if (qty) {
            std::lock_guard buyBookLock{buyBookMtx};
            Order order = {input.type, input.order_id, input.price, qty, 1, timestamp};
            buyBook.insert(order);
            lastAdded[input.order_id] = order;
            this->counters.buyBookCtr++;
        }
    }
    {
        std::lock_guard sellBookLock{sellBookMtx};
        for(const Order& sell : matchedSells) {
            Order rec{*sellBook.find(sell)};
            sellBook.erase(sell);
            if (rec.count -= sell.count) {
                ++rec.execution_id;
                sellBook.insert(rec);
            }
            Output::OrderExecuted(sell.order_id, input.order_id, sell.execution_id, sell.price, sell.count, timestamp);
        }
    }
    // output add to order book message
    if(qty) 
        Output::OrderAdded(input.order_id, input.instrument, input.price, qty, false, timestamp);
    return;
    
}

void InstrBook::processCancel(const ClientCommand& input) {
    {
        std::lock_guard counterLock{counterMtx};
        unsigned int timestamp = getTimestamp();
        if(!lastAdded.count(input.order_id)) {
            Output::OrderDeleted(input.order_id, false, timestamp);
            return;
        }
    }
    Order order = lastAdded[input.order_id];
    if(order.type == CommandType::input_buy) processCancelBuy(order);
    if(order.type == CommandType::input_sell) processCancelSell(order);
}

void InstrBook::processCancelSell(const Order& order) {
    // Treating cancel sell as a 'buy order'
    // cannot be concurrent with other buy orders
    std::unique_lock buyOrderLock{buyOrderMtx};
    std::lock_guard counterLock{counterMtx};
    unsigned int timestamp = getTimestamp();
    std::lock_guard sellBookLock{sellBookMtx};
    if(!sellBook.count(order)) {
        Output::OrderDeleted(order.order_id, false, timestamp);
        return;
    }
    sellBook.erase(order);
    Output::OrderDeleted(order.order_id, true, timestamp);
}

void InstrBook::processCancelBuy(const Order& order) {
    std::unique_lock sellOrderLock{sellOrderMtx};
    std::lock_guard counterLock{counterMtx};
    unsigned int timestamp = getTimestamp();
    std::lock_guard buyBookLock{buyBookMtx};
    if(!buyBook.count(order)) {
        Output::OrderDeleted(order.order_id, false, timestamp);
        return;
    }
    buyBook.erase(order);
    Output::OrderDeleted(order.order_id, true, timestamp);
}

uint32_t InstrBook::appendMatches(const ClientCommand& toMatch, OrderBook& matchBook, std::vector<Order>& matches) {
    uint32_t qty = toMatch.count;
    for(const Order& order : matchBook) {
        if (!qty) break;
        if (order.type == CommandType::input_buy && order.price < toMatch.price) break;
        if (order.type == CommandType::input_sell && order.price > toMatch.price) break;
        uint32_t matchedQty = std::min(qty, order.count);
        matches.push_back({order.type, order.order_id, order.price, matchedQty, order.execution_id, order.time_added});
        qty -= matchedQty;
    }
    return qty;
}
