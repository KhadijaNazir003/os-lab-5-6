#ifndef MY_LOGGER_H
#define MY_LOGGER_H

#include <fstream>
#include <mutex>
#include <string>

class MyLogger {
private:
    std::ofstream log_file;
    std::mutex log_mutex;
    
public:
    MyLogger(const std::string& filename);
    ~MyLogger();
    void log(const std::string& message);
};

#endif
