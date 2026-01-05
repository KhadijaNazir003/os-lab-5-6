#ifndef MY_LOGGER_IMPROVED_H
#define MY_LOGGER_IMPROVED_H

#include <fstream>
#include <mutex>
#include <string>
#include <vector>
#include <thread>
#include <condition_variable>
#include <queue>

class MyLogger {
private:
    std::ofstream log_file;
    std::mutex queue_mutex;
    std::condition_variable cv;
    std::queue<std::string> message_queue;
    std::thread writer_thread;
    bool should_stop;
    
    // Background thread that writes to file
    void write_loop() {
        std::vector<std::string> local_batch;
        
        while (true) {
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                
                // Wait for messages or stop signal
                cv.wait(lock, [this] { 
                    return !message_queue.empty() || should_stop; 
                });
                
                // Move messages to local batch
                while (!message_queue.empty()) {
                    local_batch.push_back(std::move(message_queue.front()));
                    message_queue.pop();
                }
            }
            
            // Write all messages at once (outside the lock!)
            for (const auto& msg : local_batch) {
                log_file << msg;
            }
            
            // Only flush every batch, not every message
            if (!local_batch.empty()) {
                log_file.flush();
            }
            
            local_batch.clear();
            
            // Exit if stopping and queue is empty
            if (should_stop && message_queue.empty()) {
                break;
            }
        }
    }
    
public:
    MyLogger(const std::string& filename) : should_stop(false) {
        log_file.open(filename, std::ios::out | std::ios::trunc);
        if (!log_file.is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
        
        // Start background writer thread
        writer_thread = std::thread(&MyLogger::write_loop, this);
    }
    
    ~MyLogger() {
        // Signal thread to stop
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            should_stop = true;
        }
        cv.notify_one();
        
        // Wait for thread to finish
        if (writer_thread.joinable()) {
            writer_thread.join();
        }
        
        if (log_file.is_open()) {
            log_file.close();
        }
    }
    
    // Just add message to queue and return immediately
    void log(const std::string& message) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            message_queue.push(message);
        }
        cv.notify_one();
    }
};

#endif // MY_LOGGER_IMPROVED_H
