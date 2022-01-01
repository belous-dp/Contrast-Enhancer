//
// Created by Danila Belous on 26.12.2021 at 01:08.
//

#ifndef CONTRASTENHANCER_IMAGE_H
#define CONTRASTENHANCER_IMAGE_H

#include <array>
#include <vector>
#include <cstdint>

class Image {
public:
    explicit Image(std::vector<uint8_t> &source, int numberOfChannels, int width, int height, int maxColorIntensity);

    [[nodiscard]] std::vector<uint8_t> GetImage() const;

    void EnhanceGlobalContrast(int ignorance);

    void PrintPixelIntensityFrequency();

    static void setOmpParameters(int numThreads);

private:

    static constexpr int MIN_NUM_CHANNELS = 1;
    static constexpr int MAX_NUM_CHANNELS = 3;
    size_t size;
    int nChannels;
    int width, height;
    int maxColorIntensity;
    std::vector<uint8_t> image;
    std::vector<int> frequency;

    std::pair<uint8_t, uint8_t> GetMinMaxIntensityLevel(int ignorance, int channel);

    void UpdateFrequency();

    static void expectBetween(int number, int from, int to, const std::string &varName);

    static void expectPositive(int number, const std::string &varName);

    static void expectNonNegative(int number, const std::string &varName);
};

#endif //CONTRASTENHANCER_IMAGE_H
