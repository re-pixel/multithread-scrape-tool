#pragma once
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_set.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_unordered_map.h>
#include <atomic>
#include <string>

class Structs {
public:
    static void update_price_stats(double price);

    static std::atomic<int> active_jobs;
    static std::atomic<double> total_price;
    static std::atomic<int> five_star_count;
    static std::atomic<int> number_of_available_books;
    static std::atomic<double> min_price;
    static std::atomic<double> max_price;

    static tbb::concurrent_unordered_set<std::string> visited_urls;
    static tbb::concurrent_queue<std::string> job_queue;
    static tbb::concurrent_vector<tbb::concurrent_unordered_map<std::string, std::string>> book_data;
    static tbb::concurrent_unordered_map<std::string, int> category_results;
};
