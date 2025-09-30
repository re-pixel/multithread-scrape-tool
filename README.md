# 🕷️ MultiThreadScraper

A high-performance parallel web scraper built in C++ that extracts book information from [Books to Scrape](http://books.toscrape.com/) using multi-threading and Intel Threading Building Blocks (TBB).

## 📑 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Technical Architecture](#technical-architecture)
- [Installation](#installation)
- [Usage](#usage)
- [How It Works](#how-it-works)
- [Performance](#performance)
- [Concurrency & Thread Safety](#concurrency--thread-safety)
- [Data Extracted](#data-extracted)
- [Dependencies](#dependencies)
- [Configuration](#configuration)
- [License](#license)

## 🎯 Overview

MultiThreadScraper is a parallel web scraping tool designed to efficiently collect book data from the Books to Scrape website. The scraper traverses all available pages, extracts detailed information about books, and computes aggregate statistics such as average price, minimum/maximum prices, availability counts, and top-rated books.

Built with modern C++ and leveraging Intel's Threading Building Blocks (TBB), this project demonstrates practical applications of:
- Parallel programming with multiple worker threads
- Thread-safe data structures and concurrent containers
- Work-stealing task scheduling
- HTTP request handling
- HTML parsing with regular expressions

## ✨ Features

- **Parallel Processing** → Multiple worker threads process URLs concurrently
- **Thread-Safe Operations** → TBB concurrent containers prevent race conditions
- **Work-Stealing Architecture** → Efficient task distribution among threads
- **Comprehensive Data Extraction** → Book titles, prices, ratings, availability, descriptions
- **Category Analysis** → Results organized by book categories
- **Statistical Aggregation** → Min/max/average prices, availability counts, top-rated books
- **HTML Parsing** → Regular expression-based content extraction
- **Performance Metrics** → Built-in timing and throughput measurements
- **File Output** → Results saved to `book_data.txt`

## 🏗️ Project Architecture

### Core Components

#### 1. **Structs Module**
Global data structures and synchronization:
- `tbb::concurrent_queue<std::string>` → URL queue for processing
- `tbb::concurrent_unordered_set<std::string>` → Visited URLs tracking
- `tbb::concurrent_vector` → Thread-safe results storage
- Atomic variables used for price statistics tracking (min, max, average)

#### 2. **Worker Module**
Thread worker implementation:
- Fetches URLs from the queue
- Sends HTTP requests using CPR library
- Passes HTML content to parser
- Adds newly discovered links to queue
- Updates global statistics

#### 3. **PageParser Module**
HTML content parsing:
- Link extraction from HTML
- Book page parsing (title, price, rating, availability, description)
- Category page processing
- Regular expression-based pattern matching

## 🚀 Installation

### Prerequisites

- **C++17** or higher
- **Intel TBB** (Threading Building Blocks)
- **CPR** (C++ Requests library)
- **CMake** 3.15 or higher
- **GCC/Clang/MSVC** compiler

### Installing Dependencies

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install libtbb-dev libcurl4-openssl-dev cmake g++
```

#### macOS
```bash
brew install tbb curl cmake
```

#### Windows (vcpkg)
```bash
vcpkg install tbb cpr
```

### Building the Project

```bash
# Clone the repository
git clone https://github.com/re-pixel/multithread-scrape-tool.git
cd multithread-scrape-tool

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(nproc)

# Run the scraper
./multithread_scraper
```

## 🎮 Usage

### Basic Usage

```bash
# Run with default settings
./multithread_scraper

# The scraper will:
# 1. Start from http://books.toscrape.com/
# 2. Spawn worker threads (hardware_concurrency() * 16)
# 3. Crawl all pages and extract book data
# 4. Save results to book_data.txt
```

### Output

The scraper generates `book_data.txt` containing:
- **Overall Statistics**
  - Total books scraped
  - Average price
  - Minimum and maximum prices
  - Number of books in stock
  - Number of 5-star rated books
  
- **Category Breakdown**
  - Books organized by category
  - Statistics per category

Example output:
```
=== SCRAPING STATISTICS ===
Total Books: 1000
Average Price: £35.67
Min Price: £10.00
Max Price: £59.99
Books In Stock: 850
5-Star Books: 124

=== CATEGORY: Fiction ===
Total: 234 books
...
```

## 🔧 How It Works

### 1. Work-Stealing Model

The scraper uses a **work-stealing** architecture:
- Main thread initializes the URL queue with the starting page
- Worker threads continuously pull URLs from the concurrent queue
- Each worker processes its URL independently
- Newly discovered links are added back to the queue
- Process continues until the queue is empty

### 2. Thread Synchronization

**TBB Concurrent Containers** provide lock-free thread safety:
- `concurrent_queue` → Thread-safe URL queue (FIFO)
- `concurrent_unordered_set` → Prevents duplicate URL processing
- `concurrent_vector` → Safe parallel writes to results

### 3. URL Processing Pipeline

```
┌─────────────┐
│  URL Queue  │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   Worker    │ ──HTTP──▶ Books to Scrape
│   Thread    │ ◀─HTML───
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   Parser    │ ──Extract──▶ Links, Data
└──────┬──────┘
       │
       ├──▶ New URLs ──▶ Queue
       │
       └──▶ Book Data ──▶ Concurrent Vector
```

### 4. HTML Parsing Strategy

Uses **regular expressions** to extract:
- Book page URLs: `/catalogue/[^"]+`
- Category URLs: `/catalogue/category/[^"]+`
- Book titles: `<h1>(.+?)</h1>`
- Prices: `£(\d+\.\d+)`
- Ratings: `class="star-rating (One|Two|Three|Four|Five)"`
- Availability: `In stock \((\d+) available\)`

## ⚡ Performance

### Threading Configuration

Optimal thread count: `hardware_concurrency() * 16` - depends on the machine.

### Benchmark Results

Performance testing with different thread configurations:

| Threads | Execution Time | Throughput | Notes |
|---------|----------------|------------|-------|
| 16 | Baseline | ~50 pages/sec | Single-core equivalent |
| 64 | -40% | ~130 pages/sec | Good improvement |
| 256 | -65% | ~220 pages/sec | **Good enough** |
| 512 | -70% | ~255 pages/sec | **Optimal** |
| 1024 | -50% | ~140 pages/sec | Diminishing returns |

**TBB Task Groups vs std::thread:**
- `std::thread` vector: Better performance for this use case
- `tbb::task_group`: Higher overhead, slower for I/O-bound tasks

### Key Performance Factors

- **I/O Bound** → Network latency dominates
- **Concurrent Connections** → More threads = more parallel requests
- **Overhead** → Too many threads cause context switching overhead
- **Work-Stealing** → Efficient load balancing across workers


## 🔒 Concurrency & Thread Safety

### Race Condition Prevention

All shared data structures use TBB concurrent containers:

```cpp
// Thread-safe queue for URLs
tbb::concurrent_queue<std::string> url_queue;

// Prevents duplicate processing
tbb::concurrent_unordered_set<std::string> visited_urls;

// Safe parallel writes
tbb::concurrent_vector<BookData> results;
```

### Synchronization Strategy

- **Lock-Free Operations** → TBB containers handle internal locking
- **RAII Pattern** → Automatic resource management
- **No Manual Mutexes** → Eliminates deadlock risks
- **Atomic Updates** → Price statistics use atomic operations

### Memory Management

- **RAII Approach** → Resources automatically cleaned up
- **Smart Pointers** → No manual memory allocation
- **TBB Containers** → Handle memory safety internally
- **No Memory Leaks** → Verified with Valgrind

## 📊 Data Extracted

### Per Book
- **Title** → Full book title
- **Price** → Price in GBP (£)
- **Rating** → 1-5 stars
- **Availability** → In stock / Out of stock + quantity
- **Description** → Book description text
- **Category** → Genre/category classification

### Aggregate Statistics
- Total number of books
- Average price across all books
- Minimum price
- Maximum price
- Total books in stock
- Count of 5-star rated books
- Category-wise breakdowns

## 📦 Dependencies

### Required Libraries

```cmake
# CMakeLists.txt
find_package(TBB REQUIRED)
find_package(CURL REQUIRED)
find_package(cpr REQUIRED)
```

### External Dependencies

- **Intel TBB** → Concurrent containers and parallel algorithms
- **CPR** → Modern C++ HTTP requests library (wrapper for libcurl)
- **libcurl** → HTTP client functionality
- **C++ Standard Library** → regex, thread, chrono

### Install via Package Manager

```bash
# requirements.txt equivalent
tbb >= 2021.5
cpr >= 1.9.0
libcurl >= 7.68
```

## ⚙️ Configuration

### Adjustable Parameters

Edit in `main.cpp`:

```cpp
// Number of worker threads
const size_t num_threads = std::thread::hardware_concurrency() * 16;

// Starting URL
const std::string start_url = "http://books.toscrape.com/";

// Output file
const std::string output_file = "book_data.txt";
```


## 📄 License

This project was developed as part of the Parallel Programming course at the Faculty of Technical Sciences, University of Novi Sad.

**Student:** Relja Brdar  
**Index:** SV30/2023  
**Course:** Parallel Programming  
**Date:** September 2025

---

## 🎓 Academic Context

This project demonstrates practical applications of:
- **Parallel Programming** → Multi-threading and task parallelism
- **Concurrent Data Structures** → Thread-safe containers
- **Work-Stealing Algorithms** → Efficient task distribution
- **Performance Optimization** → Thread count tuning
- **Real-World Problem Solving** → Web scraping at scale

For detailed technical documentation, see [docs/DokumentacijaPP.pdf](docs/DokumentacijaPP.pdf).

---

## 📈 Future Enhancements

- **Distributed Scraping** → Multiple machines
- **Caching Layer** → Redis for visited URLs
- **Machine Learning** → Content classification
- **Real-time Dashboard** → Live scraping statistics
- **API Endpoint** → RESTful interface

---
