#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <cerrno>
#include "Image.h"
#include "Time.h"

std::invalid_argument GetErr(const std::string &message);

int GetInt(const char *sNumber);

int main(int argc, char *argv[]) {

    Time *timestamps = new Time(3);

#ifndef NDEBUG
    timestamps->SaveCurrent("Overall");
    timestamps->SaveCurrent("Reading input");
#endif

    if (argc != 5) {
        std::cout << "Usage: hw5 [number of threads, 0=default] [input file name] [output file name] " <<
                  "[ignore percentage (number of pixels with extreme value to be ignored), integer in [0;50]]"
                  << std::endl;
        throw GetErr("Expected 4 arguments, found: " + std::to_string(argc - 1));
    }

    int numThreads = 0;
    try {
        numThreads = GetInt(argv[1]);
    } catch (std::invalid_argument &e) {
        // Do nothing.
    }
#ifdef _OPENMP
    Image::setOmpParameters(numThreads);
#else
    if (numThreads > 1) {
        std::cout << "Warning: OpenMP is not connected but trying to set its parameters." << std::endl
                  << "The program will run in one thread mode." << std::endl << std::endl;
        numThreads = 1;
    }
#endif

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
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(inStr), {});

    assert(buffer.size() == width * height * numberOfChannels);

#ifndef NDEBUG
    timestamps->PrintDelta("Reading input");
    timestamps->SaveCurrent("Generating image");
#endif

    std::vector<uint8_t> result;
    try {
        timestamps->SaveCurrent("Testing");
        auto *img = new Image(buffer, numberOfChannels, width, height, maxColorValue);

#ifndef NDEBUG
//        img->PrintPixelIntensityFrequency();
        timestamps->PrintDelta("Generating image");
        timestamps->SaveCurrent("Enhancing contrast");
#endif

        const int ignorance = GetInt(argv[4]);
        img->EnhanceGlobalContrast(ignorance);

        std::cout << "Time (" << numThreads << " thread(s)): "
                  << timestamps->GetDelta("Testing").wall * 1000. << " ms\n";

#ifndef NDEBUG
        timestamps->PrintDelta("Enhancing contrast");
//        img->PrintPixelIntensityFrequency();
        timestamps->SaveCurrent("Getting image");
#endif

        result = img->GetImage();

#ifndef NDEBUG
        timestamps->PrintDelta("Getting image");
        timestamps->SaveCurrent("Printing output");
#endif

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

#ifndef NDEBUG
    timestamps->PrintDelta("Printing output");
    timestamps->PrintDelta("Overall");
#endif

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
