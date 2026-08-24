#pragma once // this directive ensures that the header file is included only once during compilation, preventing duplicate definitions.
#include "price_level.hpp" // includes the definition of the PriceLevel struct, which contains a price and a list of orders at that price level.
#include "trade.hpp" // includes the definition of the Trade struct, which represents a trade that occurs when a buy and sell order are matched.
#include <map>
#include <functional> // includes the functional header, which provides function objects, such as std::greater, used for ordering the bids in descending order.
#include <vector>
#include <optional> // includes the optional header, which provides the std::optional type, used for representing a value that may or may not be present (e.g., best bid or ask price).
#include <unordered_map>
class OrderBook {
public:
    std::vector<Trade> submit_limit_order(
        OrderId order_id,
        Side side,
        Price price,
        Quantity quantity,
        TimeInForce time_in_force
    );
    std::optional<Price> best_bid() const;
    std::optional<Price> best_ask() const;

private:
    struct OrderLocation{
        Side side_;
        Price price_;
        PriceLevel::Iterator order_;
    };

    std::unordered_map<OrderId, OrderLocation> order_locations_;
    std::map<Price, PriceLevel, std::greater<Price>> bids_;
    std::map<Price, PriceLevel> asks_;
    void rest_order(OrderId order_id, Side side, Price price, Quantity quantity);
};