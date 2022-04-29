#include "parsearg.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <cstdlib>

void parseArgs(int argc, char **argv, TestSetting &setting)
{
    boost::program_options::options_description opt("Options");
    opt.add_options()("help,h", "Show help")
    ("dbpath,d", boost::program_options::value<std::string>(), "Path to the directory where RocksDB files are put")
    ("epg", boost::program_options::value<int>(), "The number of entries to be written per key group.")
    ("operation,o", boost::program_options::value<std::string>(), "The operation to be executed.")
    ("numcf", boost::program_options::value<int>(), "The number of column families.")
    ("cos", "Clear on start.");
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
            {"WRITE", Operation::WRITE},
            {"READ", Operation::READ},
            {"READ_RANGE", Operation::READ_RANGE},
            {"DELETE", Operation::DELETE},
            {"DELETE_RANGE", Operation::DELETE_RANGE}};
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

    if (vm.count("cos"))
    {
        std::cout << "cos" << std::endl;
        setting.clearOnStart = true;
    }
}
