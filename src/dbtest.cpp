#include "dbtest.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <chrono>


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

    if(setting_.numColumnFamily > 1 && setting_.clearOnStart) {
        createColomnFamily(cfopt);
    }

    columnFamilies.emplace_back(rocksdb::ColumnFamilyDescriptor(
        rocksdb::kDefaultColumnFamilyName, cfopt));
    for (int cfNum = 1; cfNum < setting_.numColumnFamily; cfNum++)
    {
        columnFamilies.emplace_back(rocksdb::ColumnFamilyDescriptor(
            getCfName(cfNum), cfopt));
    }

    rocksdb::DBOptions dbOption;
    dbOption.create_if_missing = true;
    auto s = rocksdb::DB::Open(dbOption, setting_.dbPath,
                                columnFamilies, &handles_, &db_);
    if (!s.ok())
    {
        std::cerr << s.ToString() << std::endl;
        exit(1);
    }
}

void DBTest::cleanUp()
{
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

void DBTest::run()
{
    std::string value;
    auto lh = std::make_shared<LocationHandler>(setting_);
    rocksdb::Status s;

    std::cout << "Operation " << setting_.operation << " start." << std::endl;
    auto start = std::chrono::system_clock::now();
    switch (setting_.operation)
    {
    case Operation::WRITE:
        for(int i = 0; i < setting_.numKeyGroup * setting_.numEntryPerKeyGroup; i++)
        {
            auto location = lh->getNextLocation();
            generateValue(value);
            s = db_->Put(rocksdb::WriteOptions(), handles_.at(location->cfNum), location->key, value);
            if (!s.ok())
            {
                std::cerr << s.ToString() << std::endl;
                exit(1);
            }
        }
        break;
    case Operation::READ:
    {
        rocksdb::ReadOptions read_options;
        std::vector<rocksdb::Iterator*> iters;
        db_->NewIterators(read_options, handles_, &iters);
        int readCount = 0;
        for(auto iter : iters) {
            for(iter->Seek(nullptr); iter->Valid(); iter->Next()) {
                volatile auto val = iter->value();
                readCount++;
            }
            std::cout << readCount << " entries were read." << std::endl;
            if (!iter->status().ok()) {
                  std::cerr << "READ failed. " << iter->status().ToString() << std::endl;
            }
        }
        for(auto iter : iters) {
            delete iter;
        }
        break;
    }
    case Operation::PREFIX_SEEK:
    {
        rocksdb::ReadOptions read_options;
        read_options.total_order_seek = false;
        read_options.auto_prefix_mode = false;
        std::vector<rocksdb::Iterator*> iters;
        db_->NewIterators(read_options, handles_, &iters);
        /* The reason why the following loop is executed
           for `setting_.numEntryPerKeyGroup` times is not so meaningful,
           but just to do the same amount of operations
           with the `Operation::READ` case.
        */
        for(int i = 0; i < setting_.numEntryPerKeyGroup; i++)
        {
            rocksdb::Iterator* iter;
            for (int kg = 0; kg < setting_.numKeyGroup; kg++)
            {
                iter = iters.at(kg % setting_.numColumnFamily);
                iter->Seek(lh->getKeyPrefix(kg));
            }
            if(!iter->Valid()) {
                std::cerr << "The iterator is not valid." << std::endl;
                exit(1);
            }
        }
        for(auto iter : iters) {
            delete iter;
        }
        break;
    }
    case Operation::DELETE:
    {
        rocksdb::Slice begin(LocationHandler::MIN_KEY);
        rocksdb::Slice end(LocationHandler::MAX_KEY);
        std::vector<rocksdb::Iterator*> iters;
        db_->NewIterators(rocksdb::ReadOptions(), handles_, &iters);
        int deleteCount = 0;
        for(auto iter : iters) {
            for(iter->Seek(nullptr); iter->Valid(); iter->Next()) {
                db_->Delete(rocksdb::WriteOptions(), iter->key());
                deleteCount++;
            }
            std::cout << deleteCount << " entries were deleted." << std::endl;
            if (!iter->status().ok()) {
                  std::cerr << "DELETE failed. " << iter->status().ToString() << std::endl;
            }
        }
        for(auto iter : iters) {
            delete iter;
        }
        break;
    }
    case Operation::DELETE_RANGE:
    {
        rocksdb::Slice begin(LocationHandler::MIN_KEY);
        rocksdb::Slice end(LocationHandler::MAX_KEY);
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
        s = db_->CompactRange(cropt, nullptr, nullptr);
        break;
    }
    default:
        std::cerr << "Unexpected operation" << setting_.operation << std::endl;
        exit(1);
    }
    auto end = std::chrono::system_clock::now();
    std::cout << "Operation " << setting_.operation << " finished." << std::endl;
    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "elapsed time: " << elapsed/1000.0 << "[s]" << std::endl;
}

std::string DBTest::getCfName(int cfNum)
{
    return "cf" + std::to_string(cfNum);
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

void DBTest::createColomnFamily(const rocksdb::ColumnFamilyOptions& cfopt)
{
    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::DB* db;
    rocksdb::Status s = rocksdb::DB::Open(options, setting_.dbPath, &db);
    if (!s.ok())
    {
        std::cerr << s.ToString() << std::endl;
        exit(1);
    }

    handles_.resize(setting_.numColumnFamily);
    for (int cfNum = 1; cfNum < setting_.numColumnFamily; cfNum++)
    {
        rocksdb::ColumnFamilyHandle* cf;
        rocksdb::Status s = db->CreateColumnFamily(cfopt, getCfName(cfNum), &cf);
        if (!s.ok())
        {
            std::cerr << s.ToString() << std::endl;
            exit(1);
        }

        s = db->DestroyColumnFamilyHandle(cf);
        if (!s.ok())
        {
            std::cerr << s.ToString() << std::endl;
            exit(1);
        }
        std::cout << "created column family " << getCfName(cfNum) << std::endl;
    }
    delete db;
}
