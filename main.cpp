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
#include <iomanip>

using namespace std;

int id = 0;

enum class InstrumentType
{
    Rose,
    Lavender,
    Lotus,
    Tulip,
    Orchid,
    Invalid
};

string instrumentToString(InstrumentType inst)
{
    switch (inst)
    {
    case InstrumentType::Rose:
        return "Rose";
    case InstrumentType::Lavender:
        return "Lavender";
    case InstrumentType::Lotus:
        return "Lotus";
    case InstrumentType::Tulip:
        return "Tulip";
    case InstrumentType::Orchid:
        return "Orchid";
    default:
        return "Invalid";
    }
}

string statusToString(int status)
{
    switch (status)
    {
    case 0:
        return "New";
    case 1:
        return "Reject";
    case 2:
        return "Fill";
    case 3:
        return "PFill";
    default:
        return "Unknown";
    }
}

struct Order
{
    string clientOrderId;
    InstrumentType instrument;
    int side;
    double price;
    int quantity;
    int orderId;

    void generateOrderId()
    {
        orderId = ++id;
    }
};

struct ExecutionReport
{
    string clientOrderId;
    int orderId;
    InstrumentType instrument;
    double price;
    int quantity;
    int status;
    int side;
    string reason;
};

void writeExecutionReport(const ExecutionReport &report, ofstream &output)
{
    output << report.clientOrderId << ","
           << "ord" << report.orderId << ","
           << instrumentToString(report.instrument) << ","
           << report.side << ","
           << statusToString(report.status) << ","
           << report.quantity << ","
           << fixed << setprecision(2) << report.price;
    if (!report.reason.empty())
    {
        output << "," << report.reason;
    }
    output << endl;
}

struct BuySideComparator
{
    bool operator()(const Order &a, const Order &b)
    {
        if (a.price == b.price)
            return a.orderId > b.orderId;
        return a.price < b.price;
    }
};

struct SellSideComparator
{
    bool operator()(const Order &a, const Order &b)
    {
        if (a.price == b.price)
            return a.orderId > b.orderId;
        return a.price > b.price;
    }
};

class OrderBook
{
private:
    unordered_map<InstrumentType, pair<priority_queue<Order, vector<Order>, BuySideComparator>,
                                       priority_queue<Order, vector<Order>, SellSideComparator>>>
        orderBooks;

public:
    void processOrder(Order &order, ofstream &output)
    {
        order.generateOrderId();

        ExecutionReport executionReport;
        executionReport.clientOrderId = order.clientOrderId;
        executionReport.orderId = order.orderId;
        executionReport.instrument = order.instrument;
        executionReport.side = order.side;
        executionReport.price = order.price;
        executionReport.quantity = order.quantity;

        if (order.clientOrderId.empty() || order.clientOrderId.length() > 7)
        {
            executionReport.status = 1;
            executionReport.reason = "Invalid client order ID";
            writeExecutionReport(executionReport, output);
            return;
        }
        if (order.instrument == InstrumentType::Invalid)
        {
            executionReport.status = 1;
            executionReport.reason = "Invalid instrument";
            writeExecutionReport(executionReport, output);
            return;
        }
        if (order.side != 1 && order.side != 2)
        {
            executionReport.status = 1;
            executionReport.reason = "Invalid side";
            writeExecutionReport(executionReport, output);
            return;
        }
        if (order.price <= 0.0)
        {
            executionReport.status = 1;
            executionReport.reason = "Invalid price";
            writeExecutionReport(executionReport, output);
            return;
        }
        if (order.quantity < 10 || order.quantity > 1000 || order.quantity % 10 != 0)
        {
            executionReport.status = 1;
            executionReport.reason = "Invalid quantity";
            writeExecutionReport(executionReport, output);
            return;
        }

        pair<priority_queue<Order, vector<Order>, BuySideComparator>,
             priority_queue<Order, vector<Order>, SellSideComparator>> &orderBook = orderBooks[order.instrument];

        priority_queue<Order, vector<Order>, BuySideComparator> &buySide = orderBook.first;
        priority_queue<Order, vector<Order>, SellSideComparator> &sellSide = orderBook.second;

        int originalQuantity = order.quantity;
        bool matched = false;

        if (order.side == 1)
        {
            while (!sellSide.empty() && order.quantity > 0 && sellSide.top().price <= order.price)
            {
                matched = true;
                Order sellOrder = sellSide.top();
                sellSide.pop();

                int matchedQuantity = min(order.quantity, sellOrder.quantity);
                double matchPrice = sellOrder.price;

                ExecutionReport buyReport;
                buyReport.clientOrderId = order.clientOrderId;
                buyReport.orderId = order.orderId;
                buyReport.instrument = order.instrument;
                buyReport.side = order.side;
                buyReport.price = matchPrice;
                buyReport.quantity = matchedQuantity;

                ExecutionReport sellReport;
                sellReport.clientOrderId = sellOrder.clientOrderId;
                sellReport.orderId = sellOrder.orderId;
                sellReport.instrument = sellOrder.instrument;
                sellReport.side = sellOrder.side;
                sellReport.price = matchPrice;
                sellReport.quantity = matchedQuantity;

                order.quantity -= matchedQuantity;
                sellOrder.quantity -= matchedQuantity;

                buyReport.status = (order.quantity == 0) ? 2 : 3;
                sellReport.status = (sellOrder.quantity == 0) ? 2 : 3;

                writeExecutionReport(buyReport, output);
                writeExecutionReport(sellReport, output);

                if (sellOrder.quantity > 0)
                {
                    sellSide.push(sellOrder);
                }
            }

            if (order.quantity > 0)
            {
                if (!matched)
                {
                    executionReport.status = 0;
                    executionReport.quantity = order.quantity;
                    writeExecutionReport(executionReport, output);
                }
                buySide.push(order);
            }
        }
        else
        {
            while (!buySide.empty() && order.quantity > 0 && buySide.top().price >= order.price)
            {
                matched = true;
                Order buyOrder = buySide.top();
                buySide.pop();

                int matchedQuantity = min(order.quantity, buyOrder.quantity);
                double matchPrice = buyOrder.price;

                ExecutionReport sellReport;
                sellReport.clientOrderId = order.clientOrderId;
                sellReport.orderId = order.orderId;
                sellReport.instrument = order.instrument;
                sellReport.side = order.side;
                sellReport.price = matchPrice;
                sellReport.quantity = matchedQuantity;

                ExecutionReport buyReport;
                buyReport.clientOrderId = buyOrder.clientOrderId;
                buyReport.orderId = buyOrder.orderId;
                buyReport.instrument = buyOrder.instrument;
                buyReport.side = buyOrder.side;
                buyReport.price = matchPrice;
                buyReport.quantity = matchedQuantity;

                order.quantity -= matchedQuantity;
                buyOrder.quantity -= matchedQuantity;

                sellReport.status = (order.quantity == 0) ? 2 : 3;
                buyReport.status = (buyOrder.quantity == 0) ? 2 : 3;

                writeExecutionReport(buyReport, output);
                writeExecutionReport(sellReport, output);

                if (buyOrder.quantity > 0)
                {
                    buySide.push(buyOrder);
                }
            }

            if (order.quantity > 0)
            {
                if (!matched)
                {
                    executionReport.status = 0;
                    executionReport.quantity = order.quantity;
                    writeExecutionReport(executionReport, output);
                }
                sellSide.push(order);
            }
        }
    }
};

vector<string> splitString(const string &str, char d)
{
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (getline(ss, token, d))
    {
        tokens.push_back(token);
    }
    return tokens;
}

InstrumentType stringToInstrument(const string &ins)
{
    if (ins == "Rose")
        return InstrumentType::Rose;
    else if (ins == "Lavender")
        return InstrumentType::Lavender;
    else if (ins == "Lotus")
        return InstrumentType::Lotus;
    else if (ins == "Orchid")
        return InstrumentType::Orchid;
    else if (ins == "Tulip")
        return InstrumentType::Tulip;
    else
        return InstrumentType::Invalid;
}

int main()
{
    auto start_time = chrono::high_resolution_clock::now();
    ifstream inputFile("orders.csv");
    ofstream outputFile("execution_rep.csv");
    string line;

    if (!inputFile.is_open())
    {
        cerr << "Error: Could not open orders.csv" << endl;
        return 1;
    }

    outputFile << "Client Order ID,Order ID,Instrument,Side,Exec Status,Quantity,Price,Reason" << endl;

    OrderBook orderBook;
    getline(inputFile, line);

    while (getline(inputFile, line))
    {
        if (line.empty())
            continue;

        vector<string> data = splitString(line, ',');

        if (data.size() >= 5)
        {
            Order order;
            order.clientOrderId = data[0];
            order.instrument = stringToInstrument(data[1]);
            
            try
            {
                order.side = stoi(data[2]);
                order.price = stod(data[3]);
                order.quantity = stoi(data[4]);
            }
            catch (const exception &ex)
            {
                continue;
            }

            orderBook.processOrder(order, outputFile);
        }
    }

    inputFile.close();
    outputFile.close();

    auto end_time = chrono::high_resolution_clock::now();
    auto exec_time = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
    cout << "Execution Time: " << exec_time << " Milliseconds" << endl;
    return 0;
}
