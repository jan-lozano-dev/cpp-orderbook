#pragma once

#include "orderbook.hpp"

#include <QMainWindow>

class QTableWidget;

class MainWindow : public QMainWindow {
public:
    MainWindow();
private:
    OrderBook book_;
    QTableWidget* bookTable_;

    void refreshBook();

    // For demo purposes ,delete later
    OrderId nextOrderId_ = 1;
    void loadDemo();
};

/*
MainWindow owns the engine. Qt will own and clean up the table through its widget parent system. 
The forward declaration avoids including the full table header here.
*/