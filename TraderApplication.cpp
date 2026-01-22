#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include <queue>
#include <chrono>
#include <algorithm>

using namespace std;

int id=0, orderid=0;

enum class InstrumentType {
    Rose,
    Lavender,
    Lotus,
    Tulip,
    Orchid
};

struct Order {
    string clientOrderId;
    InstrumentType instrument;
    int side;
    double price;
    int quantity;

    int generateOrderId(){
        orderid= ++id;
        return orderid;
    }
};

struct ExecutionReport {
    string clientOrderId;
    string orderId;
    InstrumentType instrument;
    double price;
    int quantity;
    int status;
};

struct BuySideComparator {
    bool operator()(Order &a, Order &b) {
        return a.price < b.price;
    }
};

struct SellSideComparator {
    bool operator()(Order &a,Order &b){
        return a.price > b.price;
    }
};

// OrderBook class
class OrderBook {
private:
        unordered_map<InstrumentType,  pair< priority_queue<Order,  vector<Order>, BuySideComparator>,
        priority_queue<Order,  vector<Order>, SellSideComparator>>> orderBooks;

public:
    void processOrder(Order &order,  ofstream &output) {
        ExecutionReport executionReport;
        executionReport.clientOrderId = order.clientOrderId;
        executionReport.instrument = order.instrument;

        if (order.clientOrderId.length() > 7) {
            executionReport.status = 1;
        }
        else if (order.instrument != InstrumentType::Rose && order.instrument != InstrumentType::Lavender &&
            order.instrument != InstrumentType::Lotus && order.instrument != InstrumentType::Tulip &&
            order.instrument != InstrumentType::Orchid) {
            executionReport.status = 1;
        }
        else if (order.side != 1 && order.side != 2) {
            executionReport.status = 1;
        }
        else if (order.price <= 0.0) {
            executionReport.status = 1;
        }
        else if (order.quantity < 10 || order.quantity > 1000 || order.quantity % 10 != 0) {
            executionReport.status = 1;
        }
        else {
            pair< priority_queue<Order,  vector<Order>, BuySideComparator>,
            priority_queue<Order,  vector<Order>, SellSideComparator>> &orderBook = orderBooks[order.instrument];

            priority_queue<Order,  vector<Order>, BuySideComparator> &buySide = orderBook.first;
            priority_queue<Order,  vector<Order>, SellSideComparator> &sellSide = orderBook.second;

            executionReport.orderId = orderid;
            executionReport.price = order.price;
            executionReport.quantity = order.quantity;

            if (order.side == 1) {
                while (!sellSide.empty() && order.quantity > 0 && sellSide.top().price <= order.price) {
                    Order sellOrder = sellSide.top();
                    sellSide.pop();

                    int matchedQuantity =  min(order.quantity, sellOrder.quantity);

                    executionReport.quantity = matchedQuantity;
                    executionReport.status = (order.quantity == matchedQuantity) ? 2 : 3;

                    sellOrder.quantity -= matchedQuantity;
                    if (sellOrder.quantity > 0) {
                        sellSide.push(sellOrder);
                    }

                    order.quantity -= matchedQuantity;

                    output << executionReport.clientOrderId << ","
                        << executionReport.orderId << ","
                        << static_cast<int>(executionReport.instrument) << ","
                        << executionReport.price << ","
                        << executionReport.quantity << ","
                        << executionReport.status <<  endl;
                }

                if (order.quantity > 0) {
                    buySide.push(order);
                }
            }
            else if (order.side == 2) {
                while (!buySide.empty() && order.quantity > 0 && buySide.top().price >= order.price) {
                    Order buyOrder = buySide.top();
                    buySide.pop();

                    int matchedQuantity =  min(order.quantity, buyOrder.quantity);

                    executionReport.quantity = matchedQuantity;
                    executionReport.status = (order.quantity == matchedQuantity) ? 2 : 3;

                    buyOrder.quantity -= matchedQuantity;
                    if (buyOrder.quantity > 0) {
                        buySide.push(buyOrder);
                    }

                    order.quantity -= matchedQuantity;

                    output << executionReport.clientOrderId << ","
                        << executionReport.orderId << ","
                        << static_cast<int>(executionReport.instrument) << ","
                        << executionReport.price << ","
                        << executionReport.quantity << ","
                        << executionReport.status <<  endl;
                }

                if (order.quantity > 0) {
                    sellSide.push(order);
                }
            }
        }
    }

    void printExecutionReport() {
         ofstream output("report.csv");
        output << "Client_order_ID,Order_ID,Instrument,Price,Quantity,Status" <<  endl;

        for (auto &orderBook : orderBooks) {
            priority_queue<Order,  vector<Order>, BuySideComparator>&buySide = orderBook.second.first;
             priority_queue<Order,  vector<Order>, SellSideComparator>&sellSide = orderBook.second.second;

            while (!buySide.empty()) {
                Order order = buySide.top();
                buySide.pop();

                ExecutionReport executionReport;
                executionReport.clientOrderId = order.clientOrderId;
                executionReport.orderId = order.generateOrderId();
                executionReport.instrument = order.instrument;
                executionReport.price = order.price;
                executionReport.quantity = order.quantity;
                executionReport.status = 3;

                output << executionReport.clientOrderId << ","
                    << executionReport.orderId << ","
                    << static_cast<int>(executionReport.instrument) << ","
                    << executionReport.price << ","
                    << executionReport.quantity << ","
                    << executionReport.status <<  endl;
            }

            while (!sellSide.empty()) {
                Order order = sellSide.top();
                sellSide.pop();

                ExecutionReport executionReport;
                executionReport.clientOrderId = order.clientOrderId;
                executionReport.orderId = orderid;
                executionReport.instrument = order.instrument;
                executionReport.price = order.price;
                executionReport.quantity = order.quantity;
                executionReport.status = 3;

                output << executionReport.clientOrderId << ","
                    << executionReport.orderId << ","
                    << static_cast<int>(executionReport.instrument) << ","
                    << executionReport.price << ","
                    << executionReport.quantity << ","
                    << executionReport.status <<  endl;
            }
        }

        output.close();
    }
};

 vector< string> splitString(const  string& str, char d) {
     vector< string> tokens;
     stringstream ss(str);
     string token;
    while ( getline(ss, token, d)) {
        tokens.push_back(token);
    }
    return tokens;
}

int main() {
     auto start_time = std::chrono::high_resolution_clock::now();
     ifstream inputFile("orders.csv");
     ofstream outputFile("report.csv");
     string line;

    OrderBook orderBook;

    while ( getline(inputFile, line)) {
         vector< string> data = splitString(line, ',');

        if (data.size() == 5) {
            Order order;
            order.clientOrderId = data[0];
            order.instrument = static_cast<InstrumentType>( stoi(data[1]));
            order.side = stoi(data[2]);
            order.price = stod(data[3]);
            order.quantity = stoi(data[4]);

            orderBook.processOrder(order, outputFile);
        }
    }

    inputFile.close();
    orderBook.printExecutionReport();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto exec_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    cout << "Execution Time: " << exec_time << " Milliseconds" << endl;


    return 0;
}
