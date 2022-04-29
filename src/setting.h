#ifndef SETTING_H
#define SETTING_H

#include <string>

enum class WritePattern
{
    RANDOM = 0,
    SEQUENTIAL
};

enum class Operation
{
    WRITE = 0,
    READ,
    READ_RANGE,
    DELETE,
    DELETE_RANGE
};

std::ostream &operator<<(std::ostream &os, const Operation &ope);

typedef struct TestSetting
{
    // RocksDB options
    int numColumnFamily = 1;

    // Run settings
    std::string dbPath = "test";
    bool clearOnStart = false;
    Operation operation = Operation::WRITE;

    // Table settings
    int numKeyGroup = 1;
    int numEntryPerKeyGroup = 1000;
    int valueLength = 128;
    WritePattern writePattern = WritePattern::RANDOM;
} TestSetting;

#endif // SETTING_H
