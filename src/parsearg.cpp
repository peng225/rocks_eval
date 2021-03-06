#include "parsearg.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <cstdlib>
#include <unordered_map>

void parseArgs(int argc, char **argv, TestSetting &setting)
{
    boost::program_options::options_description opt("Options");
    opt.add_options()("help,h", "Show help")
    ("dbpath,d", boost::program_options::value<std::string>(), "Path to the directory where RocksDB files are put")
    ("epg", boost::program_options::value<int>(), "The number of entries to be written per key group.")
    ("operation,o", boost::program_options::value<std::string>(), "The operation to be executed.")
    ("numcf", boost::program_options::value<int>(), "The number of column families.")
    ("numkg", boost::program_options::value<int>(), "The number of key groups.")
    ("clear_on_start,c", "Clear on start.")
    ("allow_not_found", "Allow 'not found' error on read.")
    ("verbose,v", "Verbose.")
    ("small_set", "Use small rocksdb settings.");
    // TODO: set writePattern
    boost::program_options::variables_map vm;
    try
    {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, opt), vm);
    }
    catch (const boost::program_options::error_with_option_name &e)
    {
        std::cout << e.what() << std::endl;
        exit(1);
    }
    boost::program_options::notify(vm);

    if (vm.count("help"))
    {
        std::cout << opt << std::endl;
        exit(1);
    }

    if (vm.count("dbpath"))
    {
        try
        {
            setting.dbPath = vm["dbpath"].as<std::string>();
        }
        catch (const boost::bad_any_cast &e)
        {
            std::cout << e.what() << std::endl;
            exit(1);
        }
    }

    if (vm.count("epg"))
    {
        try
        {
            setting.numEntryPerKeyGroup = vm["epg"].as<int>();
        }
        catch (const boost::bad_any_cast &e)
        {
            std::cout << e.what() << std::endl;
            exit(1);
        }
    }

    if (vm.count("operation"))
    {
        std::unordered_map<std::string, Operation> strToOpe = {
            {"write", Operation::WRITE},
            {"read", Operation::READ},
            {"prefix_seek", Operation::PREFIX_SEEK},
            {"delete", Operation::DELETE},
            {"delete_range", Operation::DELETE_RANGE},
            {"compact", Operation::COMPACTION}};
        std::string ope;
        try
        {
            ope = vm["operation"].as<std::string>();
        }
        catch (const boost::bad_any_cast &e)
        {
            std::cout << e.what() << std::endl;
            exit(1);
        }
        if (strToOpe.find(ope) == std::end(strToOpe))
        {
            std::cerr << "invalid operation: " << ope << std::endl;
            exit(1);
        }
        setting.operation = strToOpe[ope];
    } else {
            std::cerr << "-o/--operation option is required." << std::endl;
            exit(1);
    }

    if (vm.count("numcf"))
    {
        try
        {
            setting.numColumnFamily = vm["numcf"].as<int>();
        }
        catch (const boost::bad_any_cast &e)
        {
            std::cout << e.what() << std::endl;
            exit(1);
        }
    }

    if (vm.count("numkg"))
    {
        try
        {
            setting.numKeyGroup = vm["numkg"].as<int>();
        }
        catch (const boost::bad_any_cast &e)
        {
            std::cout << e.what() << std::endl;
            exit(1);
        }
    }

    if (vm.count("clear_on_start"))
    {
        setting.clearOnStart = true;
    }

    if (vm.count("allow_not_found"))
    {
        setting.allowNotFound = true;
    }

    if (vm.count("small_set"))
    {
        setting.smallSet = true;
    }

    if (vm.count("verbose"))
    {
        setting.verbose = true;
    }
}
