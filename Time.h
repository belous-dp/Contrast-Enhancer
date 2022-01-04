//
// Created by Danila Belous on 30.12.2021 at 13:25.
//

#ifndef CONTRASTENHANCER_TIME_H
#define CONTRASTENHANCER_TIME_H

#ifdef _WIN32
//  Windows
#include <Windows.h>

#else
//  Posix/Linux
#include <time.h>
#include <sys/time.h>
#endif

#include <string>
#include <unordered_map>
#include <iomanip>

class Time {
public:
    struct AllTime {
        double wall;
        double cpu;
    };

    void SaveCurrent(const std::string &label);

    Time::AllTime GetDelta(const std::string &label);

    void PrintDelta(const std::string &label);

    void ShowWarnings(const bool &flag);

    explicit Time(int precision = 0) {
        std::cout << std::fixed << std::setprecision(precision);
        warnings = true;
    }

private:

    bool warnings;

    std::unordered_map<std::string, AllTime> data_;

    static AllTime GetAllTime();

// Copy-paste solution from Stack Overflow
#ifdef _WIN32

    static double GetWallTime() {
        LARGE_INTEGER time, freq;
        if (!QueryPerformanceFrequency(&freq)) {
            //  Handle error
            return 0;
        }
        if (!QueryPerformanceCounter(&time)) {
            //  Handle error
            return 0;
        }
        return (double) time.QuadPart / (double) freq.QuadPart;
    }

    static double GetCpuTime() {
        FILETIME a, b, c, d;
        if (GetProcessTimes(GetCurrentProcess(), &a, &b, &c, &d) != 0) {
            //  Returns total user time.
            //  Can be tweaked to include kernel times as well.
            return (double) (d.dwLowDateTime | ((unsigned long long) d.dwHighDateTime << 32)) * 0.0000001;
        } else {
            //  Handle error
            return 0;
        }
    }

#else

    double GetWallTime() {
        struct timeval time;
        if (gettimeofday(&time, NULL)) {
            //  Handle error
            return 0;
        }
        return (double) time.tv_sec + (double) time.tv_usec * 0.000001;
    }

    double GetCpuTime() {
        return (double) clock() / CLOCKS_PER_SEC;
    }

#endif

};


#endif //CONTRASTENHANCER_TIME_H
