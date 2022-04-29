#include "setting.h"
#include <sstream>

std::ostream &operator<<(std::ostream &os, const Operation &ope)
{
    switch (ope)
    {
    case Operation::WRITE:
        os << "WRITE";
        break;
    case Operation::READ:
        os << "READ";
        break;
    case Operation::READ_RANGE:
        os << "READ_RANGE";
        break;
    case Operation::DELETE:
        os << "DELETE";
        break;
    case Operation::DELETE_RANGE:
        os << "DELETE_RANGE";
        break;
    default:
        os << "UNKNOWN (" << static_cast<int>(ope) << ")";
        break;
    }
    return os;
}
