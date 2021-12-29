//
// Created by Danila Belous on 26.12.2021 at 01:08.
//

#ifndef CONTRASTENHANCER_IMAGE_H
#define CONTRASTENHANCER_IMAGE_H

#define MAX_NUM_CHANNELS 3

#include <array>
#include <vector>
#include <cstdint>

namespace LN {

    class Image {
    public:
        explicit Image(std::vector<uint8_t> source, int numberOfChannels, int width, int height, int maxColorIntensity);

        [[nodiscard]] std::vector<uint8_t> GetImage() const;

        void EnhanceGlobalContrast(int ignorance);

        void PrintPixelIntensityFrequency();

    private:

        using pixel = std::array<uint8_t, MAX_NUM_CHANNELS>;
        using matrix = std::vector<std::vector<pixel>>;

        int nChannels;
        int width, height;
        int maxColorIntensity;
        matrix image;
        std::vector<std::vector<int>> frequency;

        uint8_t GetMinMaxIntensityLevel(int ignorance, int channel, int step);

        void UpdateFrequency();

        static void expectBetween(int number, int from, int to, const std::string &varName);

        static void expectPositive(int number, const std::string &varName);

        static void expectNonNegative(int number, const std::string &varName);
    };
}

#endif //CONTRASTENHANCER_IMAGE_H
