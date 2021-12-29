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
    image = matrix(width, std::vector<RGB>(height));
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            RGB *pixel = new RGB(source[id++]);
            if (nChannels > 1) {
                pixel->g = source[id++];
                if (nChannels > 2) {
                    pixel->b = source[id++];
                }
            }
            image[i][j] = *pixel;
        }
    }

    UpdateFrequency();
}

// Applies min-max contrast stretching (percentile contrast stretching if ignorance is not zero)
void Image::EnhanceGlobalContrast(int ignorance) {
    expectBetween(ignorance, 0, 50, "ignorance percentage");
    uint8_t min = GetMinMaxIntensityLevel(ignorance, 0, 1);
    uint8_t max = GetMinMaxIntensityLevel(ignorance, 0, -1);
    for (auto &row: image) {
        for (auto &pixel: row) {
            pixel.r = std::max(pixel.r, min);
            pixel.r = std::min(pixel.r, max);
            pixel.r = min == max ? 0 : ((pixel.r - min) * maxColorIntensity) / (max - min);
        }
    }

    if (nChannels > 1) {
        min = GetMinMaxIntensityLevel(ignorance, 1, 1);
        max = GetMinMaxIntensityLevel(ignorance, 1, -1);
        for (auto &row: image) {
            for (auto &pixel: row) {
                pixel.g = std::max(pixel.g, min);
                pixel.g = std::min(pixel.g, max);
                pixel.g = min == max ? 0 : ((pixel.g - min) * maxColorIntensity) / (max - min);
            }
        }

        if (nChannels > 2) {
            min = GetMinMaxIntensityLevel(ignorance, 2, 1);
            max = GetMinMaxIntensityLevel(ignorance, 2, -1);
            for (auto &row: image) {
                for (auto &pixel: row) {
                    pixel.b = std::max(pixel.b, min);
                    pixel.b = std::min(pixel.b, max);
                    pixel.b = min == max ? 0 : ((pixel.b - min) * maxColorIntensity) / (max - min);
                }
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
            frequency[0][pixel.r]++;
            if (nChannels > 1) {
                frequency[1][pixel.g]++;
                if (nChannels > 2) {
                    frequency[2][pixel.b]++;
                }
            }
        }
    }
}

void Image::PrintPixelIntensityFrequency() {
    std::cout << "Image color frequency: " << std::endl;
    std::cout << "Channel 1 | " << (nChannels > 1 ? "R:" : "grayscale:") << std::endl;
    for (int i = 0; i <= maxColorIntensity; ++i) {
        std::cout << i << ": " << frequency[0][i] << std::endl;
    }
    if (nChannels > 1) {
        std::cout << "Channel 2 | G:" << std::endl;
        for (int i = 0; i <= maxColorIntensity; ++i) {
            std::cout << i << ": " << frequency[1][i] << std::endl;
        }
        if (nChannels > 2) {
            std::cout << "Channel 3 | B:" << std::endl;
            for (int i = 0; i <= maxColorIntensity; ++i) {
                std::cout << i << ": " << frequency[2][i] << std::endl;
            }
        }
    }
    std::cout << std::endl;
}

std::vector<uint8_t> Image::GetImage() const {
    std::vector<uint8_t> result;
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            result.push_back(image[i][j].r);
            if (nChannels > 1) {
                result.push_back(image[i][j].g);
                if (nChannels > 2) {
                    result.push_back(image[i][j].b);
                }
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
