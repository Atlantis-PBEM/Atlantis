#pragma once
#ifndef _RNG_HPP
#define _RNG_HPP

#include <random>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <optional>
#include <type_traits>
#include <iterator>
#include <concepts>

namespace rng {
namespace detail {

// Returns a reference to the lazily initialized generator
inline std::mt19937& get_initialized_generator() {
    // This static generator is initialized only on the first call to this function.
    static std::mt19937 twister = [] {
        std::minstd_rand minstd_rand(std::random_device{}());
        std::seed_seq seed_seq{
            minstd_rand(), minstd_rand(), minstd_rand(),
            minstd_rand(), minstd_rand(), minstd_rand()
        };
        return std::mt19937(seed_seq);
    }();
    return twister;
}

// Seeds the generator
inline void seed_generator(std::seed_seq& seed_seq) {
    get_initialized_generator().seed(seed_seq);
}

} // namespace detail

inline void seed_random() {
    // Re-seed with a new random device value - note this creates a new sequence
    std::minstd_rand minstd_rand(std::random_device{}());
    std::seed_seq seed_seq{
        minstd_rand(), minstd_rand(), minstd_rand(),
        minstd_rand(), minstd_rand(), minstd_rand()
    };
    detail::seed_generator(seed_seq);
}

inline void seed_random(int seed) {
    std::minstd_rand minstd_rand(seed); // Create a minimal standard random number generator with the provided seed
    std::seed_seq seed_seq{
        minstd_rand(), minstd_rand(), minstd_rand(),
        minstd_rand(), minstd_rand(), minstd_rand()
    };
    detail::seed_generator(seed_seq); // Seed with a specific value
}

inline int get_random(int range) {
    static int last_range = 0; // this will let us reuse the same distribution for multiple calls with same range.
    static std::uniform_int_distribution<> distribution(0, 1); // default distribution

    int neg = (range < 0);
    if (!range) return 0;
    if (neg) range = -range;

    if (range != last_range) {
        last_range = range; // Update last_range to the current range
        // Create a new distribution for the new range
        distribution.param(std::uniform_int_distribution<>::param_type(0, range - 1));
    }

    int ret = distribution(detail::get_initialized_generator());
    if (neg) ret = -ret;
    return ret;
}

inline int make_roll(int rolls, int sides) {
    int result = 0;
    for (int i = 0; i < rolls; i++) {
        // Generate a random number in the range [1, sides]
        result += get_random(sides) + 1; // +1 to shift from [0, sides-1] to [1, sides]
    }
    return result;
}

inline std::mt19937& generator() {
    // Return the generator instance
    return detail::get_initialized_generator();
}

// Returns a const reference to a random element from a container
// Requires the container to support .at() and size().
inline const std::string& one_of(const std::vector<std::string>& container) {
    const auto size = container.size();
    if (size == 0) throw std::out_of_range("Cannot select one_of from an empty container.");
    // Ensure range is non-negative for get_random
    if (size > static_cast<size_t>(std::numeric_limits<int>::max())) {
         throw std::overflow_error("Container size exceeds representable range for int.");
    }
    int index = get_random(static_cast<int>(size));
    return container.at(index);
}

// Calculates the number of items lost given an amount and a percentage chance of loss per item.
inline int calculate_losses(int amount, int percentage) {
    if (amount <= 0) return 0; // No items to potentially lose

    // Clamp percentage to the valid range [0, 100]
    int clamped_percentage = std::clamp(percentage, 0, 100);

    if (clamped_percentage == 0) return 0; // 0% chance means no losses
    if (clamped_percentage == 100) return amount; // 100% chance means all items are lost


    // Probability of loss for a single item
    double probability = static_cast<double>(clamped_percentage) / 100.0;

    // Use binomial distribution: 'amount' trials, 'probability' chance of success (loss) per trial
    std::binomial_distribution<> distribution(amount, probability);
    return distribution(detail::get_initialized_generator());
}

// Shuffles the elements in the given container in place using the internal generator.
template <typename C>
inline void shuffle(C& container) {
    std::shuffle(container.begin(), container.end(), detail::get_initialized_generator());
}

// Concept to check if a type is a range with an unsigned integral value type
template <typename T>
concept UnsignedIntegralRange = requires(T c) {
    typename T::value_type;
    requires std::ranges::range<T> || requires { c.begin(); c.end(); };
    requires std::unsigned_integral<typename T::value_type>;
};

// Performs a weighted random selection based on the provided weights.
// Returns an optional containing the index of the selected weight in the input container,
// or std::nullopt if the input is invalid (empty, or all weights are 0).
// The container must satisfy the UnsignedIntegralRange concept.
template <UnsignedIntegralRange WeightContainer>
inline std::optional<size_t> get_weighted_index(const WeightContainer& weights) {

    if (std::ranges::empty(weights)) return std::nullopt;

    unsigned long long weight_sum = 0;
    for(const auto& weight : weights) weight_sum += weight;

    if (weight_sum == 0) return std::nullopt;

    std::discrete_distribution<size_t> distribution(std::ranges::begin(weights), std::ranges::end(weights));
    return distribution(detail::get_initialized_generator());
}

} // namespace rng
#endif // _RNG_HPP
