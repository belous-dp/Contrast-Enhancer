//
// Created by Danila Belous on 26.12.2021 at 03:34.
//

#include <iostream>
#include <cassert>
#include <omp.h>
#include <cmath>
#include "Image.h"
#include "Time.h"

Image::Image(std::vector<uint8_t> &source, int numberOfChannels, int width, int height, int maxColorIntensity) {

    expectBetween(numberOfChannels, MIN_NUM_CHANNELS, MAX_NUM_CHANNELS, "number of channels");
    this->nChannels = numberOfChannels;

    expectPositive(width, "width");
    this->width = width;

    expectPositive(height, "height");
    this->height = height;

    expectBetween(maxColorIntensity, 0, MAX_COLOR_VALUE, "maximum color intensity");
    this->maxColorIntensity = maxColorIntensity;

    this->size = width * height * numberOfChannels;
    if (this->size != (uint64_t) width * height * numberOfChannels) {
        throw std::invalid_argument("Image is too big");
    }

    if (source.size() != size) {
        throw std::invalid_argument("Image expected size does not match real size");
    }
    image = source;

    frequency.resize(numberOfChannels * (maxColorIntensity + 1));
    UpdateFrequency();
}

// Applies min-max contrast stretching (percentile contrast stretching if ignore is not zero)
void Image::EnhanceGlobalContrast(double ignore) {
    expectBetween((int) (ignore * 100), 0, 49, "ignore percentage");

    Time *timestamps = new Time(3);
#ifndef NDEBUG
    timestamps->SaveCurrent("Getting min-max params");
#endif

    uint8_t min = MAX_COLOR_VALUE, max = 0;
    GetMinMaxIntensityLevel(ignore, min, max);

#ifndef NDEBUG
    std::cout << "Global min and max pixel values after ignoring: " << (int) min << ' ' << (int) max << std::endl;

    timestamps->PrintDelta("Getting min-max params");
#endif
    timestamps->SaveCurrent("Enhancing");

    std::vector<int> cntThreadsWork;
    uint32_t numProcessed = 0;


#pragma omp parallel shared(ignore, cntThreadsWork, min, max, numProcessed) default(none)
    {

#ifdef _OPENMP
#pragma omp single
        {
            cntThreadsWork.resize(omp_get_num_threads());
        }
#else
        cntThreadsWork.resize(1);
#endif

#pragma omp for schedule(static)
        for (int j = 0; j < size; j += nChannels) {
            for (int i = 0; i < nChannels; ++i) {
#ifndef NDEBUG
#ifdef _OPENMP
//#pragma omp single
//                {
//                    std::cout << "cur pixel: " << j + i << " , cur thread: " << omp_get_thread_num() << std::endl;
//                }
                cntThreadsWork[omp_get_thread_num()]++;
#else
                numProcessed++;
#endif
#endif
                image[j + i] = std::max(image[j + i], min);
                image[j + i] = std::min(image[j + i], max);
                image[j + i] = min == max ? 0 : ((image[j + i] - min) * maxColorIntensity) / (max - min);
            }
        }
    }

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
    // uncomment if you want to print time like in logs
    std::cout << "Time (" << cntThreadsWork.size() << " thread(s)): "
              << timestamps->GetDelta("Enhancing").wall * 1000. << " ms\n";
#ifndef NDEBUG
    timestamps->PrintDelta("Enhancing");
    timestamps->SaveCurrent("Updating frequency");
#endif
    UpdateFrequency();
#ifndef NDEBUG
    timestamps->PrintDelta("Updating frequency");
#endif
}

void Image::UpdateFrequency() {
#pragma omp parallel for default(none)
    for (int j = 0; j < size; j += nChannels) {
        for (int i = 0; i < nChannels; ++i) {
            frequency[image[j + i] * nChannels + i]++;
        }
    }
}

void Image::PrintPixelIntensityFrequency() {
    std::cout << "Image color frequency: " << std::endl;
    for (int i = 0; i < nChannels; ++i) {
        std::cout << "Channel " << i + 1 << std::endl;
        for (int j = i, k = 0; j < frequency.size(); j += nChannels, ++k) {
            std::cout << k << ": " << frequency[j] << std::endl;
        }
    }
    std::cout << std::endl;
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

void Image::GetMinMaxIntensityLevel(double ignore, uint8_t &min, uint8_t &max) {
    assert(0 <= ignore && ignore <= 50);
    auto toSkip = (uint32_t) round(width * height * ignore);
    std::vector<uint8_t> cmin(MAX_NUM_CHANNELS, maxColorIntensity), cmax(MAX_NUM_CHANNELS);
    std::vector<uint32_t> skipped(MAX_NUM_CHANNELS);
    uint8_t id = 0;
    for (int i = 0; i < frequency.size(); i += nChannels, ++id) {
        for (int j = 0; j < nChannels; ++j) {
            skipped[j] += frequency[i + j];
            if (skipped[j] > toSkip) {
                cmin[j] = std::min(cmin[j], id);
            }
        }
    }

    skipped.assign(MAX_NUM_CHANNELS, 0);
    id = maxColorIntensity;
    for (int i = (int) frequency.size() - nChannels; i >= 0; i -= nChannels, --id) {
        for (int j = 0; j < nChannels; ++j) {
            skipped[j] += frequency[i + j];
            if (skipped[j] > toSkip) {
                cmax[j] = std::max(cmax[j], id);
            }
        }
    }

    for (int i = 0; i < nChannels; ++i) {
        min = std::min(min, cmin[i]);
        max = std::max(max, cmax[i]);
#ifndef NDEBUG
        std::cout << "Min and max color values for " << i + 1 << "th channel: min = " << (int) cmin[i] << ", max = "
                  << (int) cmax[i] << std::endl;
#endif
    }
    assert(min <= max);
}

void Image::setOmpParameters(int numThreads) {
#ifdef _OPENMP
    expectNonNegative(numThreads, "Number of threads");
    if (numThreads > 0) {
        omp_set_num_threads(numThreads);
    }
#endif
}
