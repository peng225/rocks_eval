#include "setting.h"
#include "parsearg.h"
#include "dbtest.h"
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char **argv)
{
    TestSetting setting;
    parseArgs(argc, argv, setting);

    if(setting.clearOnStart) {
        fs::remove_all(setting.dbPath);
    }

    auto dbtest = std::make_shared<DBTest>(setting);
    dbtest->setUp();
    dbtest->runTest();
    dbtest->cleanUp();

    return 0;
}
