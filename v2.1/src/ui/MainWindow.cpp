#include "MainWindow.h"

#include <QHeaderView>
#include <QTableWidget>
#include <algorithm>
// For demo
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <random>

// For BUY/SELL dropdown
#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QRegularExpression>
#include <optional>

namespace {
    QString formatPrice(Price ticks) {
        return QString("%1.%2")
        .arg(static_cast<qulonglong>(ticks/100))
        .arg(static_cast<qulonglong>(ticks % 100), 2, 10, QLatin1Char('0'));
    }

    std::optional<Price> parsePrice(QString text) {
        text = text.trimed(); // remove surrounding whitespace

        static const QRegularExpression pattern(R"(^[0-9]+\.[0-9]{2}$)");
        if(!pattern.match(text).hasMatch()) {
            return std::nullopt;
        }

        bool valid = false;
        const Price ticks = text.remove('.').toULongLong(&valid);

        if(!valid || ticks == 0) {
            return std::nullopt;
        }

        return ticks;
    }
}

MainWindow::MainWindow() : bookTable_(new QTableWidget(0, 4, this)) {
    setWindowTitle("Order Book Terminal");
    resize(1100, 700);

    bookTable_->setHorizontalHeaderLabels({
        "Bid quantity", "Bid price", "Ask price", "Ask quantity"
    });

    bookTable_->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
    );

    bookTable_->verticalHeader()->hide();
    bookTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bookTable_->setSelectionMode(QAbstractItemView::NoSelection);

    //setCentralWidget(bookTable_);
    
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    auto *demoButton = new QPushButton("Load demo", central);

    // Create input
    sideInput_ = new QComboBox(central);
    sideInput_->addItems({"BUY", "SELL"});
    priceInput_ = new QLineEdit("100.01", central);

    quantityInput_ = new QSpinBox(central);
    quantityInput_->setRange(1, 1'000'000);
    quantityInput_->setValue(100);
    
    auto* orderForm = new QFormLayout;
    orderForm->addRow("Side", sideInput_);
    orderForm->addRow("Limit Price", priceInput_);
    orderForm->addRow("Quantity", quantityInput_);

    layout->addWidget(demoButton, 0, Qt::AlignLeft);
    layout->addLayout(orderForm);
    layout->addWidget(bookTable_, 1);

    setCentralWidget(central);
    connect(demoButton, &QPushButton::clicked, this, [this, demoButton] {
        loadDemo();
        demoButton->setEnabled(false);
    });
    
    refreshBook();
}

void MainWindow::loadDemo() {
    std::mt19937 random(42);
    std::uniform_int_distribution<int> quantity(1,20);

    for(Price level = 0; level < 10; ++level) {
        for(int order = 0; order < 3; ++order) {
            book_.submit_limit_order(
                nextOrderId_++, Side::buy, 9'999 - level,
                static_cast<Quantity>(quantity(random)) * 10,
                TimeInForce::gtc
            );

            book_.submit_limit_order(
                nextOrderId_++, Side::sell, 10'001 + level,
                static_cast<Quantity>(quantity(random)) * 10,
                TimeInForce::gtc
            );
        }
    }

    refreshBook();
}

void MainWindow::refreshBook() {
    const auto snapshot = book_.snapshot();
    bookTable_->clearContents();
    bookTable_->setRowCount(static_cast<int>
        (std::max(snapshot.bids.size(), snapshot.asks.size())));

        const auto setCell = [this](int row, int column, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            bookTable_->setItem(row, column, item);
        };

        int row = 0;
        for(const auto& level : snapshot.bids) {
            setCell(row, 0, QString::number(static_cast<qulonglong>(level.quantity)));
            setCell(row, 1, formatPrice(level.price));
            ++row;
        }

        row = 0;
        for(const auto& level : snapshot.asks) {
            setCell(row, 2, QString::number(static_cast<qulonglong>(level.quantity)));
            setCell(row, 3, formatPrice(level.price));
            ++row;
        }
}