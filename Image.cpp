//
// Created by Danila Belous on 26.12.2021 at 03:34.
//

#include <iostream>
#include <cassert>
#include <exception>
#include "Image.h"

using namespace LN;

Image::Image(std::vector<uint8_t> source, int numberOfChannels, int width, int height, int maxColorIntensity) {

    expectBetween(numberOfChannels, 1, 3, "number of channels");
    this->nChannels = numberOfChannels;

    expectPositive(width, "width");
    this->width = width;

    expectPositive(height, "height");
    this->height = height;

    expectNonNegative(maxColorIntensity, "maximum color intensity");
    this->maxColorIntensity = maxColorIntensity;

    size_t id = 0;

    image = matrix(width, std::vector<pixel>(height));
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            for (int k = 0; k < numberOfChannels; ++k) {
                image[i][j][k] = source[id++];
            }
        }
    }

    UpdateFrequency();
}

// Applies min-max contrast stretching (percentile contrast stretching if ignorance is not zero)
void Image::EnhanceGlobalContrast(int ignorance) {
    expectBetween(ignorance, 0, 50, "ignorance percentage");
    for (int i = 0; i < nChannels; ++i) {
        uint8_t min = GetMinMaxIntensityLevel(ignorance, i, 1);
        uint8_t max = GetMinMaxIntensityLevel(ignorance, i, -1);
        for (auto &row: image) {
            for (auto &pixel: row) {
                pixel[i] = std::max(pixel[i], min);
                pixel[i] = std::min(pixel[i], max);
                pixel[i] = min == max ? 0 : ((pixel[i] - min) * maxColorIntensity) / (max - min);
            }
        }
    }

    UpdateFrequency();
}

void Image::UpdateFrequency() {
    frequency.clear();
    frequency.resize(nChannels, std::vector<int>(maxColorIntensity + 1, 0));
    for (auto &row: image) {
        for (auto &pixel: row) {
            for (int i = 0; i < nChannels; ++i) {
                frequency[i][pixel[i]]++;
            }
        }
    }
}

void Image::PrintPixelIntensityFrequency() {
    std::cout << "Image color frequency: " << std::endl;
    for (int i = 0; i < nChannels; ++i) {
        std::cout << "Channel " << i + 1 << std::endl;
        for (int j = 0; j <= maxColorIntensity; ++j) {
            std::cout << j << ": " << frequency[i][j] << std::endl;
        }
    }
    std::cout << std::endl;
}

std::vector<uint8_t> Image::GetImage() const {
    std::vector<uint8_t> result;
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            for (int k = 0; k < nChannels; ++k) {
                result.push_back(image[i][j][k]);
            }
        }
    }
    return result;
}

void Image::expectPositive(int number, const std::string &varName) {
    expectBetween(number, 1, INT_MAX - 1, varName);
}

void Image::expectNonNegative(int number, const std::string &varName) {
    expectPositive(number - 1, varName);
}

void Image::expectBetween(int number, int from, int to, const std::string &varName) {
    if (number < from || number > to) {
        throw std::invalid_argument(varName + " must be in interval [" + std::to_string(from) +
                                    ";" + std::to_string(to) + "]");
    }
}

uint8_t Image::GetMinMaxIntensityLevel(int ignorance, int channel, int step) {
    assert(0 <= ignorance && ignorance <= 50);
    assert(0 <= channel && channel < nChannels);
    assert(step == -1 || step == 1);
    uint32_t toSkip = width * height * ignorance / 100;
    uint32_t skipped = 0;
    int first = (step > 0 ? 0 : maxColorIntensity);
    int last = (step > 0 ? maxColorIntensity + 1 : -1);
    for (int i = first; i != last; i += step) {
        if (skipped + frequency[channel][i] > toSkip) {
            return i;
        } else {
            skipped += frequency[channel][i];
        }
    }
    assert(false);
    return last;
}
