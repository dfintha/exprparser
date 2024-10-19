#include "location.h"

#include <iostream>         // std::ostream

std::ostream& operator<<(
    std::ostream& stream,
    const expr::location_t& location
) {
    const std::size_t offset = (location.begin == location.end) ? 0 : 1;
    return stream << location.begin << '-' << (location.end - offset);
}
