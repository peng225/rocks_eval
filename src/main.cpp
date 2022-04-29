#include "setting.h"
#include "parsearg.h"
#include <rocksdb/db.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <filesystem>
#include <vector>
#include <unordered_map>

namespace fs = std::filesystem;

std::string getCfName(int cfNum) {
    return "cf" + std::to_string(cfNum);
}

typedef struct Location
{
    int cfNum;
    std::string key;
    Location(int cfNum, const std::string key) : cfNum(cfNum), key(key)
    {
    }
} Location;

void generateLocations(std::vector<Location> &locations, const TestSetting &setting)
{
    for (int kg = 0; kg < setting.numKeyGroup; kg++)
    {
        std::stringstream key;
        key << "key" << std::setfill('0') << std::setw(5) << kg << "/";
        for (int i = 0; i < setting.numEntryPerKeyGroup; i++)
        {
            key << std::setfill('0') << std::setw(12) << i;
            locations.emplace_back(Location(i%setting.numColumnFamily, key.str()));
        }
    }
}

void cleanUp(rocksdb::DB *db, std::vector<rocksdb::ColumnFamilyHandle*> &handles)
{
    for (int i = 0; i < static_cast<int>(handles.size()) - 1; i++)
    {
        auto s = db->DropColumnFamily(handles[i + 1]);
        if (!s.ok())
        {
            std::cerr << s.ToString() << std::endl;
            exit(1);
        }
    }
    for (auto handle : handles)
    {
        auto s = db->DestroyColumnFamilyHandle(handle);
        if (!s.ok())
        {
            std::cerr << s.ToString() << std::endl;
            exit(1);
        }
    }
    delete db;
}

int main(int argc, char **argv)
{
    TestSetting setting;
    parseArgs(argc, argv, setting);

    if(setting.clearOnStart) {
        fs::remove_all(setting.dbPath);
    }

    std::vector<rocksdb::ColumnFamilyDescriptor> columnFamilies;
    columnFamilies.emplace_back(rocksdb::ColumnFamilyDescriptor(
        rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions()));
    for(int cfNum = 0; cfNum < setting.numColumnFamily-1; cfNum++)
    {
        columnFamilies.emplace_back(rocksdb::ColumnFamilyDescriptor(
            getCfName(cfNum), rocksdb::ColumnFamilyOptions()));
    }
    std::vector<rocksdb::ColumnFamilyHandle *> handles;

    rocksdb::DB *db;
    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status s = rocksdb::DB::Open(options, setting.dbPath, columnFamilies, &handles, &db);
    if (!s.ok())
    {
        std::cerr << s.ToString() << std::endl;
        exit(1);
    }

    std::string value;
    std::vector<Location> locations;
    generateLocations(locations, setting);
    // rocksdb::WriteOptions wop();
    // rocksdb::ReadOptions rop();

    std::cout << "Operation " << setting.operation << " start." << std::endl;
    switch(setting.operation){
    case Operation::WRITE:
        for (const auto &location : locations)
        {
            s = db->Put(rocksdb::WriteOptions(), handles.at(location.cfNum), location.key, std::string("hoge")); // TODO: set value
            if (!s.ok())
            {
                std::cerr << s.ToString() << std::endl;
                return 1;
            }
        }
        break;
    case Operation::READ:
        for (const auto &location : locations)
        {
            s = db->Get(rocksdb::ReadOptions(), handles.at(location.cfNum), location.key, &value);
            if (!s.ok())
            {
                std::cerr << s.ToString() << std::endl;
                return 1;
            }
        }
        break;
    case Operation::READ_RANGE:
        break;
    case Operation::DELETE:
        for (const auto &location : locations)
        {
            if (s.ok())
            {
                s = db->Delete(rocksdb::WriteOptions(), handles.at(location.cfNum), location.key);
            }
        }
        break;
    case Operation::DELETE_RANGE:
        break;
    default:
        std::cerr << "Unexpected operation" << setting.operation << std::endl;
        exit(1);
    }
    std::cout << "Operation " << setting.operation << " finished." << std::endl;

    cleanUp(db, handles);
}
