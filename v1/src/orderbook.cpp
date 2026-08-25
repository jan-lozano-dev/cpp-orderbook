#include "orderbook.hpp"
#include "trade.hpp"
#include <stdexcept>
#include <algorithm> // for std::min
 

std::vector<Trade> OrderBook::submit_limit_order(
    OrderId order_id,
    Side side,
    Price price,
    Quantity quantity,
    TimeInForce time_in_force)
{
    if(order_locations_.contains(order_id)) throw std::invalid_argument("Order ID already exists."); // contains only works in c++20
    if(price == 0) throw std::invalid_argument("Price cannot be zero.");
    if(quantity == 0) throw std::invalid_argument("Quantity cannot be zero.");
    if(side != Side::buy && side != Side::sell) throw std::invalid_argument("Invalid side.");
    if(time_in_force != TimeInForce::gtc && time_in_force != TimeInForce::ioc && time_in_force != TimeInForce::fok) throw std::invalid_argument("Invalid time in force.");

    // This lambda function checks if the incoming order can be fully filled
    // by examining the available liquidity on the opposite side of the order book.
    // It iterates through the price levels and checks if the total quantity at each
    // level is sufficient to fill the incoming order's quantity. If it finds a level that
    // can fully fill the order, it returns true; otherwise, it returns false.
    
    // A little remeneinder of how lambda work, [quantity] captures the quantity variable
    // from the surrounding scope, allowing the lambda to access it.
    // The parameters (const auto& levels, const auto& crosses) represent the price
    // levels and a function to determine if a price level crosses the incoming order's price.
    const auto can_fully_fill = [quantity](const auto& levels, const auto& crosses)
    {
        Quantity remaining = quantity;
        for(const auto& [level_price, level] : levels)
        {
            if(!crosses(level_price)) break;
            if(level.total_quantity() >= remaining) return true;
            remaining -= level.total_quantity();
        }
        return false;
    };

    const auto crosses_ask = [price](Price ask_price){ return ask_price <= price; };
    const auto crosses_bid = [price](Price bid_price){ return bid_price >= price; };

    if(time_in_force == TimeInForce::fok)
    {
        const bool fillable = side == Side::buy
            ? can_fully_fill(asks_, crosses_ask)
            : can_fully_fill(bids_, crosses_bid);

        if(!fillable) return {}; // FOK order cannot be filled, discard it
    }

    Quantity incoming_remaining = quantity;
    std::vector<Trade> trades;

    const auto& match = [&](auto& opposite_levels, const auto& crosses)
    {
        while(incoming_remaining > 0 && !opposite_levels.empty())
        {
            auto level_it = opposite_levels.begin();
            const Price execution_price = level_it->first;
            if(!crosses(execution_price)) break;

            PriceLevel& level = level_it->second;
            const Order& maker = level.front_order();
            const OrderId maker_order_id = maker.order_id;
            const Quantity maker_quantity = maker.quantity;
            const Quantity executed_quantity = std::min(incoming_remaining, maker_quantity);
            trades.push_back(Trade{
                maker_order_id,
                order_id,
                execution_price,
                executed_quantity
            });
            level.apply_fill_front(executed_quantity);
            incoming_remaining -= executed_quantity;
            if(executed_quantity == maker_quantity) order_locations_.erase(maker_order_id);
            if(level.empty()) opposite_levels.erase(execution_price);
        }
    };

    if(side == Side::buy)match(asks_, crosses_ask);
    else match(bids_, crosses_bid);
    if(incoming_remaining > 0 && time_in_force == TimeInForce::gtc)
    {
        rest_order(order_id, side, price, incoming_remaining);
    }

    return trades;

}

bool OrderBook::cancel_order(OrderId order_id)
{
    const auto location_it = order_locations_.find(order_id);
    if(location_it == order_locations_.end()) return false;

    const OrderLocation location = location_it->second;
    const auto erase_from_level = [&location](auto& levels)
    {
        const auto level_it = levels.find(location.price_);
        if(level_it == levels.end())
            throw std::logic_error("Order location refers to a missing price level.");

        PriceLevel& level = level_it->second;
        level.erase_order(location.order_);
        if(level.empty()) levels.erase(level_it);
    };

    if(location.side_ == Side::buy) erase_from_level(bids_);
    else if(location.side_ == Side::sell) erase_from_level(asks_);
    else throw std::logic_error("Order location contains an invalid side.");

    order_locations_.erase(location_it);
    return true;
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
