#include "Structs.h"
#include "Worker.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <chrono>
#include <tbb/task_group.h>

int main() {
    auto time_start = std::chrono::high_resolution_clock::now();
    std::string start_url = "https://books.toscrape.com/index.html";
    Structs::job_queue.push(start_url);

    for (int i = 2; i <= 50; ++i) {
        Structs::job_queue.push("https://books.toscrape.com/catalogue/page-" + std::to_string(i) + ".html");
    }

    const int num_workers = std::thread::hardware_concurrency() * 16;
    std::vector<std::thread> workers;

    tbb::task_group tasks;

    std::cout << "Starting crawl with " << num_workers << " threads." << std::endl;

    for (int i = 0; i < num_workers; ++i) {
        workers.emplace_back(Worker());
		//tasks.run([]() { Worker{}(); });
    }

	//tasks.wait();

    for (auto& t : workers) t.join();

    auto time_end = std::chrono::high_resolution_clock::now();

    std::ofstream file("book_data.txt");

    file << "Total books found: " << Structs::book_data.size() << std::endl;
    file << "Average price of all books: " << Structs::total_price / Structs::book_data.size() << std::endl;
    file << "Minimum book price: " << Structs::min_price.load() << std::endl;
    file << "Maximum book price: " << Structs::max_price.load() << std::endl;
    file << "Books with 5-star rating: " << Structs::five_star_count << std::endl;
    file << "Total stock: " << Structs::number_of_available_books.load()
        << ", average availability: " << Structs::number_of_available_books.load() / Structs::book_data.size() << std::endl;
    file << "Crawling completed. Visited " << Structs::visited_urls.size() << " unique URLs." << std::endl;
    file << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count() << " miliseconds." << std::endl;

    for (auto& [category, results] : Structs::category_results)
        file << "Category: " << category << ", Results: " << results << std::endl;

    file.close();

    return 0;
}
