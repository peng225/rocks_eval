#ifndef LOCATION_HANDLER_H
#define LOCATION_HANDLER_H

#include "location.h"
#include "setting.h"
#include <memory>

class LocationHandler {
public:
    LocationHandler(const TestSetting &setting);
    std::shared_ptr<Location> getNextLocation();
private:
    TestSetting setting_;
    int currentKg_;
    int numEntryInKg_;
};
#endif // LOCATION_HANDLER_H
