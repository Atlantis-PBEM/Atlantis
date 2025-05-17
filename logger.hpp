#pragma once
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>

namespace logger {
  namespace detail {
    inline std::ostream* current_stream = &std::cout;
  } // namespace detail

  inline static void set_stream(std::ostream& os) { detail::current_stream = &os; }

  inline static void write(const std::string& s) { *detail::current_stream << s << std::endl; }
  inline static void dot() { *detail::current_stream << "."; }
} // namespace logger

#endif // LOGGER_HPP
