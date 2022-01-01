//
// Created by Danila Belous on 26.12.2021 at 03:34.
//

#include <iostream>
#include <cassert>
#include <omp.h>
#include "Image.h"
#include "Time.h"

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
    image = source;

    frequency.resize(numberOfChannels * (maxColorIntensity + 1));
    UpdateFrequency();
}

// Applies min-max contrast stretching (percentile contrast stretching if ignorance is not zero)
void Image::EnhanceGlobalContrast(int ignorance) {
    expectBetween(ignorance, 0, 50, "ignorance percentage");

#ifndef NDEBUG
    Time *timestamps = new Time();
    timestamps->SaveCurrent("Getting min-max params");
#endif

    std::vector<std::pair<uint8_t, uint8_t>> minmax(MAX_NUM_CHANNELS);
#pragma omp parallel for shared(minmax, ignorance) default(none)
    for (int i = 0; i < nChannels; ++i) {
        minmax[i] = GetMinMaxIntensityLevel(ignorance, i);
    }

#ifndef NDEBUG
    for (int i = 0; i < nChannels; ++i) {
        std::cout << "Min and max pixel values after ignoring: Channel " << i << ": " << (int) minmax[i].first << ' '
                  << (int) minmax[i].second << std::endl;
    }

    timestamps->PrintDelta("Getting min-max params");
    timestamps->SaveCurrent("Enhancing");
#endif

    std::vector<int> cntThreadsWork;
    uint32_t numProcessed = 0;

#pragma omp parallel shared(image, ignorance, cntThreadsWork, minmax, numProcessed) default(none)
    {

#if defined(_OPENMP) && !defined(NDEBUG)
#pragma omp single
        {
            cntThreadsWork.resize(omp_get_num_threads());
        }
#endif

#pragma omp for schedule(static)
        for (int j = 0; j < size; j += nChannels) {
#ifndef NDEBUG
#ifdef _OPENMP
//#pragma omp single
//            {
//                std::cout << "cur pixel: " << j + i << " , cur thread: " << omp_get_thread_num() << std::endl;
//            }
            cntThreadsWork[omp_get_thread_num()] += nChannels;
#else
            numProcessed++;
#endif
#endif
            image[j] = std::max(image[j], minmax[0].first);
            image[j] = std::min(image[j], minmax[0].second);
            image[j] = minmax[0].first == minmax[0].second ? 0 :
                       ((image[j] - minmax[0].first) * maxColorIntensity) /
                       (minmax[0].second - minmax[0].first);

            if (nChannels > 1) {
                image[j + 1] = std::max(image[j + 1], minmax[1].first);
                image[j + 1] = std::min(image[j + 1], minmax[1].second);
                image[j + 1] = minmax[1].first == minmax[1].second ? 0 :
                               ((image[j + 1] - minmax[1].first) * maxColorIntensity) /
                               (minmax[1].second - minmax[1].first);

                if (nChannels > 2) {
                    image[j + 2] = std::max(image[j + 2], minmax[2].first);
                    image[j + 2] = std::min(image[j + 2], minmax[2].second);
                    image[j + 2] = minmax[2].first == minmax[2].second ? 0 :
                                   ((image[j + 2] - minmax[2].first) * maxColorIntensity) /
                                   (minmax[2].second - minmax[2].first);
                }
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

#ifndef NDEBUG
    timestamps->PrintDelta("Enhancing");
    timestamps->SaveCurrent("Updating frequency");
#endif
//    UpdateFrequency();
#ifndef NDEBUG
    timestamps->PrintDelta("Updating frequency");
#endif
}

void Image::UpdateFrequency() {
#pragma omp parallel for shared(image, frequency) default(none)
    for (int j = 0; j < size; j += nChannels) {
        frequency[image[j]]++;
        if (nChannels > 1) {
            frequency[image[j + 1] + 1]++;
            if (nChannels > 2) {
                frequency[image[j + 2] + 2]++;
            }
        }
    }
}

void Image::PrintPixelIntensityFrequency() {
    std::cout << "Image color frequency: " << std::endl;
    for (int i = 0; i < nChannels; ++i) {
        std::cout << "Channel " << i + 1 << std::endl;
        for (int j = i; j <= maxColorIntensity * nChannels; j += nChannels) {
            std::cout << j / nChannels << ": " << frequency[j] << std::endl;
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

std::pair<uint8_t, uint8_t> Image::GetMinMaxIntensityLevel(int ignorance, int channel) {
    assert(0 <= ignorance && ignorance <= 50);
    assert(0 <= channel && channel < nChannels);
    //todo упомянуть в отчёте про int'ы (43 8к картинки влезает)
    uint32_t toSkip = (uint64_t) width * height * ignorance / 100;
    uint32_t skipped = 0;
    uint8_t first;
    for (int i = 0; i < maxColorIntensity; ++i) {
        skipped += frequency[channel + i];
        if (skipped > toSkip) {
            first = i;
            break;
        }
    }
    skipped = 0;
    uint8_t last;
    for (int i = maxColorIntensity - 1; i >= 0; --i) {
        skipped += frequency[channel + i];
        if (skipped > toSkip) {
            last = i;
            break;
        }
    }
    return { first, last };
}

void Image::setOmpParameters(int numThreads) {
#ifdef _OPENMP
    expectNonNegative(numThreads, "Number of threads");
    if (numThreads > 0) {
        omp_set_num_threads(numThreads);
    }
#endif
}
