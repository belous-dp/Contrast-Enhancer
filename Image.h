//
// Created by Danila Belous on 26.12.2021 at 01:08.
//

#ifndef CONTRASTENHANCER_IMAGE_H
#define CONTRASTENHANCER_IMAGE_H

#include <vector>
#include <cstdint>

namespace LN {

    class Image {
    public:
        explicit Image(std::vector<uint8_t> source, int numberOfChannels, int width, int height, int maxColorIntensity);

        std::vector<uint8_t> GetImage() const;

        void EnhanceGlobalContrast(int ignorance);

        void PrintPixelIntensityFrequency();

    private:

        struct RGB {
            uint8_t r;
            uint8_t g;
            uint8_t b;

            explicit RGB(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) :
                    r(r), g(g), b(b) {
            }
        };

        using matrix = std::vector<std::vector<RGB>>;

        int nChannels;
        int width, height;
        int maxColorIntensity;
        matrix image;
        std::vector<std::vector<int>> frequency;

        uint8_t GetMinMaxIntensityLevel(int ignorance, int channel, int step);

        void UpdateFrequency();

        static void expectBetween(int number, int from, int to, const std::string &varName);

        void expectPositive(int number, const std::string &varName);

        void expectNonNegative(int number, const std::string &varName);
    };
}

#endif //CONTRASTENHANCER_IMAGE_H
