#if !defined(EXPRPARSER_QUANTITY_HEADER)
#define EXPRPARSER_QUANTITY_HEADER

#include "result.h"

namespace expr {
    struct measurement_unit {
        int length_dimension;
        int angle_dimension;

        bool is_scalar() const;
        bool is_length() const;
        bool is_area() const;
        bool is_volume() const;
        bool is_angle() const;
        bool is_mixed() const;

        bool operator==(const measurement_unit&) const = default;
    };

    struct quantity {
        measurement_unit unit;
        double value;

        bool is_scalar() const {
            return unit.is_scalar();
        }

        bool is_length() const {
            return unit.is_length();
        }

        bool is_area() const {
            return unit.is_area();
        }

        bool is_volume() const {
            return unit.is_volume();
        }

        bool is_angle() const {
            return unit.is_angle();
        }

        bool is_mixed() const {
            return unit.is_mixed();
        }
    };

    using arithmetic_result = result<quantity, error>;

    inline quantity make_scalar(double value) {
        return quantity{{0, 0}, value};
    }

    inline quantity make_length(double value) {
        return quantity{{1, 0}, value};
    }

    inline quantity make_angle(double value) {
        return quantity{{0, 1}, value};
    }

    arithmetic_result identity(quantity operand);
    arithmetic_result negate(quantity operand);
    arithmetic_result add(quantity lhs, quantity rhs);
    arithmetic_result subtract(quantity lhs, quantity rhs);
    arithmetic_result multiply(quantity lhs, quantity rhs);
    arithmetic_result divide(quantity lhs, quantity rhs);
    arithmetic_result modulo(quantity lhs, quantity rhs);
    arithmetic_result power(quantity lhs, quantity rhs);
}

std::ostream& operator<<(std::ostream& stream, const expr::quantity& token);

#endif
