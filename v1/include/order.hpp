#pragma once
#include <cstdint>

using OrderId = std::uint64_t; // OrderId is a type alias for uint64_t, representing the unique identifier for an order.
using Quantity = std::uint64_t; // Quantity is a type alias for uint64_t, representing the quantity of an order.
using Price = std::uint64_t;

enum class Side {
    buy,
    sell
};

enum class TimeInForce {
    gtc, // Good 'Til Canceled
    ioc, // Immediate or Cancel
    fok  // Fill or Kill
};

struct Order {
    OrderId order_id;
    Quantity quantity;
};

