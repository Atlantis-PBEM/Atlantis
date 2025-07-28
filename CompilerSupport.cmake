include(CheckCXXSourceCompiles)

if(MSVC AND (MSVC_TOOLSET_VERSION GREATER_EQUAL 143))
# we know that modern MSVC versions have the required C++20 support
set (COMPILER_HAS_RANGES TRUE)
# we know that GCC before 12 does not have the required C++20 support
elseif((CMAKE_CXX_COMPILER_ID EQUALS "GNU") AND (CMAKE_CXX_COMPILER_VERSION LESS 12))
set (COMPILER_HAS_RANGES FALSE)
else()
# for all other compilers (including clang), we check if they can compile this sample code
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
