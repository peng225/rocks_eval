#include "rocksdb/db.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

void generateDataset(std::vector<std::pair<std::string, std::string>> &dataset, int count)
{
    for (int i = 0; i < count; i++)
    {
        std::stringstream ss;
        ss << i;
        dataset.emplace_back(std::pair<std::string, std::string>(ss.str(), ss.str()));
    }
}

int main(int argc, char **argv)
{
    boost::program_options::options_description opt("Options");
    opt.add_options()("help,h", "Show help")("dbdir,d", boost::program_options::value<std::string>(), "Path to the directory where RocksDB files are put")("count,c", boost::program_options::value<int>()->default_value(1), "The number of records to be written.");
    boost::program_options::variables_map vm;
    try
    {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, opt), vm);
    }
    catch (const boost::program_options::error_with_option_name &e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }
    boost::program_options::notify(vm);

    if (vm.count("help"))
    {
        std::cout << opt << std::endl;
        return 0;
    }

    std::string dbdir;
    if (vm.count("dbdir"))
    {
        try
        {
            dbdir = vm["dbdir"].as<std::string>();
            std::cout << dbdir << std::endl;
        }
        catch (const boost::bad_any_cast &e)
        {
            std::cout << e.what() << std::endl;
            return 1;
        }
    }

    int count;
    if (vm.count("count"))
    {
        try
        {
            count = vm["count"].as<int>();
            std::cout << count << std::endl;
        }
        catch (const boost::bad_any_cast &e)
        {
            std::cout << e.what() << std::endl;
            return 1;
        }
    }

    rocksdb::DB *db;
    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status s = rocksdb::DB::Open(options, dbdir, &db);
    if (!s.ok())
    {
        std::cerr << s.ToString() << std::endl;
        return 1;
    }

    std::string value;
    std::vector<std::pair<std::string, std::string>> dataset;
    generateDataset(dataset, count);

    std::cout << "Write phase start" << std::endl;
    for (const auto &data : dataset)
    {
        s = db->Put(rocksdb::WriteOptions(), data.first, data.second);
        if (!s.ok())
        {
            std::cerr << s.ToString() << std::endl;
            return 1;
        }
    }
    std::cout << "Write phase end" << std::endl;
    std::cout << "Read phase start" << std::endl;
    for (const auto &data : dataset)
    {
        s = db->Get(rocksdb::ReadOptions(), data.first, &value);
        if (!s.ok())
        {
            std::cerr << s.ToString() << std::endl;
            return 1;
        }
    }
    std::cout << "Read phase end" << std::endl;
    for (const auto &data : dataset)
    {
        if (s.ok())
        {
            s = db->Delete(rocksdb::WriteOptions(), data.first);
        }
    }

    delete db;
}
