#include "dbtest.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <chrono>

const std::string MIN_KEY = "key";
const std::string MAX_KEY = "key:";

DBTest::DBTest(const TestSetting &setting) :
    setting_(setting),
    // Use 0 as a fixed seed to generate the same result
    // every time this program is executed
    mt_(0)
{
}

void DBTest::setUp()
{
    std::vector<rocksdb::ColumnFamilyDescriptor> columnFamilies;
    rocksdb::ColumnFamilyOptions cfopt;
    if(setting_.smallSet) {
        // For cf options, see https://github.com/facebook/rocksdb/wiki/Leveled-Compaction#levels-target-size
        cfopt.write_buffer_size = 10 * 1024;
        cfopt.max_bytes_for_level_base = 20 * 1024;
        cfopt.max_bytes_for_level_multiplier = 2;
        cfopt.num_levels = 4;
    }
    columnFamilies.emplace_back(rocksdb::ColumnFamilyDescriptor(
        rocksdb::kDefaultColumnFamilyName, cfopt));
    for (int cfNum = 0; cfNum < setting_.numColumnFamily - 1; cfNum++)
    {
        columnFamilies.emplace_back(rocksdb::ColumnFamilyDescriptor(
            getCfName(cfNum), cfopt));
    }

    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status s = rocksdb::DB::Open(options, setting_.dbPath, columnFamilies, &handles_, &db_);
    if (!s.ok())
    {
        std::cerr << s.ToString() << std::endl;
        exit(1);
    }
}

void DBTest::cleanUp()
{
    // i==0 is default column family
    for (int i = 1; i < static_cast<int>(handles_.size()); i++)
    {
        auto s = db_->DropColumnFamily(handles_[i]);
        if (!s.ok())
        {
            std::cerr << s.ToString() << std::endl;
            exit(1);
        }
    }
    for (auto handle : handles_)
    {
        auto s = db_->DestroyColumnFamilyHandle(handle);
        if (!s.ok())
        {
            std::cerr << s.ToString() << std::endl;
            exit(1);
        }
    }
    delete db_;
}

void DBTest::runTest()
{
    std::string value;
    std::vector<Location> locations;
    generateLocations(locations);
    rocksdb::Status s;
    // rocksdb::WriteOptions wop();
    // rocksdb::ReadOptions rop();

    std::cout << "Operation " << setting_.operation << " start." << std::endl;
    auto start = std::chrono::system_clock::now();
    switch (setting_.operation)
    {
    case Operation::WRITE:
        for (const auto &location : locations)
        {
            generateValue(value);
            s = db_->Put(rocksdb::WriteOptions(), handles_.at(location.cfNum), location.key, value);
            if (!s.ok())
            {
                std::cerr << s.ToString() << std::endl;
                exit(1);
            }
        }
        break;
    case Operation::READ:
        for (const auto &location : locations)
        {
            s = db_->Get(rocksdb::ReadOptions(), handles_.at(location.cfNum), location.key, &value);
            if (!s.ok())
            {
                if(!setting_.allowNotFound || !s.IsNotFound()) {
                    std::cerr << s.ToString() << std::endl;
                    exit(1);
                }
            }
        }
        break;
    case Operation::PREFIX_SEEK:
    {
        rocksdb::ReadOptions read_options;
        read_options.total_order_seek = false;
        read_options.auto_prefix_mode = false;
        rocksdb::Iterator *iter = db_->NewIterator(read_options);
        for (int kg = 0; kg < setting_.numKeyGroup; kg++)
        {
            std::stringstream keyPrefix;
            keyPrefix << "key" << std::setfill('0') << std::setw(5) << kg << "/";
            iter->Seek(keyPrefix.str());
        }
        if(!iter->Valid()) {
            std::cerr << "The iterator is not valid." << std::endl;
            exit(1);
        }
        // std::cout << "key = " << iter->key().ToString() << ", value = " << iter->value().ToString() << std::endl;
        delete iter;
        break;
    }
    case Operation::DELETE:
        for (const auto &location : locations)
        {
            s = db_->Delete(rocksdb::WriteOptions(), handles_.at(location.cfNum), location.key);
            if (!s.ok())
            {
                std::cerr << s.ToString() << std::endl;
                exit(1);
            }
        }
        break;
    case Operation::DELETE_RANGE:
    {
        rocksdb::Slice begin(MIN_KEY);
        rocksdb::Slice end(MAX_KEY);
        for(auto &handle : handles_) {
            s = db_->DeleteRange(rocksdb::WriteOptions(), handle, begin, end);
            if (!s.ok())
            {
                std::cerr << s.ToString() << std::endl;
                exit(1);
            }
        }
        break;
    }
    case Operation::COMPACTION:
    {
        rocksdb::CompactRangeOptions cropt;
        rocksdb::Slice begin(MIN_KEY);
        rocksdb::Slice end(MAX_KEY);
        s = db_->CompactRange(cropt, &begin, &end);
        break;
    }
    default:
        std::cerr << "Unexpected operation" << setting_.operation << std::endl;
        exit(1);
    }
    auto end = std::chrono::system_clock::now();
    std::cout << "Operation " << setting_.operation << " finished." << std::endl;
    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "elapsed time: " << elapsed/1000.0 << "[ms]" << std::endl;
}

std::string DBTest::getCfName(int cfNum)
{
    return "cf" + std::to_string(cfNum);
}

// TODO: generating location info in memory is not good when the data size is large.
//       Instead, on-demand locatoin info generator is better.
void DBTest::generateLocations(std::vector<Location> &locations)
{
    for (int kg = 0; kg < setting_.numKeyGroup; kg++)
    {
        std::stringstream keyPrefix;
        keyPrefix << "key" << std::setfill('0') << std::setw(5) << kg << "/";
        for (int i = 0; i < setting_.numEntryPerKeyGroup; i++)
        {
            std::stringstream keyBody;
            keyBody << std::setfill('0') << std::setw(12) << i;
            locations.emplace_back(Location(i % setting_.numColumnFamily, keyPrefix.str() + keyBody.str()));
        }
    }
}

void DBTest::generateValue(std::string &value)
{
    value.clear();
    value.reserve(setting_.valueLength);
    for (int i = 0; i < setting_.valueLength; i++)
    {
        value += static_cast<char>(mt_() % 26 + static_cast<int>('a'));
    }
}
