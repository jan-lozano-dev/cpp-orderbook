#pragma once

#include "orderbook.hpp"

#include <QMainWindow>

// Forward declarations to let compiler know classes exist, allowing
// usage of pointers without full headers here.
class QTableWidget;
class QComboBox; // BUY or SELL
class QLineEdit; // Price, such as 100.01
class QSpinBox; // Quantity

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

    // Let MainWindow access this inputs later
    QComboBox* sideInput_;
    QLineEdit* priceInput_;
    QSpinBox* quantityInput_;

    bool submitOrder();
};

/*
MainWindow owns the engine. Qt will own and clean up the table through its widget parent system. 
The forward declaration avoids including the full table header here.
*/