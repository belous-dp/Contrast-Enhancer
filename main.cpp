#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <cerrno>
#include "Image.h"

std::invalid_argument GetErr(const std::string &message);

int main(int argc, char *argv[]) {
    if (argc != 5) {
        //todo readme
        throw GetErr("Expected 4 arguments, found: " + std::to_string(argc - 1));
    }
    std::string inFileName = argv[2];
    std::ifstream inStr(inFileName, std::ios::in | std::ios::binary);
    if (!inStr) {
        throw GetErr("Could not read from the input file: " + inFileName + ". File does not exist or corrupted");
    }
    int width, height, maxColorValue;
    std::string fileType;
    inStr >> fileType >> width >> height >> maxColorValue;
    int numberOfChannels;
    if (fileType == "P5") {
        numberOfChannels = 1;
    } else if (fileType == "P6") {
        numberOfChannels = 3;
    } else {
        throw GetErr("Not an PGM or PPM file");
    }
    inStr.ignore(2, '\n');
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(inStr), {});

    assert(buffer.size() == width * height * numberOfChannels);
    assert(inStr); // some bytes left

    std::vector<uint8_t> result;
    try {
        auto *img = new LN::Image(buffer, numberOfChannels, width, height, maxColorValue);

//#ifndef DNDEBUG
//        img->PrintPixelIntensityFrequency();
//#endif

        errno = 0;
        const char *pIgn = argv[4];
        char *pEnd = nullptr;
        int ignorance = std::strtol(pIgn, &pEnd, 10);
        if (!(pIgn != pEnd && errno == 0 && !*pEnd) || ignorance < 0 || ignorance > 50) {
            throw GetErr("bad ignorance percentage value: " + std::string(argv[4]));
        }

        img->EnhanceGlobalContrast(ignorance);

//#ifndef DNDEBUG
//        img->PrintPixelIntensityFrequency();
//#endif

        result = img->GetImage();

        //todo прикрутить время
        //todo многопоточность

    } catch (std::invalid_argument &e) {
        throw GetErr("Invalid image: " + std::string(e.what()));
    }

    std::string outFileName = argv[3];
    std::ofstream outStr(outFileName, std::ios::out | std::ios::binary);
    if (!outStr) {
        throw GetErr("Could not write to the output file: " + inFileName);
    }
    outStr << fileType << '\n' << width << ' ' << height << '\n' << maxColorValue << '\n';
    outStr.write(reinterpret_cast<char *>(result.data()), result.size());

    return 0;
}

std::invalid_argument GetErr(const std::string &message) { return std::invalid_argument(message); }
