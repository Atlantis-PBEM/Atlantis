#ifndef STRING_PARSER_HPP
#define STRING_PARSER_HPP

#include <string>
#include <vector>
#include <optional>
#include <cctype>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <iostream>

#include "strings_util.hpp"
#include "string_filters.hpp"

namespace parser {

/**
 * A simple set of string parser and tokenizer classes that can work with various
 * string types and extract typed values from them without unnecessary reallocations.
 */

class token {
public:
    explicit token(std::optional<strings::ci_string> value) noexcept : value_{std::move(value)} {}
    // Non-explicit to allow implicit conversion from C-style strings
    token(const char* value) noexcept : value_{value ? std::optional<strings::ci_string>{value} : std::nullopt} {}
    // Non-explicit to allow implicit conversion from std::string
    token(const strings::ci_string& value) noexcept : value_{value} {}
    explicit token(strings::ci_string&& value) noexcept : value_{std::move(value)} {}
    // Add constructor for std::string
    token(const std::string& value) noexcept : value_{value.c_str()} {}

    [[nodiscard]] std::optional<int> get_number() const noexcept;
    [[nodiscard]] std::string get_string() const noexcept { return std::string{value_.value_or("").c_str()}; }
    [[nodiscard]] std::optional<bool> get_bool() const noexcept;

    [[nodiscard]] bool operator==(const strings::ci_string& s) const noexcept { return value_.value_or("") == s; }
    [[nodiscard]] bool operator==(const std::string& s) const noexcept { return value_.value_or("") == s; }
    [[nodiscard]] bool operator==(const char* s) const noexcept { return s && value_.value_or("") == s; }
    [[nodiscard]] bool operator!=(const strings::ci_string& s) const noexcept { return value_.value_or("") != s; }
    [[nodiscard]] bool operator!=(const std::string& s) const noexcept { return value_.value_or("") != s; }
    [[nodiscard]] bool operator!=(const char* s) const noexcept { return !s || value_.value_or("") != s; }

    [[nodiscard]] explicit operator bool() const noexcept { return value_.has_value(); }

private:
    std::optional<strings::ci_string> value_;
};

// Forward declarations for friend operators
class string_parser;
std::istream& operator>>(std::istream& is, string_parser& parser);

class string_parser {
public:
    constexpr string_parser() noexcept : data_{}, pos_{0} {}
    explicit string_parser(const std::string& input) noexcept : data_{input}, pos_{0} {}
    explicit string_parser(std::string&& input) noexcept : data_{std::move(input)}, pos_{0} {}
    explicit string_parser(const char* input) noexcept : data_{input ? input : ""}, pos_{0} {}

    // Assignment operators
    string_parser& operator=(const std::string& input) noexcept;
    string_parser& operator=(const char* input) noexcept;

    // Parsing methods
    [[nodiscard]] token get_token();
    [[nodiscard]] bool get_at();
    [[nodiscard]] std::vector<token> tokenize();

    // Utility methods
    void strip_white() noexcept;
    constexpr void reset() noexcept { pos_ = 0; }
    [[nodiscard]] std::string str() const noexcept { return data_.substr(pos_); }
    [[nodiscard]] const std::string& original() const noexcept { return data_; }
    [[nodiscard]] constexpr bool empty() const noexcept { return pos_ >= data_.length(); }
    [[nodiscard]] constexpr size_t length() const noexcept { return pos_ < data_.length() ? data_.length() - pos_ : 0; }

    void set_data(const std::string& input) noexcept { data_ = input; pos_ = 0; }

    friend std::istream& operator>>(std::istream& is, string_parser& parser);

private:
    std::string data_;
    size_t pos_;

    [[nodiscard]] static constexpr bool is_whitespace(char c) noexcept { return c == ' ' || c == '\t'; }
};

//
// Implementation section
//


// string_parser class implementations
inline std::istream& operator>>(std::istream& is, string_parser& parser) {
    std::string input;
    std::getline(is >> std::ws, input);
    parser.set_data(input);
    return is;
}

inline token string_parser::get_token() {
    strip_white();

    if (pos_ >= data_.length()) return token{std::nullopt};
    if (data_[pos_] == ';') {
        pos_ = data_.length();
        return token{std::nullopt};
    }

    std::string temp;
    size_t end_pos;

    if (data_[pos_] == '"') {
        pos_++;
        end_pos = data_.find('"', pos_);

        if (end_pos == std::string::npos) {
            temp = data_.substr(pos_);
            pos_ = data_.length();
            return token{temp};
        }

        temp = data_.substr(pos_, end_pos - pos_);
        pos_ = end_pos + 1;
    } else {
        end_pos = data_.find_first_of(" \t;", pos_);

        if (end_pos == std::string::npos) {
            temp = data_.substr(pos_);
            pos_ = data_.length();
            return token{temp};
        }

        temp = data_.substr(pos_, end_pos - pos_);
        pos_ = end_pos;

        if (data_[end_pos] == ';') {
            pos_ = data_.length();
            return token{temp};
        }
    }

    strip_white();
    return token{temp};
}

inline bool string_parser::get_at() {
    strip_white();

    if (pos_ < data_.length() && data_[pos_] == '@') {
        pos_++;  // Advance past the @ symbol
        return true;
    }

    return false;
}

inline void string_parser::strip_white() noexcept {
    while (pos_ < data_.length() && is_whitespace(data_[pos_])) pos_++;
}

inline std::vector<token> string_parser::tokenize() {
    std::vector<token> tokens;
    tokens.reserve(16);

    while (token temp = get_token()) tokens.push_back(temp);

    return tokens;
}

// token class implementations
inline std::optional<int> token::get_number() const noexcept {
    if (!value_) return std::nullopt;

    const auto& temp = *value_;
    if (temp.empty()) return std::nullopt;

    // Fast path for single-digit numbers
    if (temp.length() == 1 && std::isdigit(temp[0])) return temp[0] - '0';

    // Handle negative numbers and more complex parsing
    bool negative = false;
    size_t pos = 0;

    if (temp[0] == '-') {
        negative = true;
        pos++;
    }

    if (pos >= temp.length() || !std::isdigit(temp[pos])) return std::nullopt;

    constexpr int max_div_10 = std::numeric_limits<int>::max() / 10;
    constexpr int max_mod_10 = std::numeric_limits<int>::max() % 10;

    int value = 0;
    while (pos < temp.length() && std::isdigit(temp[pos])) {
        int digit = temp[pos] - '0';

        // Check for overflow
        if (value > max_div_10 || (value == max_div_10 && digit > max_mod_10)) return std::nullopt;

        value = value * 10 + digit;
        pos++;
    }

    // Make sure the entire string was consumed
    if (pos != temp.length()) return std::nullopt;

    return negative ? -value : value;
}

inline std::optional<bool> token::get_bool() const noexcept {
    if (!value_) return std::nullopt;
    const auto& temp = *value_;

    if (temp == "true" || temp == "t" || temp == "yes" || temp == "y" || temp == "on" || temp == "1")
        return true;
    if (temp == "false" || temp == "f" || temp == "no" || temp == "n" || temp == "off" || temp == "0")
        return false;

    return std::nullopt;
}

// string_parser assignment operators
inline string_parser& string_parser::operator=(const std::string& input) noexcept {
    data_ = input;
    pos_ = 0;
    return *this;
}

inline string_parser& string_parser::operator=(const char* input) noexcept {
    data_ = input ? input : "";
    pos_ = 0;
    return *this;
}

} // namespace parser

#endif // STRING_PARSER_HPP
