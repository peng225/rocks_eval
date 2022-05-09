#ifndef LOCATION_HANDLER_H
#define LOCATION_HANDLER_H

#include "location.h"
#include "setting.h"
#include <memory>

class LocationHandler {
public:
    LocationHandler(const TestSetting &setting);
    static const std::string MIN_KEY;
    static const std::string MAX_KEY;
    std::shared_ptr<Location> getNextLocation();
    std::string getKeyPrefix(int kg);
private:
    TestSetting setting_;
    int currentKg_;
    int numEntryInKg_;
};
#endif // LOCATION_HANDLER_H
