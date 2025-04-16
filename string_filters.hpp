#pragma once

#include <string>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <iterator>
#include <unordered_set>

namespace filter {

/**
 * Utility that filters a string to include only legal characters.
 * Usage: std::string result = legal_characters(input);
 * Or with pipe syntax: std::string result = input | legal_characters;
 */
struct legal_characters_t {
    std::string process(const std::string& str) const {
        static const std::unordered_set<char> legal_special_chars {
            ' ', '!', ',', '.', '{', '}', '@', '#', '$', '%', '^', '&', '*',
            '-', '_', '+', '=', ':', '<', '>', '?', '/', '~', '\'', '\\', '`'
        };

        auto is_legal = [](char c) { return std::isalnum(c) || legal_special_chars.count(c) > 0; };

        std::string result;
        result.reserve(str.size());

        std::copy_if(str.begin(), str.end(), std::back_inserter(result), [is_legal](char c) { return is_legal(c); });

        if (result.find_first_not_of(' ') == std::string::npos) result.clear();

        return result;
    }

    std::string operator()(const std::string& str) const { return process(str); }

    friend std::string operator|(const std::string& str, const legal_characters_t& filter) { return filter(str); }
};

// Not constexpr due to the unordered_set member

// Not constexpr due to the unordered_set member
inline legal_characters_t legal_characters{};

/**
 * Utility that strips text after the last '[' or '(' character.
 * Usage: std::string result = strip_number(input);
 * Or with pipe syntax: std::string result = input | strip_number;
 */
struct strip_number_t {
    std::string process(const std::string& str) const {
        auto pos = str.find_last_of("[(");

        if (pos == std::string::npos) return str;

        auto trim_pos = pos;
        // NOTE: this should probably be a while loop, but there are units who named themselve with trailing spaces
        // and I don't want to change that at this time.
        if (trim_pos > 0 && std::isspace(str[trim_pos-1])) trim_pos--;

        return str.substr(0, trim_pos);
    }

    std::string operator()(const std::string& str) const { return process(str); }

    friend std::string operator|(const std::string& str, const strip_number_t& stripper) { return stripper(str); }
};

inline constexpr strip_number_t strip_number{};

} // namespace filter
