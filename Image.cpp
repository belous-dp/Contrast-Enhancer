//
// Created by Danila Belous on 26.12.2021 at 03:34.
//

#include <iostream>
#include <cassert>
#include <omp.h>
#include "Image.h"

Image::Image(std::vector<uint8_t> &source, int numberOfChannels, int width, int height, int maxColorIntensity) {

    expectBetween(numberOfChannels, MIN_NUM_CHANNELS, MAX_NUM_CHANNELS, "number of channels");
    this->nChannels = numberOfChannels;

    expectPositive(width, "width");
    this->width = width;

    expectPositive(height, "height");
    this->height = height;

    expectNonNegative(maxColorIntensity, "maximum color intensity");
    this->maxColorIntensity = maxColorIntensity;

    this->size = width * height * numberOfChannels;

    if (source.size() != size) {
        throw std::invalid_argument("Image expected size does not match real size");
    }
//    image = std::move(source);
    image = source;

    frequency.resize(nChannels * (maxColorIntensity + 1));
    UpdateFrequency();
}

// Applies min-max contrast stretching (percentile contrast stretching if ignorance is not zero)
void Image::EnhanceGlobalContrast(int ignorance) {
    expectBetween(ignorance, 0, 50, "ignorance percentage");

    std::vector<uint8_t> min(MAX_NUM_CHANNELS), max(MAX_NUM_CHANNELS);
#pragma omp parallel for shared(min, max, ignorance) default(none)
    for (int i = 0; i < nChannels; ++i) {
        min[i] = GetMinMaxIntensityLevel(ignorance, i, 1);
        max[i] = GetMinMaxIntensityLevel(ignorance, i, -1);
    }
    std::vector<int> cntThreadsWork;
    uint32_t numProcessed = 0;
#pragma omp parallel shared(image, ignorance, cntThreadsWork, min, max, numProcessed) default(none)
    {
#if defined(_OPENMP) && !defined(NDEBUG)
#pragma omp single
        {
            cntThreadsWork.resize(omp_get_num_threads());
        }
#endif
#pragma omp for
        for (int j = 0; j < size; j += nChannels) {
            for (int i = 0; i < nChannels; ++i) {
#ifndef NDEBUG
#ifdef _OPENMP
//#pragma omp single
//                {
//                    std::cout << "cur pixel: " << j + i << " , cur thread: " << omp_get_thread_num() << std::endl;
//                }
                cntThreadsWork[omp_get_thread_num()]++;
                //#pragma omp atomic
#else
                numProcessed++;
#endif
#endif
                image[j + i] = std::max(image[j + i], min[i]);
                image[j + i] = std::min(image[j + i], max[i]);
                image[j + i] = min[i] == max[i] ? 0 : ((image[j + i] - min[i]) * maxColorIntensity) / (max[i] - min[i]);
            }
        }
    }
    UpdateFrequency();

#ifndef NDEBUG
#ifdef _OPENMP
    numProcessed = 0;
    for (int k = 0; k < cntThreadsWork.size(); ++k) {
        std::cout << k << "th thread processed " << cntThreadsWork[k] << " pixels" << std::endl;
        numProcessed += cntThreadsWork[k];
    }
#endif
    std::cout << "Total number of processed pixels: " << numProcessed << " out of: " << size << std::endl;
#endif
}

void Image::UpdateFrequency() {
#pragma omp parallel for shared(image, frequency) default(none)
    for (int j = 0; j < size; j += nChannels) {
        for (int i = 0; i < nChannels; ++i) {
            frequency[image[j + i] + i]++;
        }
    }
}

void Image::PrintPixelIntensityFrequency() {
//    std::cout << "Image color frequency: " << std::endl;
//    for (int i = 0; i < nChannels; ++i) {
//        std::cout << "Channel " << i + 1 << std::endl;
//        for (int j = 0; j <= maxColorIntensity; ++j) {
//            std::cout << j << ": " << frequency[i][j] << std::endl;
//        }
//    }
//    std::cout << std::endl;
}

std::vector<uint8_t> Image::GetImage() const {
    return image;
}

void Image::expectPositive(int number, const std::string &varName) {
    expectBetween(number, 1, INT_MAX - 1, varName);
}

void Image::expectNonNegative(int number, const std::string &varName) {
    expectBetween(number, 0, INT_MAX - 1, varName);
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
    uint64_t toSkip = (uint64_t) width * height * ignorance / 100;
    uint64_t skipped = 0;
    int first = (step > 0 ? 0 : maxColorIntensity);
    int last = (step > 0 ? maxColorIntensity + 1 : -1);
    for (int i = first; i != last; i += step) {
        if (skipped + frequency[channel + i] > toSkip) {
            return i;
        } else {
            skipped += frequency[channel + i];
        }
    }
    std::cout << toSkip << ' ' << skipped << std::endl;
    assert(false);
    return last;
}

void Image::setOmpParameters(int numThreads) {
#ifdef _OPENMP
    expectNonNegative(numThreads, "Number of threads");
    if (numThreads > 0) {
        omp_set_num_threads(numThreads);
    }
#endif
}
