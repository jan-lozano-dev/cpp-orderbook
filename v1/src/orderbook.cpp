#include "orderbook.hpp"
#include <stdexcept>
 

std::vector<Trade> OrderBook::submit_limit_order(OrderId order_id, Side side, 
    Price price, Quantity quantity, TimeInForce time_in_force)
{
    if(order_locations_.contains(order_id)) throw std::invalid_argument("Order ID already exists.");
    if(price == 0) throw std::invalid_argument("Price cannot be zero.");
    if(quantity == 0) throw std::invalid_argument("Quantity cannot be zero.");

    // set incoming_remaining = quantity
    
    // Examine best price on opposite side

    // while best price crosses the limit:
        // match against oldest order at that level
        //Emit a trade
        // reduce incoming_remaining by executed quantity
        // remove filled resting orders
        // remove an empty price level
    // if incoming_remaining > 0 and TIF is GTC
        // rest the remainder
    // if incoming_remaining > 0 and TIF is IOC
        //discard the remainder
    // FOK
        // pre-check liquidity before performing matches

        // add that remainder to the same side as the incoming order
    // return all generated trades

    return {};
}


std::optional<Price> OrderBook::best_bid() const
{
    if(bids_.empty()) return std::nullopt;
    return bids_.begin()->first;
}
std::optional<Price> OrderBook::best_ask() const
{
    if(asks_.empty()) return std::nullopt;
    return asks_.begin()->first;
}

void OrderBook::rest_order(OrderId order_id, Side side, Price price, Quantity quantity)
{
    if(side == Side::buy)
    {
        auto& level = bids_[price];
        auto position = level.add_order(Order{order_id, quantity});

        order_locations_.emplace(order_id, OrderLocation{side, price, position});
        // emplace method constructs the OrderLocation in place, avoiding unnecessary copies and improving performance.
    }
    else
    {
        auto& level = asks_[price];
        auto position = level.add_order(Order{order_id, quantity});

        order_locations_.emplace(order_id, OrderLocation{side, price, position});
    }

}