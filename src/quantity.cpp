#include "quantity.h"
#include "utility.h"

#include <cfloat>
#include <cmath>
#include <iostream>

using unit_result = expr::result<expr::measurement_unit, expr::error>;

static bool is_integer(double number) {
    return number - std::floor(number) < DBL_EPSILON;
}

static unit_result add_or_subtract_unit(
    expr::measurement_unit lhs,
    expr::measurement_unit rhs
) {
    if (lhs != rhs) {
        return expr::error{
            .code = expr::error_code::QUANTITY_INVALID_BINARY_OPERATION,
            .location = {},
            .description = "Invalid binary operation."
        };
    }
    return lhs;
}

static unit_result multiply_unit(
    expr::measurement_unit lhs,
    expr::measurement_unit rhs
) {
    return expr::measurement_unit{
        .length_dimension = lhs.length_dimension + rhs.length_dimension,
        .angle_dimension = lhs.angle_dimension + rhs.angle_dimension,
    };
}

static unit_result divide_unit(
    expr::measurement_unit lhs,
    expr::measurement_unit rhs
) {
    return expr::measurement_unit{
        .length_dimension = lhs.length_dimension - rhs.length_dimension,
        .angle_dimension = lhs.angle_dimension - rhs.angle_dimension,
    };
}

static unit_result exponentiate_unit(
    expr::measurement_unit lhs,
    expr::quantity rhs
) {
    if (!is_integer(rhs.value) || !rhs.is_scalar()) {
        return expr::error{
            .code = expr::error_code::QUANTITY_SCALAR_INTEGER_EXPECTED_AS_POWER,
            .location = {},
            .description = "Scalar integer expected as power."
        };
    }

    return expr::measurement_unit{
        .length_dimension = lhs.length_dimension * int(rhs.value),
        .angle_dimension = lhs.angle_dimension * int(rhs.value),
    };
}

bool expr::measurement_unit::is_scalar() const {
    return length_dimension == 0 && angle_dimension == 0;
}

bool expr::measurement_unit::is_length() const {
    return length_dimension == 1 && angle_dimension == 0;
}

bool expr::measurement_unit::is_area() const {
    return length_dimension == 2 && angle_dimension == 0;
}

bool expr::measurement_unit::is_volume() const {
    return length_dimension == 3 && angle_dimension == 0;
}

bool expr::measurement_unit::is_angle() const {
    return length_dimension == 0 && angle_dimension == 1;
}

bool expr::measurement_unit::is_mixed() const {
    return length_dimension > 0 && angle_dimension > 0;
}

expr::arithmetic_result expr::identity(expr::quantity operand) {
    return operand;
}

expr::arithmetic_result expr::negate(expr::quantity operand) {
    operand.value *= -1;
    return operand;
}

expr::arithmetic_result expr::add(expr::quantity lhs, expr::quantity rhs) {
    if (auto unit = add_or_subtract_unit(lhs.unit, rhs.unit)) {
        return expr::quantity{.unit = *unit, .value = lhs.value + rhs.value};
    } else {
        return unit.error();
    }
}

expr::arithmetic_result expr::subtract(expr::quantity lhs, expr::quantity rhs) {
    if (auto unit = add_or_subtract_unit(lhs.unit, rhs.unit)) {
        return expr::quantity{.unit = *unit, .value = lhs.value - rhs.value};
    } else {
        return unit.error();
    }
}

expr::arithmetic_result expr::multiply(expr::quantity lhs, expr::quantity rhs) {
    if (auto unit = multiply_unit(lhs.unit, rhs.unit)) {
        return expr::quantity{.unit = *unit, .value = lhs.value * rhs.value};
    } else {
        return unit.error();
    }
}

expr::arithmetic_result expr::divide(expr::quantity lhs, expr::quantity rhs) {
    if (auto unit = divide_unit(lhs.unit, rhs.unit)) {
        if (expr::is_near(rhs.value, 0)) {
             return expr::error{
                .code = expr::error_code::QUANTITY_DIVISION_BY_ZERO,
                .location = {},
                .description = "Division by zero."
            };
        }
        return expr::quantity{.unit = *unit, .value = lhs.value / rhs.value};
    } else {
        return unit.error();
    }
}

expr::arithmetic_result expr::modulo(expr::quantity lhs, expr::quantity rhs) {
    if (lhs.unit == rhs.unit) {
        return expr::quantity{
            .unit = lhs.unit,
            .value = std::fmod(lhs.value, rhs.value)
        };
    } else {
        return expr::error{
            .code = expr::error_code::QUANTITY_EXPECTED_SAME_UNIT,
            .location = {},
            .description = "Expected operands with identical units."
        };

    }
}

expr::arithmetic_result expr::power(expr::quantity lhs, expr::quantity rhs) {
    if (auto unit = exponentiate_unit(lhs.unit, rhs)) {
        return expr::quantity{
            .unit = *unit,
            .value = std::pow(lhs.value, rhs.value)
        };
    } else {
        return unit.error();
    }
}

std::ostream& operator<<(std::ostream& stream, const expr::quantity& quantity) {
    stream << quantity.value;

    if (quantity.unit.length_dimension > 0) {
        stream << " m";
        if (quantity.unit.length_dimension > 1)
            stream << '^' << quantity.unit.length_dimension;
    }

    if (quantity.unit.angle_dimension > 0) {
        stream << " rad";
        if (quantity.unit.angle_dimension > 1)
            stream << '^' << quantity.unit.angle_dimension;
    }

    return stream;
}
