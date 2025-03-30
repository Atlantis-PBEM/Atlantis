#ifndef _RNG_H
#define _RNG_H

#include <random>

class rng
{
    public:
    inline static void seed_random() {
        seed_random(std::random_device{}()); // Seed with a random value using std::random_device
    }

    inline static void seed_random(int seed) {
        std::minstd_rand minstd_rand(seed); // Create a minimal standard random number generator with the provided seed
        std::seed_seq seed_seq{
            minstd_rand(), minstd_rand(), minstd_rand(),
            minstd_rand(), minstd_rand(), minstd_rand()
        };
        instance().seed(seed_seq); // Seed with a specific value
    }

    inline static int get_random(int range) {
        static int last_range = 0; // this will let us reuse the same distribution for multiple calls with same range.
        static std::uniform_int_distribution<> distribution(0, 1); // default distribution

        if (range != last_range) {
            last_range = range; // Update last_range to the current range
            // Create a new distribution for the new range
            distribution.param(std::uniform_int_distribution<>::param_type(0, range - 1));
        }

        int neg = (range < 0);
        if (!range) return 0;
        if (neg) range = -range;
        int ret = distribution(instance().generator());
        if (neg) ret = -ret;
        return ret;
    }

    inline static int make_roll(int rolls, int sides) {
        int result = 0;
        for (int i = 0; i < rolls; i++) {
            // Generate a random number in the range [1, sides]
            result += get_random(sides) + 1; // +1 to shift from [0, sides-1] to [1, sides]
        }
        return result;
    }

    inline static std::mt19937& generator() {
        // Return the generator instance
        return instance().twister;
    }

    protected:
    inline static rng& instance() {
        static rng instance;
        return instance;
    }

    private:
        std::mt19937 twister; // Mersenne Twister random number generator

        // Private constructor to prevent instantiation
        inline rng() {
            // Initialize the random number generator with a seed
            // This will be called only once when the instance is created
            std::minstd_rand minstd_rand(std::random_device{}());
            std::seed_seq seed_seq{
                minstd_rand(), minstd_rand(), minstd_rand(),
                minstd_rand(), minstd_rand(), minstd_rand()
            };
            twister.seed(seed_seq);
        }

        inline void seed(std::seed_seq& seed_seq) {
            // Seed the generator with a seed sequence
            twister.seed(seed_seq);
        }

        // Delete copy constructor and assignment operator
        rng(const rng&) = delete;
        rng& operator=(const rng&) = delete;
};
#endif // _RNG_H
