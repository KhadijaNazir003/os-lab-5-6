#include "my_logger.h"
#include <iostream>

MyLogger::MyLogger(const std::string& filename) {
    log_file.open(filename, std::ios::out | std::ios::app);
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

MyLogger::~MyLogger() {
    if (log_file.is_open()) {
        log_file.close();
    }
}

void MyLogger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (log_file.is_open()) {
        log_file << message;
        log_file.flush();
    }
}
