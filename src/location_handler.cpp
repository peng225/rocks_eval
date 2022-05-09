#include "location_handler.h"
#include <iomanip>

LocationHandler::LocationHandler(const TestSetting &setting) :
    setting_(setting),
    currentKg_(0),
    numEntryInKg_(0)
{
}

std::shared_ptr<Location> LocationHandler::getNextLocation()
{
    std::stringstream key;
    key << "key" << std::setfill('0') << std::setw(5) << currentKg_ << "/"
        << std::setw(12) << numEntryInKg_;
    int cfNum = currentKg_ % setting_.numColumnFamily;

    numEntryInKg_++;
    if(numEntryInKg_ == setting_.numEntryPerKeyGroup) {
        numEntryInKg_ = 0;
        currentKg_ = (currentKg_ + 1) % setting_.numKeyGroup;
    }

    return std::make_shared<Location>(cfNum, key.str());
}
