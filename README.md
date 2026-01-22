# 🌸 Flower Exchange - Order Matching Engine (2023)

A C++ implementation of an order matching engine for a flower trading exchange system. This project simulates a real-time trading platform that processes buy and sell orders for various flower instruments.

## 📋 Overview

The Flower Exchange is an order matching system that:

- Processes trading orders from a CSV input file
- Matches buy and sell orders based on price-time priority
- Maintains separate order books for different flower instruments
- Generates execution reports for all processed orders
- Validates orders against business rules before processing

### Supported Instruments

| Instrument | Description |
|------------|-------------|
| Rose | Rose flowers |
| Lavender | Lavender flowers |
| Lotus | Lotus flowers |
| Tulip | Tulip flowers |
| Orchid | Orchid flowers |

### Order Types

| Side | Description |
|------|-------------|
| 1 | Buy Order |
| 2 | Sell Order |

### Execution Statuses

| Status | Description |
|--------|-------------|
| New | Order accepted and added to order book |
| Fill | Order fully executed |
| PFill | Order partially filled |
| Reject | Order rejected due to validation failure |

## Architecture

### Data Structures

- **Priority Queues**: Used for maintaining buy-side (max-heap by price) and sell-side (min-heap by price) order books
- **Hash Map**: Maps each instrument type to its corresponding order book pair
- **Custom Comparators**: `BuySideComparator` and `SellSideComparator` for price-time priority ordering

### Order Matching Algorithm

1. **Buy Orders**: Match against sell orders with price ≤ buy price (lowest sell price first)
2. **Sell Orders**: Match against buy orders with price ≥ sell price (highest buy price first)
3. **Price-Time Priority**: Orders at the same price level are matched by arrival time (earlier orders first)

## Project Structure

```
Flower-Exchange/
├── main.cpp              # Main order matching engine
├── TraderApplication.cpp # Alternative trader application
├── orders.csv            # Input file with trading orders
├── execution_rep.csv     # Output file with execution reports
├── report.csv            # Additional report output
├── main                  # Compiled executable
├── lseg.cbp              # Code::Blocks project file
├── lseg.depend           # Dependency file
├── lseg.layout           # Layout configuration
├── bin/                  # Binary output directory
│   └── Debug/
└── obj/                  # Object files directory
    └── Debug/
```

## Input Format

The input file `orders.csv` should have the following format:

```csv
Cl. Ord. ID,Instrument,Side,Price,Quantity
aa13,Rose,2,1,100
aa14,Rose,1,2,100
```

### Input Validation Rules

| Field | Validation Rule |
|-------|-----------------|
| Client Order ID | Non-empty, max 7 characters |
| Instrument | Must be: Rose, Lavender, Lotus, Tulip, or Orchid |
| Side | Must be 1 (Buy) or 2 (Sell) |
| Price | Must be greater than 0 |
| Quantity | Must be between 10-1000, divisible by 10 |

## Output Format

The output file `execution_rep.csv` contains execution reports:

```csv
Client Order ID,Order ID,Instrument,Side,Exec Status,Quantity,Price,Reason
aa13,ord1,Rose,2,New,100,1.00
aa14,ord2,Rose,1,Fill,100,1.00
aa13,ord1,Rose,2,Fill,100,1.00
```

## Execution

### Prerequisites

- C++ compiler with C++11 support (g++, clang++, or MSVC)
- Standard C++ libraries

### Compilation

#### Using g++ (Linux/macOS)

```bash
g++ -std=c++11 -o main main.cpp
```

#### Using Code::Blocks

1. Open `lseg.cbp` in Code::Blocks
2. Build the project (F9 or Build → Build)

### Running the Application

1. **Prepare the input file**: Ensure `orders.csv` is in the same directory as the executable with your trading orders.

2. **Run the executable**:

   ```bash
   ./main
   ```

3. **Check the output**: The execution reports will be generated in `execution_rep.csv`.

### Example Run

```bash
$ ./main
Execution Time: 2 Milliseconds
```

## Sample Execution Flow

Given input orders:
```
aa13,Rose,2,1,100   # Sell 100 Rose at price 1
aa14,Rose,1,2,100   # Buy 100 Rose at price 2
```

Execution flow:
1. Order `aa13` (Sell Rose @ 1): No matching buy orders → Status: **New**
2. Order `aa14` (Buy Rose @ 2): Matches with `aa13` (Sell @ 1 ≤ Buy @ 2)
   - `aa14`: Status **Fill** (fully matched)
   - `aa13`: Status **Fill** (fully matched)
   - Trade executed at seller's price (1.00)

## ⚡ Performance

The application measures and displays execution time in milliseconds using `chrono::high_resolution_clock`.

## Technical Details

- **Language**: C++11
- **Build System**: Code::Blocks / Make
- **Core Libraries**: 
  - `<queue>` - Priority queue for order books
  - `<unordered_map>` - Hash map for instrument mapping
  - `<chrono>` - High-resolution timing
  - `<fstream>` - File I/O operations

## Notes

- Orders are matched at the **passive order's price** (the order already in the book)
- Each order receives a unique sequential Order ID (`ord1`, `ord2`, etc.)
- Invalid orders are rejected with a descriptive reason in the execution report
- The system maintains separate order books for each instrument type
