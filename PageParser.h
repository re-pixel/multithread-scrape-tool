#pragma once
#include <string>
#include <vector>
#include <regex>
#include <tbb/concurrent_unordered_map.h>

class PageParser {
public:
    static std::vector<std::string> extract_links(const std::string& html, const std::string& current);
    static void parse_book_page(const std::string& html);
    static void parse_category_results(const std::string& url, const std::string& html);

private:
    static std::string href_to_url(const std::string& base, const std::string& href);
};
