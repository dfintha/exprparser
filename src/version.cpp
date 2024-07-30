#include "version.h"

#include <iostream>

std::ostream& operator<<(std::ostream& stream, const expr::version_t& version) {
    return stream << 'v' << version.major
                  << '.' << version.minor
                  << '.' << version.revision;
}