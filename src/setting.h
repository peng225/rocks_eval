#ifndef SETTING_H
#define SETTING_H

#include <string>

enum class IOPattern
{
    RANDOM = 0,
    SEQUENTIAL
};

enum class Operation
{
    WRITE = 0,
    READ,
    PREFIX_SEEK,
    DELETE_RANGE,
    COMPACTION
};

std::ostream &operator<<(std::ostream &os, const Operation &ope);

typedef struct TestSetting
{
    // RocksDB options
    int numColumnFamily = 1;
    bool smallSet = false;

    // Run settings
    std::string dbPath = "test";
    bool clearOnStart = false;
    bool allowNotFound = false;
    Operation operation = Operation::WRITE;

    // Table settings
    int numKeyGroup = 1;
    int numEntryPerKeyGroup = 1000;
    int valueLength = 32;
    IOPattern writePattern = IOPattern::RANDOM;
    //TODO:  How about ReadPattern?
} TestSetting;

#endif // SETTING_H
