#pragma once
#ifndef SCOPED_ENUM_HPP
#define SCOPED_ENUM_HPP

#include <type_traits>

template <typename E, typename UnderlyingType>
using enable_if_enum_has_underlying_type = std::enable_if_t<
    std::is_enum<E>::value && std::is_same<std::underlying_type_t<E>, UnderlyingType>::value
>;

template <typename EnumType, typename = enable_if_enum_has_underlying_type<EnumType, int>>
bool operator==(EnumType lhs, EnumType rhs) {
    return static_cast<std::underlying_type_t<EnumType>>(lhs) == static_cast<std::underlying_type_t<EnumType>>(rhs);
}

template <typename EnumType, typename = enable_if_enum_has_underlying_type<EnumType, int>>
bool operator!=(EnumType lhs, EnumType rhs) {
    return !(lhs == rhs);
}

template <typename EnumType, typename = enable_if_enum_has_underlying_type<EnumType, int>>
int operator<=>(EnumType lhs, EnumType rhs) {
    return static_cast<std::underlying_type_t<EnumType>>(lhs) <=> static_cast<std::underlying_type_t<EnumType>>(rhs);
}

template <typename EnumType, typename = enable_if_enum_has_underlying_type<EnumType, int>>
bool operator<(EnumType lhs, EnumType rhs) {
    return static_cast<std::underlying_type_t<EnumType>>(lhs) < static_cast<std::underlying_type_t<EnumType>>(rhs);
}

template <typename EnumType, typename = enable_if_enum_has_underlying_type<EnumType, int>>
bool operator>(EnumType lhs, EnumType rhs) {
    return static_cast<std::underlying_type_t<EnumType>>(lhs) > static_cast<std::underlying_type_t<EnumType>>(rhs);
}

template <typename EnumType, typename = enable_if_enum_has_underlying_type<EnumType, int>>
bool operator<=(EnumType lhs, EnumType rhs) {
    return lhs < rhs || lhs == rhs;
}

template <typename EnumType, typename = enable_if_enum_has_underlying_type<EnumType, int>>
bool operator>=(EnumType lhs, EnumType rhs) {
    return lhs > rhs || lhs == rhs;
}
#endif // SCOPED_ENUM_HPP
