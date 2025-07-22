#pragma once
#ifndef STRING_FILTERS_HPP
#define STRING_FILTERS_HPP
#include "strings_util.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iterator>
#include <string>
#include <unordered_set>
#include <vector>
#include <string_view>
#ifdef CPP20_RANGES_ARE_BROKEN
#include <sstream>
#include <iostream>
#else
#include <ranges>
#endif
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

/**
 * Utility that capitalizes a string.
 * Usage: std::string result = capitalize(input);
 * Or with pipe syntax: std::string result = input | capitalize;
 */
struct capitalize_t {
    std::string process(const std::string& str) const {
        std::string result = str;
        if (!result.empty()) {
            result[0] = std::toupper(result[0]);
        }
        return result;
    }

    std::string operator()(const std::string& str) const { return process(str); }

    friend std::string operator|(const std::string& str, const capitalize_t& capitalizer) { return capitalizer(str); }
};

inline constexpr capitalize_t capitalize{};

/**
 * Utility that lowercases a string.
 * Usage: std::string result = lowercase(input);
 * Or with pipe syntax: std::string result = input | lowercase;
 */
struct lowercase_t {
    std::string process(const std::string& str) const {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    std::string operator()(const std::string& str) const { return process(str); }

    friend std::string operator|(const std::string& str, const lowercase_t& lowercaser) { return lowercaser(str); }
};

inline constexpr lowercase_t lowercase{};

/**
 * Utility that canonicalizes a string.
 * It capitalizes each word and replaces spaces between words with underscores.
 * Example: "summon wind" -> "Summon_Wind"
 * Usage: std::string result = canonicalize(input);
 * Or with pipe syntax: std::string result = input | canonicalize;
 */
struct canonicalize_t {
    std::string process(const std::string& str) const {
#ifdef CPP20_RANGES_ARE_BROKEN
        /** gcc 11 doesn't like the range solution, ergo this hack */
        std::vector<std::string> words;
        std::istringstream f(str);
        std::string s;
        while (std::getline(f, s, ' ')) {
            words.push_back(s | capitalize);
        }
        return strings::join(words, "_");
#else
        auto parts_view = str
            | std::views::split(std::string_view{" "}) // Split by space
            | std::views::transform([](auto&& subrange) {
                std::string str = std::string(subrange.begin(), subrange.end());
                return str | capitalize;
            });

        std::vector<std::string> processed_words;
        // Efficiently copy the view elements into the vector
        std::ranges::copy(parts_view, std::back_inserter(processed_words));
        // Join the processed words with an underscore
        return strings::join(processed_words, "_");
#endif
    }

    std::string operator()(const std::string& str) const { return process(str); }

    friend std::string operator|(const std::string& str, const canonicalize_t& canonicalizer) {
        return canonicalizer(str);
    }
};

inline constexpr canonicalize_t canonicalize{};

} // namespace filter

#endif // STRING_FILTERS_HPP
