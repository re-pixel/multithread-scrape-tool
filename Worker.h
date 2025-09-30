#pragma once
#include <string>

class Worker {
public:
    void operator()();
private:
    std::string fetch(const std::string& url, int attempts = 3, int timeout_ms = 5000);
};
