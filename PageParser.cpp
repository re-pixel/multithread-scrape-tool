#include "PageParser.h"
#include "Structs.h"
#include <algorithm>

static const std::regex href_regex(R"(<a[^>]+href\s*=\s*["']?([^"'\s>]+\.html))", std::regex::icase | std::regex::optimize);
static const std::regex title_regex(R"(<title>([\s\S]*?)</title>)", std::regex::icase | std::regex::optimize);
static const std::regex price_regex(R"(<p class="price_color">([^<]+)</p>)", std::regex::optimize);
static const std::regex availability_regex(R"((?:In stock|Available).*?\((\d+)\s+available\))", std::regex::icase | std::regex::optimize);
static const std::regex rating_regex(R"(<p class="star-rating\s+(\w+))", std::regex::optimize);
static const std::regex description_regex(R"(<meta[^>]+name=["']description["'][^>]+content=["']([\s\S]*?)["'])", std::regex::icase | std::regex::optimize);
static const std::regex results_regex(R"(<strong>(\d+)</strong>\s+results?\.)", std::regex::icase | std::regex::optimize);

std::string PageParser::href_to_url(const std::string& base, const std::string& href) {
    if (href.find("http") == 0) return href;
    else if (href.find("//") == 0) return "https:" + href;
    else if (href.find("/") == 0) return base + href.substr(1);

    std::string ret = base;
    std::string tmp = href;
    while (tmp.find("../") == 0) {
        tmp = tmp.substr(3);
        auto pos = ret.find_last_of('/', ret.length() - 2);
        if (pos != std::string::npos) ret = ret.substr(0, pos + 1);
    }
    return ret + tmp;
}

std::vector<std::string> PageParser::extract_links(const std::string& html, const std::string& current) {
    std::vector<std::string> links;

    auto begin = std::sregex_iterator(html.begin(), html.end(), href_regex);
    auto end = std::sregex_iterator();

    std::string base = current;
    auto pos = base.find_last_of('/');
    if (pos != std::string::npos) base = base.substr(0, pos + 1);

    for (auto it = begin; it != end; ++it) {
        std::string link = href_to_url(base, (*it)[1].str());
        pos = link.find("page-");
        auto pos2 = link.find(".html");
        if (pos != std::string::npos && pos2 != std::string::npos && pos2 > pos + 5) {
            int page_num = std::stoi(link.substr(pos + 5, pos2 - (pos + 5)));
            if (page_num /*page_num>7*/) {
                continue;
            }
        }
        if (Structs::visited_urls.find(link) != Structs::visited_urls.end()) continue;
        links.push_back(link);
    }

    return links;
}

void PageParser::parse_book_page(const std::string& html) {
    std::smatch m;
    tbb::concurrent_unordered_map<std::string, std::string> data;

    // Title
    if (std::regex_search(html, m, title_regex)) {
        std::string title = m[1].str();
        auto pos = title.find(" | ");
        if (pos != std::string::npos) title = title.substr(0, pos);
        data.insert({ "title", title });
    }

    // Price
    if (std::regex_search(html, m, price_regex)) {
        std::string raw = m[1].str();
        raw.erase(std::remove_if(raw.begin(), raw.end(),
            [](unsigned char c) { return !(std::isdigit(c) || c == '.'); }), raw.end());
        if (!raw.empty()) {
            Structs::total_price += std::stod(raw);
            Structs::update_price_stats(std::stod(raw));
            data.insert({ "price", raw });
        }
    }

    // Availability
    if (std::regex_search(html, m, availability_regex)) {
        data.insert({ "availability", m[1].str() });
        Structs::number_of_available_books += std::stoi(m[1].str());
    }

    // Rating
    if (std::regex_search(html, m, rating_regex)) {
        std::string rating = m[1].str();
        data.insert({ "rating", rating });
        if (rating == "Five") Structs::five_star_count++;
    }

    // Description
    if (std::regex_search(html, m, description_regex)) {
        data.insert({ "description", m[1].str() });
    }

    Structs::book_data.push_back(data);
}

void PageParser::parse_category_results(const std::string& url, const std::string& html) {
    std::string category;
    auto pos1 = url.find("books/");
    auto pos2 = url.find("_");
    category = url.substr(pos1 + 6, pos2 - (pos1 + 6));

    std::smatch m;
    if (std::regex_search(html, m, results_regex)) {
        int results = std::stoi(m[1].str());
        Structs::category_results[category] = results;
    }
}
