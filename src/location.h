#ifndef LOCATION_H
#define LOCATION_H

typedef struct Location
{
    int cfNum;
    std::string key;
    Location(int cfNum, const std::string key) : cfNum(cfNum), key(key)
    {
    }
} Location;

#endif // LOCATION_H