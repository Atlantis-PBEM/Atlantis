#ifndef STRINGS_UTIL_HPP
#define STRINGS_UTIL_HPP

#include <string>
#include <sstream>
#include <optional>
#include <string_view>
#include <span>

namespace strings {

    // Joins elements of a string container into a single string with a delimiter,
    // optionally using a different delimiter before the last element.
    inline const std::string join(
        std::span<const std::string> elements, std::string_view delimiter,
        std::optional<std::string_view> last_delimiter_opt = std::nullopt
    )
    {
        if (elements.empty()) return "";
        if (elements.size() == 1) return elements[0];

        std::string_view last_delimiter = last_delimiter_opt.value_or(delimiter);

        std::ostringstream oss;
        oss << elements[0];

        for (size_t i = 1; i < elements.size() - 1; ++i) oss << delimiter << elements[i];
        oss << last_delimiter << elements.back();

        return oss.str();
    }

    inline const std::string plural(int count, const std::string &one, const std::string &many) {
        return count != 1 ? many : one;
    }
} // namespace strings

#endif // STRINGS_UTIL_HPP
