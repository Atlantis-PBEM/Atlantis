include(CheckCXXSourceCompiles)

if ((CMAKE_CXX_COMPILER_ID EQUALS "GNU") AND (CMAKE_CXX_COMPILER_VERSION LESS_EQUAL 11))
set (COMPILER_HAS_RANGES FALSE)
else()

check_cxx_source_compiles("
#include <ranges>
#include <vector>
#include <algorithm>
#include <string>

int main() {
    auto parts_view = std::string(\"Hello World\")
        | std::views::split(std::string_view{\" \"}) // Split by space
        | std::views::transform([](auto&& subrange) {
            return std::string(subrange.begin(), subrange.end());
        });

    std::vector<std::string> processed_words;
    std::ranges::copy(parts_view, std::back_inserter(processed_words));
    return (2 == processed_words.size()) ? 0 : 1;
}
" COMPILER_HAS_RANGES)
endif()
