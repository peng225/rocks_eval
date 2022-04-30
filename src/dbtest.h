#ifndef DBTEST_H
#define DBTEST_H

#include "setting.h"
#include "location.h"
#include <rocksdb/db.h>
#include <string>
#include <random>

class DBTest {
public:
    explicit DBTest(const TestSetting &setting);
    void setUp();
    void cleanUp();
    void runTest();
private:
    rocksdb::DB *db_;
    std::vector<rocksdb::ColumnFamilyHandle *> handles_;
    TestSetting setting_;
    std::mt19937 mt_;

    std::string getCfName(int cfNum);
    void generateLocations(std::vector<Location> &locations);
    void generateValue(std::string &value);
};

#endif // DBTEST_H
