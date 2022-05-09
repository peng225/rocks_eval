#include "setting.h"
#include <sstream>

std::ostream &operator<<(std::ostream &os, const Operation &ope)
{
    switch (ope)
    {
    case Operation::WRITE:
        os << "write";
        break;
    case Operation::READ:
        os << "read";
        break;
    case Operation::PREFIX_SEEK:
        os << "prefix_seek";
        break;
    case Operation::DELETE_RANGE:
        os << "delee_range";
        break;
    case Operation::COMPACTION:
        os << "compaction";
        break;
    default:
        os << "UNKNOWN (" << static_cast<int>(ope) << ")";
        break;
    }
    return os;
}
