//#include <tbb/tbb.h>
//#include <tbb/concurrent_queue.h>
//#include <tbb/concurrent_unordered_set.h>
//#include <iostream>
//#include <cpr/cpr.h>
//#include <regex>
//#include <atomic>
//#include <thread>
//#include <chrono>
//#include <vector>
//#include <mutex>
//#include <filesystem>
//
//
//std::atomic<int> active_jobs{ 0 };
//std::atomic<double> total_price{ 0 };
//std::atomic<int> five_star_count{ 0 };
//std::atomic<int> number_of_available_books{ 0 };
//std::atomic<double> min_price{ std::numeric_limits<double>::max() };
//std::atomic<double> max_price{ 0 };
//
//tbb::concurrent_unordered_set<std::string> visited_urls;
//tbb::concurrent_queue<std::string> job_queue;
//tbb::concurrent_vector<tbb::concurrent_unordered_map<std::string, std::string>> book_data;
//tbb::concurrent_unordered_map<std::string, int> category_results;
//
//std::string start_url = "https://books.toscrape.com/index.html";
//
//void update_price_stats(double price) {
//
//	double current_min = min_price.load();
//	while (current_min > price && !min_price.compare_exchange_weak(current_min, price)) {}
//
//	double current_max = max_price.load();
//	while (current_max < price && !max_price.compare_exchange_weak(current_max, price)) {}
//}
//
//std::string href_to_url(const std::string& base, const std::string& href) {
//	if (href.find("http") == 0) {
//		return href;
//	}
//	else if (href.find("//") == 0) {
//		return "https:" + href;
//	}
//	else if (href.find("/") == 0) {
//		return base + href.substr(1);
//	}
//
//	std::string ret = base;
//	std::string tmp = href;
//	while (tmp.find("../") == 0) {
//		tmp = tmp.substr(3);
//		auto pos = ret.find_last_of('/', ret.length() - 2);
//		if (pos != std::string::npos) {
//			ret = ret.substr(0, pos + 1);
//		}
//	}
//	return ret + tmp;
//}
//
//std::string fetch(const std::string& url, int attempts = 3, int timeout_ms = 5000) {
//	static std::mutex cout_mutex;
//	{
//		std::lock_guard<std::mutex> lock(cout_mutex);
//		std::cout << std::this_thread::get_id() << std::endl;
//		std::cout << "Fetching URL: " << url << std::endl; 
//	}
//
//	for (int i = 0; i < attempts; ++i) {
//		cpr::Response response = cpr::Get(cpr::Url{ url }, cpr::Timeout{ timeout_ms });
//		if (response.status_code == 200) {
//			return response.text;
//		}
//		else {
//			std::cout << "Attempt " << (i + 1) << " failed for URL: " << url << " with status code: " << response.status_code << std::endl;
//			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//		}
//	}
//
//	std::cout << "All attempts failed for URL: " << url << std::endl;
//	return "";
//}
//
//static const std::regex href_regex(R"(<a[^>]+href\s*=\s*["']?([^"'\s>]+\.html))", std::regex::icase | std::regex::optimize);
//static const std::regex title_regex(R"(<title>([\s\S]*?)</title>)", std::regex::icase | std::regex::optimize);
//static const std::regex price_regex(R"(<p class="price_color">([^<]+)</p>)", std::regex::optimize);
//static const std::regex availability_regex(R"((?:In stock|Available).*?\((\d+)\s+available\))", std::regex::icase | std::regex::optimize); static const std::regex rating_regex(R"(<p class="star-rating\s+(\w+))", std::regex::optimize);
//static const std::regex description_regex(R"(<meta[^>]+name=["']description["'][^>]+content=["']([\s\S]*?)["'])", std::regex::icase | std::regex::optimize);
//static const std::regex product_regex(R"(product_main")", std::regex::optimize);
//static const std::regex results_regex(R"(<strong>(\d+)</strong>\s+results?\.)", std::regex::icase | std::regex::optimize);
//
//std::vector<std::string> extract_links(const std::string& html, const std::string& current) {
//	std::vector<std::string> links;
//
//	auto begin = std::sregex_iterator(html.begin(), html.end(), href_regex);
//	auto end = std::sregex_iterator();
//
//	std::string base = current;
//	auto pos = base.find_last_of('/');
//	if (pos != std::string::npos) {
//		base = base.substr(0, pos + 1);
//	}
//
//	for (auto it = begin; it != end; ++it) {
//		std::string link = href_to_url(base, (*it)[1].str());
//		pos = link.find("page-");
//		auto pos2 = link.find(".html");
//		if (pos != std::string::npos && pos2 != std::string::npos && pos2 > pos + 5) {
//			int page_num = std::stoi(link.substr(pos + 5, pos2 - (pos + 5)));
//			if (page_num) {
//				continue;
//			}
//		}
//		if (visited_urls.find(link) != visited_urls.end()) continue;
//		links.push_back(link);
//	}
//
//	return links;
//}
//
//void parse_book_page(const std::string& html) {
//	std::smatch m;
//	tbb::concurrent_unordered_map<std::string, std::string> data;
//
//	// Title
//	if (std::regex_search(html, m, title_regex)) {
//		std::string title = m[1].str();
//		auto pos = title.find(" | ");
//		if (pos != std::string::npos) {
//			title = title.substr(0, pos);
//		}
//		data.insert({ "title", title });
//	}
//
//	// Price
//	if (std::regex_search(html, m, price_regex)) {
//		std::string raw = m[1].str();
//		raw.erase(std::remove_if(raw.begin(), raw.end(),
//			[](unsigned char c) { return !(std::isdigit(c) || c == '.'); }),
//			raw.end());
//		if (!raw.empty()) {
//			total_price += std::stod(raw);
//			update_price_stats(std::stod(raw));
//			data.insert({ "price", raw });
//		}
//	}
//
//	// Availability
//	if (std::regex_search(html, m, availability_regex)) {
//		data.insert({ "availability", m[1].str() });
//		number_of_available_books += std::stoi(m[1].str());
//	}
//
//	// Rating
//	if (std::regex_search(html, m, rating_regex)) {
//		std::string rating = m[1].str();
//		data.insert({ "rating", rating });
//		if (rating == "Five") {
//			five_star_count++;
//		}
//	}
//
//	// Description
//	if (std::regex_search(html, m, description_regex)) {
//		data.insert({ "description", m[1].str() });
//	}
//
//	book_data.push_back(data);
//}
//
//void parse_category_results(const std::string& url, const std::string& html) {
//	std::string category;
//	auto pos1 = url.find("books/");
//	auto pos2 = url.find("_");
//	category = url.substr(pos1 + 6, pos2 - (pos1 + 6));
//
//	std::smatch m;
//	if (std::regex_search(html, m, results_regex)) {
//		int results = std::stoi(m[1].str());
//		category_results[category] = results;
//	}
//}
//
//void crawl_worker() {
//	std::string current_url;
//	while (true) {
//		if (!job_queue.try_pop(current_url)) {
//			if (active_jobs.load() == 0) break;
//			std::this_thread::yield();
//			continue;
//		}
//
//		if (!visited_urls.insert(current_url).second) continue;
//
//		active_jobs.fetch_add(1); 
//
//		std::string html = fetch(current_url);
//		if (!html.empty()) {
//			auto links = extract_links(html, current_url);
//
//			if (std::regex_search(html, std::regex(R"(product_main")"))) {
//				parse_book_page(html);
//			}
//			else if (current_url.find("category") != std::string::npos) {
//				parse_category_results(current_url, html);
//			}
//
//			for (auto& link : links) {
//				if (visited_urls.find(link) == visited_urls.end())
//				{
//					job_queue.push(link);
//				}
//			}
//		}
//		active_jobs.fetch_sub(1); 
//	}
//}
//
//int main() {
//
//	auto time_start = std::chrono::high_resolution_clock::now();
//	job_queue.push({ start_url, 0 });
//
//	for (int i = 2; i <= 50; ++i) {
//		job_queue.push("https://books.toscrape.com/catalogue/page-" + std::to_string(i) + ".html");
//	}
//
//	const int num_workers = std::thread::hardware_concurrency() * 16;
//	std::vector<std::thread> workers;
//
//	std::cout << "Starting crawl with " << num_workers << " threads." << std::endl;
//
//	for (int i = 0; i < num_workers; ++i) {
//		workers.emplace_back([]() { crawl_worker(); });
//	}
//
//	for (auto& t : workers) t.join();
//
//	auto time_end = std::chrono::high_resolution_clock::now();
//
//	//for (const auto& book : book_data) {
//	//	std::cout << "Title: " << book.at("title") << std::endl;
//	//	std::cout << "Price: " << book.at("price") << std::endl;
//	//	std::cout << "Availability: " << book.at("availability") << std::endl;
//	//	std::cout << "Rating: " << book.at("rating") << std::endl;
//	//	std::cout << "Description: " << book.at("description") << std::endl;
//	//	std::cout << "----------------------------------------" << std::endl;
//	//}
//
//	std::ofstream file("book_data.txt");
//
//	file << "Total books found: " << book_data.size() << std::endl;
//	file << "Average price of all books: " << total_price / book_data.size() << std::endl;
//	file << "Minimum book price: " << min_price.load() << std::endl;
//	file << "Maximum book price: " << max_price.load() << std::endl;
//	file << "Books with 5-star rating: " << five_star_count << std::endl;
//	file << "Total stock: " << number_of_available_books.load() << ", average availability: " << number_of_available_books.load() / book_data.size() << std::endl;
//	file << "Crawling completed. Visited " << visited_urls.size() << " unique URLs." << std::endl;
//	file << "Time taken: " << std::chrono::duration_cast<std::chrono::seconds>(time_end - time_start).count() << " seconds." << std::endl;
//
//	for (auto& [category, results] : category_results)
//		file << "Category: " << category << ", Results: " << results << std::endl;
//
//	return 0;
//}