#include "Structs.h"
#include <limits>

std::atomic<int> Structs::active_jobs{ 0 };
std::atomic<double> Structs::total_price{ 0 };
std::atomic<int> Structs::five_star_count{ 0 };
std::atomic<int> Structs::number_of_available_books{ 0 };
std::atomic<double> Structs::min_price{ std::numeric_limits<double>::max() };
std::atomic<double> Structs::max_price{ 0 };

tbb::concurrent_unordered_set<std::string> Structs::visited_urls;
tbb::concurrent_queue<std::string> Structs::job_queue;
tbb::concurrent_vector<tbb::concurrent_unordered_map<std::string, std::string>> Structs::book_data;
tbb::concurrent_unordered_map<std::string, int> Structs::category_results;

void Structs::update_price_stats(double price) {
    double current_min = min_price.load();
    while (current_min > price && !min_price.compare_exchange_weak(current_min, price)) {}

    double current_max = max_price.load();
    while (current_max < price && !max_price.compare_exchange_weak(current_max, price)) {}
}
