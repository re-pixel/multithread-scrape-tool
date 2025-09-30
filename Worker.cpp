#include "Worker.h"
#include "Structs.h"
#include "PageParser.h"
#include <cpr/cpr.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <regex>

std::string Worker::fetch(const std::string& url, int attempts, int timeout_ms) {
    static std::mutex cout_mutex;
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << std::this_thread::get_id() << " fetching: " << url << std::endl;
    }

    for (int i = 0; i < attempts; ++i) {
        cpr::Response response = cpr::Get(cpr::Url{ url }, cpr::Timeout{ timeout_ms });
        if (response.status_code == 200) return response.text;
		{
			std::lock_guard<std::mutex> lock(cout_mutex);
			std::cout << "Attempt " << (i + 1) << " failed for URL: " << url
				<< " with status code: " << response.status_code << std::endl;
		}
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return "";
}

void Worker::operator()() {
    std::string current_url;
    while (true) {
        if (!Structs::job_queue.try_pop(current_url)) {
            if (Structs::active_jobs.load() == 0) break;
            std::this_thread::yield();
            continue;
        }

        if (!Structs::visited_urls.insert(current_url).second) continue;

        Structs::active_jobs.fetch_add(1);


        std::string html = fetch(current_url);
        if (!html.empty()) {
            auto links = PageParser::extract_links(html, current_url);

            if (std::regex_search(html, std::regex(R"(product_main")"))) {
                PageParser::parse_book_page(html);
            }
            else if (current_url.find("category") != std::string::npos) {
                PageParser::parse_category_results(current_url, html);
            }

            for (auto& link : links) {
                if (Structs::visited_urls.find(link) == Structs::visited_urls.end())
                    Structs::job_queue.push(link);
            }
        }
        Structs::active_jobs.fetch_sub(1);
    }
}
