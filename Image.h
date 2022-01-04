//
// Created by Danila Belous on 26.12.2021 at 01:08.
//

#ifndef CONTRASTENHANCER_IMAGE_H
#define CONTRASTENHANCER_IMAGE_H

#include <vector>

class Image {
public:
    explicit Image(std::vector<uint8_t> &source, int numberOfChannels, int width, int height, int maxColorIntensity);

    [[nodiscard]] std::vector<uint8_t> GetImage() const;

    void EnhanceGlobalContrast(double ignore);

    void PrintPixelIntensityFrequency();

    static void setOmpParameters(int numThreads);

private:

    static constexpr int MIN_NUM_CHANNELS = 1;
    static constexpr int MAX_NUM_CHANNELS = 3;
    static constexpr int MAX_COLOR_VALUE = 255;
    uint32_t size;
    int nChannels;
    int width, height;
    int maxColorIntensity;
    std::vector<uint8_t> image;
    std::vector<int> frequency;

    void GetMinMaxIntensityLevel(double ignore, uint8_t &min, uint8_t &max);

    void UpdateFrequency();

    static void expectBetween(int number, int from, int to, const std::string &varName);

    static void expectPositive(int number, const std::string &varName);

    static void expectNonNegative(int number, const std::string &varName);
};

#endif //CONTRASTENHANCER_IMAGE_H
