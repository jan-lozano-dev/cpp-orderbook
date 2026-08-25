#pragma once

#include "order.hpp"

#include <list>

class PriceLevel{
public:
    using Iterator = std::list<Order>::iterator;
    Iterator add_order(const Order &order);
    void erase_order(Iterator order);
    const Order& front_order() const;
    bool empty() const;
    Quantity total_quantity() const;
    void apply_fill_front(Quantity executed_quantity);

private:
    std::list<Order> orders_;    
    Quantity total_quantity_{0}; 
};
