#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <cerrno>
#include "Image.h"

//  Windows
#ifdef _WIN32

#include <Windows.h>

double get_wall_time() {
    LARGE_INTEGER time, freq;
    if (!QueryPerformanceFrequency(&freq)) {
        //  Handle error
        return 0;
    }
    if (!QueryPerformanceCounter(&time)) {
        //  Handle error
        return 0;
    }
    return (double) time.QuadPart / freq.QuadPart;
}

double get_cpu_time() {
    FILETIME a, b, c, d;
    if (GetProcessTimes(GetCurrentProcess(), &a, &b, &c, &d) != 0) {
        //  Returns total user time.
        //  Can be tweaked to include kernel times as well.
        return
                (double) (d.dwLowDateTime |
                          ((unsigned long long) d.dwHighDateTime << 32)) * 0.0000001;
    } else {
        //  Handle error
        return 0;
    }
}

//  Posix/Linux
#else
#include <time.h>
#include <sys/time.h>
double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
double get_cpu_time(){
    return (double)clock() / CLOCKS_PER_SEC;
}
#endif

std::invalid_argument GetErr(const std::string &message);

int GetInt(const char *sNumber);

int main(int argc, char *argv[]) {
    const double wall0 = get_wall_time();
    const double cpu0 = get_cpu_time();

    if (argc != 5) {
        //todo readme
        throw GetErr("Expected 4 arguments, found: " + std::to_string(argc - 1));
    }
    const std::string inFileName = argv[2];
    std::ifstream inStr(inFileName, std::ios::in | std::ios::binary);
    if (!inStr) {
        throw GetErr("Could not read from the input file: " + inFileName + ". File does not exist or corrupted");
    }

    std::string fileType;
    inStr >> fileType;

    int numberOfChannels;
    if (fileType == "P5") {
        numberOfChannels = 1;
    } else if (fileType == "P6") {
        numberOfChannels = 3;
    } else {
        throw GetErr("Not an PGM or PPM file");
    }

    std::string line;
    do {
        std::getline(inStr, line);
    } while (line.empty() || line[0] == '#');
    size_t splitPos = line.find(' ');
    const int width = GetInt(line.substr(0, splitPos).data());
    const int height = GetInt(line.substr(splitPos + 1, line.size()).data());
    int maxColorValue;
    inStr >> maxColorValue;
    inStr.ignore(1, '\n');
    const std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(inStr), {});

    assert(buffer.size() == width * height * numberOfChannels);
    assert(inStr); // some bytes left

    std::vector<uint8_t> result;
    try {
        auto *img = new LN::Image(buffer, numberOfChannels, width, height, maxColorValue);

#ifndef DNDEBUG
        img->PrintPixelIntensityFrequency();
#endif

        const int ignorance = GetInt(argv[4]);
        img->EnhanceGlobalContrast(ignorance);

#ifndef DNDEBUG
        img->PrintPixelIntensityFrequency();
#endif

        result = img->GetImage();

        //todo прикрутить время
        //todo многопоточность

    } catch (std::invalid_argument &e) {
        throw GetErr("Invalid image: " + std::string(e.what()));
    }

    const std::string outFileName = argv[3];
    std::ofstream outStr(outFileName, std::ios::out | std::ios::binary);
    if (!outStr) {
        throw GetErr("Could not write to the output file: " + inFileName);
    }
    outStr << fileType << '\n' << width << ' ' << height << '\n' << maxColorValue << '\n';
    outStr.write(reinterpret_cast<char *>(result.data()), result.size());

    const double wall1 = get_wall_time();
    const double cpu1 = get_cpu_time();

    std::cout << "Wall time passed: " << wall1 - wall0 << "s" << std::endl;
    std::cout << "CPU time passed: " << cpu1 - cpu0 << "s" << std::endl;

    return 0;
}

int GetInt(const char *sNumber) {
    errno = 0;
    const char *pIgn = sNumber;
    char *pEnd = nullptr;
    const int num = strtol(pIgn, &pEnd, 10);
    if (pIgn != pEnd && errno == 0 && !*pEnd) {
        return num;
    }
    throw GetErr("Expected integer value. Found: " + std::string(sNumber));
}

std::invalid_argument GetErr(const std::string &message) { return std::invalid_argument(message); }
