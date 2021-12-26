#include <iostream>
#include <fstream>
#include <vector>
#include "Image.h"

int main(int argc, char *argv[]) {
    if (argc != 5) {
        std::cout << "expected 4 arguments, found: " << argc << std::endl;
        for (int i = 0; i < argc; ++i) {
            std::cout << i << ": " << argv[i] << std::endl;
        }
        return 0;
        //todo readme
    }
    std::string inFileName = argv[2];
//    std::cout << inFileName.length() << " " << inFileName << std::endl;
    std::ifstream input(inFileName, std::ios::in | std::ios::binary);
    int width, height, maxColorValue;
    std::string fileType;
//    bool fileIsOpen = input.is_open();
//    std::cout << fileIsOpen << std::endl;
    input >> fileType >> width >> height >> maxColorValue;
    //todo навалить тут исключений
    int numberOfChannels;
    if (fileType == "P5") {
        numberOfChannels = 1;
    } else if (fileType == "P6") {
        numberOfChannels = 3;
    }
    int uselessWhitespaces;
    input >> uselessWhitespaces;
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(input), {});
//    std::cout << buffer.size() << std::endl;
    
    while (input) {
        std::cout << "left characters" << std::endl;
        input.ignore(1);
    }


//    std::cout << "Hello, World!" << std::endl;

    auto *img = new LN::Image(buffer, numberOfChannels, width, height, maxColorValue);
    
    img->PrintPixelIntensityFrequency();
    img->EnhanceGlobalContrast(atoi(argv[4]));
    img->PrintPixelIntensityFrequency();

    std::vector<uint8_t> result = img->GetImage();
    
    std::string outFileName = argv[3];
    std::ofstream out(outFileName, std::ios::out | std::ios::binary);
    out << fileType << '\n' << width << ' ' << height << '\n' << maxColorValue << '\n';
    out.write(reinterpret_cast<char*>(result.data()), result.size());

    //todo прикрутить время
    //todo многопоточность

    return 0;
}
