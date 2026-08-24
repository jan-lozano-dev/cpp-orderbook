# C++ Order Book from Scratch

A correctness-first C++20 limit order book and matching-engine learning project.
The implementation is being built incrementally so that every data-structure and
matching decision can be explained and tested.

## V1 scope

- Single instrument, single threaded, and in memory
- Integer price and quantity types
- Price-time priority
- Limit and market orders
- GTC, IOC, and FOK time-in-force policies
- Partial fills, cancellation, and amendment
- Trade generation and invariant-focused tests

V1 currently contains the core order, trade, price-level, price-ladder, and
order-location data structures. Matching behavior is under active development.

V2 will revisit the design with execution-report events, replay, profiling,
cache-friendly containers, and production-oriented memory management.
