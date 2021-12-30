//
// Created by Danila Belous on 30.12.2021 at 13:25.
//

#include <iostream>
#include "Time.h"

Time::AllTime Time::GetAllTime() {
    return { GetWallTime(), GetCpuTime() };
}

void Time::SaveCurrent(const std::string &label) {
    if (warnings && data_.count(label)) {
        std::cout << "Warning: overwriting old timestamp for " << label << std::endl;
    }
    data_.insert_or_assign(label, GetAllTime());
}

Time::AllTime Time::GetDelta(const std::string &label) {
    if (!data_.count(label)) {
        throw std::invalid_argument("Error: there's no timestamp for label " + label);
    }
    AllTime now = GetAllTime();
    AllTime old = data_.at(label);
    return { now.wall - old.wall, now.cpu - old.cpu };
}

void Time::PrintDelta(const std::string &label) {
    AllTime delta = GetDelta(label);
    std::cout << label << " time | wall: " << delta.wall << " s" << std::endl;
    std::cout << label << " time | CPU : " << delta.cpu << " s" << std::endl;
}

void Time::ShowWarnings(const bool &flag) {
    warnings = flag;
}
