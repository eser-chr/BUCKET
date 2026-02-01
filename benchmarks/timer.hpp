#pragma once
#include <chrono>
#include <iostream>
#include <vector>

class MyTimer {
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    //   std::chrono::milliseconds end;
    std::vector<std::chrono::high_resolution_clock::time_point> _laps;

   public:
    MyTimer() : start(std::chrono::high_resolution_clock::now()) { _laps.push_back(start); }

    double get() {
        end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    }

    void add_lap() { _laps.push_back(std::chrono::high_resolution_clock::now()); }

    void print_times() {
        std::cout << "times: ";
        for (size_t i = 1; i < _laps.size(); ++i) {
            std::cout << std::chrono::duration_cast<std::chrono::duration<double>>(_laps[i] -
                                                                                   _laps[i - 1])
                             .count();
            if (i < _laps.size() - 1) std::cout << ",";
        }
        std::cout << std::endl;
    }
};