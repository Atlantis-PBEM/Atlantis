/**
 * @file    SimplexNoise.h
 * @brief   A Perlin Simplex Noise C++ Implementation (1D, 2D, 3D).
 *
 * Copyright (c) 2014-2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */
#pragma once

#include <cstddef>  // size_t
#include <cstdint>
#include <vector>

/**
 * @brief A Perlin Simplex Noise C++ Implementation (1D, 2D, 3D, 4D).
 */
class SimplexNoise {
public:
    /**
     * Constructor of to initialize a fractal noise summation
     *
     * @param[in] frequency    Frequency ("width") of the first octave of noise (default to 1.0)
     * @param[in] amplitude    Amplitude ("height") of the first octave of noise (default to 1.0)
     * @param[in] lacunarity   Lacunarity specifies the frequency multiplier between successive octaves (default to 2.0).
     * @param[in] persistence  Persistence is the loss of amplitude between successive octaves (usually 1/lacunarity)
     */
    SimplexNoise(
        const double frequency = 1.0f, const double amplitude = 1.0f, const double lacunarity = 2.0f, const double persistence = 0.5f
    );

    // 1D Perlin simplex noise
    double noise(const double x);
    
    // 2D Perlin simplex noise
    double noise(const double x, const double y);
    
    // 3D Perlin simplex noise
    double noise(const double x, const double y, const double z);

    // Fractal/Fractional Brownian Motion (fBm) noise summation
    double fractal(const size_t octaves, const double x);
    double fractal(const size_t octaves, const double x, const double y);
    double fractal(const size_t octaves, const double x, const double y, const double z);

    // 2D Perlin simplex noise on cylinder
    double cylinderNoise(const double nx, const double ny);

    // Fractal/Fractional Brownian Motion (fBm) noise summation
    double cylinderFractal(const size_t octaves, const double nx, const double ny);

private:
    // Parameters of Fractional Brownian Motion (fBm) : sum of N "octaves" of noise
    double mFrequency;   ///< Frequency ("width") of the first octave of noise (default to 1.0)
    double mAmplitude;   ///< Amplitude ("height") of the first octave of noise (default to 1.0)
    double mLacunarity;  ///< Lacunarity specifies the frequency multiplier between successive octaves (default to 2.0).
    double mPersistence; ///< Persistence is the loss of amplitude between successive octaves (usually 1/lacunarity)

    /**
     * Permutation table. This is just a random jumble of all numbers 0-255.
     *
     * This produce a repeatable pattern of 256, but Ken Perlin stated
     * that it is not a problem for graphic texture as the noise features disappear
     * at a distance far enough to be able to see a repeatable pattern of 256.
     *
     * This needs to be exactly the same for all instances on all platforms,
     * so it's easiest to just keep it as static explicit data.
     * This also removes the need for any initialisation of this class.
     *
     * Note that making this an uint32_t[] instead of a uint8_t[] might make the
     * code run faster on platforms with a high penalty for unaligned single
     * byte addressing. Intel x86 is generally single-byte-friendly, but
     * some other CPUs are faster with 4-aligned reads.
     * However, a char[] is smaller, which avoids cache trashing, and that
     * is probably the most important aspect on most architectures.
     * This array is accessed a *lot* by the noise functions.
     * A vector-valued noise over 3D accesses it 96 times, and a
     * float-valued 4D noise 64 times. We want this to fit in the cache!
     */
    std::vector<uint8_t> perm;

    /**
     * Helper function to hash an integer using the above permutation table
     *
     *  This inline function costs around 1ns, and is called N+1 times for a noise of N dimension.
     *
     *  Using a real hash function would be better to improve the "repeatability of 256" of the above permutation table,
     * but fast integer Hash functions uses more time and have bad random properties.
     *
     * @param[in] i Integer value to hash
     *
     * @return 8-bits hashed value
     */
    uint8_t h(int32_t i);

    /**
     * Helper function to compute gradients-dot-residual vectors (1D)
     *
     * @note that these generate gradients of more than unit length. To make
     * a close match with the value range of classic Perlin noise, the final
     * noise values need to be rescaled to fit nicely within [-1,1].
     * (The simplex noise functions as such also have different scaling.)
     * Note also that these noise functions are the most practical and useful
     * signed version of Perlin noise.
     *
     * @param[in] hash  hash value
     * @param[in] x     distance to the corner
     *
     * @return gradient value
     */
    double grad(int32_t hash, double x);

    /**
     * Helper functions to compute gradients-dot-residual vectors (2D)
     *
     * @param[in] hash  hash value
     * @param[in] x     x coord of the distance to the corner
     * @param[in] y     y coord of the distance to the corner
     *
     * @return gradient value
     */
    double grad(int32_t hash, double x, double y);

    /**
     * Helper functions to compute gradients-dot-residual vectors (3D)
     *
     * @param[in] hash  hash value
     * @param[in] x     x coord of the distance to the corner
     * @param[in] y     y coord of the distance to the corner
     * @param[in] z     z coord of the distance to the corner
     *
     * @return gradient value
     */
    double grad(int32_t hash, double x, double y, double z);
};