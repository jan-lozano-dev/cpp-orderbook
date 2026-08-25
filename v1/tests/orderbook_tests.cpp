#include "orderbook.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error(std::string{message});
    }
}

void test_buy_matches_by_price_then_time()
{
    OrderBook book;
    book.submit_limit_order(1, Side::sell, 100, 3, TimeInForce::gtc);
    book.submit_limit_order(2, Side::sell, 100, 4, TimeInForce::gtc);
    book.submit_limit_order(3, Side::sell, 101, 5, TimeInForce::gtc);

    const auto trades =
        book.submit_limit_order(10, Side::buy, 101, 10, TimeInForce::gtc);

    require(trades.size() == 3, "buy should match three makers");
    require(trades[0].maker_order_id == 1 && trades[0].price == 100 &&
                trades[0].quantity == 3,
            "oldest order at the best ask should execute first");
    require(trades[1].maker_order_id == 2 && trades[1].price == 100 &&
                trades[1].quantity == 4,
            "second order at the best ask should execute second");
    require(trades[2].maker_order_id == 3 && trades[2].price == 101 &&
                trades[2].quantity == 3,
            "matching should continue at the next crossed ask");
    require(book.best_ask() == 101, "partially filled ask should remain");
    require(!book.best_bid().has_value(), "fully filled buy should not rest");
}

void test_sell_matches_highest_bid_first()
{
    OrderBook book;
    book.submit_limit_order(1, Side::buy, 100, 2, TimeInForce::gtc);
    book.submit_limit_order(2, Side::buy, 99, 3, TimeInForce::gtc);

    const auto trades =
        book.submit_limit_order(10, Side::sell, 99, 4, TimeInForce::gtc);

    require(trades.size() == 2, "sell should match two bid levels");
    require(trades[0].maker_order_id == 1 && trades[0].price == 100 &&
                trades[0].quantity == 2,
            "sell should execute against the highest bid first");
    require(trades[1].maker_order_id == 2 && trades[1].price == 99 &&
                trades[1].quantity == 2,
            "sell should continue at the next crossed bid");
    require(book.best_bid() == 99, "partially filled bid should remain");
}

void test_ioc_discards_unfilled_quantity()
{
    OrderBook book;
    book.submit_limit_order(1, Side::sell, 100, 2, TimeInForce::gtc);

    const auto trades =
        book.submit_limit_order(10, Side::buy, 100, 5, TimeInForce::ioc);

    require(trades.size() == 1 && trades[0].quantity == 2,
            "IOC should execute available crossing quantity");
    require(!book.best_ask().has_value(), "filled ask should be removed");
    require(!book.best_bid().has_value(), "IOC remainder should not rest");
}

void test_fok_is_atomic()
{
    OrderBook book;
    book.submit_limit_order(1, Side::sell, 100, 2, TimeInForce::gtc);
    book.submit_limit_order(2, Side::sell, 101, 2, TimeInForce::gtc);

    const auto rejected =
        book.submit_limit_order(10, Side::buy, 100, 3, TimeInForce::fok);

    require(rejected.empty(), "unfillable FOK should produce no trades");
    require(book.best_ask() == 100, "unfillable FOK should not mutate the book");

    const auto accepted =
        book.submit_limit_order(11, Side::buy, 101, 4, TimeInForce::fok);

    require(accepted.size() == 2,
            "fillable FOK should consume all required liquidity");
    require(accepted[0].quantity == 2 && accepted[1].quantity == 2,
            "fillable FOK should fill its complete quantity");
    require(!book.best_ask().has_value(),
            "fillable FOK should remove consumed levels");
}

void test_cancel_removes_order_and_empty_level()
{
    OrderBook book;
    book.submit_limit_order(1, Side::buy, 100, 5, TimeInForce::gtc);
    book.submit_limit_order(2, Side::buy, 101, 3, TimeInForce::gtc);

    require(book.cancel_order(2), "existing order should be cancelled");
    require(book.best_bid() == 100,
            "cancelling the best bid should expose the next bid");
    require(!book.cancel_order(2),
            "cancelling an unknown order should report failure");
    require(book.cancel_order(1), "remaining bid should be cancellable");
    require(!book.best_bid().has_value(),
            "cancelling the final bid should remove its price level");
}

void test_cancel_preserves_fifo_and_aggregate_quantity()
{
    OrderBook book;
    book.submit_limit_order(1, Side::sell, 100, 2, TimeInForce::gtc);
    book.submit_limit_order(2, Side::sell, 100, 3, TimeInForce::gtc);
    book.submit_limit_order(3, Side::sell, 100, 4, TimeInForce::gtc);

    require(book.cancel_order(2), "middle order should be cancellable");

    const auto rejected =
        book.submit_limit_order(10, Side::buy, 100, 7, TimeInForce::fok);
    require(rejected.empty(),
            "cancelled quantity must not count toward FOK liquidity");

    const auto trades =
        book.submit_limit_order(11, Side::buy, 100, 6, TimeInForce::fok);
    require(trades.size() == 2, "remaining orders should satisfy the FOK");
    require(trades[0].maker_order_id == 1 &&
                trades[1].maker_order_id == 3,
            "cancelling a middle order should preserve FIFO priority");
}

} // namespace

int main()
{
    try {
        test_buy_matches_by_price_then_time();
        test_sell_matches_highest_bid_first();
        test_ioc_discards_unfilled_quantity();
        test_fok_is_atomic();
        test_cancel_removes_order_and_empty_level();
        test_cancel_preserves_fifo_and_aggregate_quantity();

        std::cout << "All OrderBook tests passed.\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& exception) {
        std::cerr << "OrderBook test failed: " << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}
