#include "simplex.h"

#include "logger.hpp"

#include <cstdint>  // int32_t/uint8_t
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include <cmath>

#include "rng.hpp"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

const double TAU = 2 * M_PI;

/**
 * Computes the largest integer value not greater than the float one
 *
 * This method is faster than using (int32_t)std::floor(fp).
 *
 * I measured it to be approximately twice as fast:
 *  float:  ~18.4ns instead of ~39.6ns on an AMD APU),
 *  double: ~20.6ns instead of ~36.6ns on an AMD APU),
 * Reference: http://www.codeproject.com/Tips/700780/Fast-floor-ceiling-functions
 *
 * @param[in] fp    double input value
 *
 * @return largest integer value not greater than fp
 */
static inline int32_t fastfloor(double fp) {
    int32_t i = static_cast<int32_t>(fp);
    return (fp < i) ? (i - 1) : (i);
}

/* NOTE Gradient table to test if lookup-table are more efficient than calculs
static const double gradients1D[16] = {
        -8., -7., -6., -5., -4., -3., -2., -1.,
         1.,  2.,  3.,  4.,  5.,  6.,  7.,  8.
};
*/

SimplexNoise::SimplexNoise(
    const double frequency, const double amplitude, const double lacunarity, const double persistence
) : mFrequency(frequency), mAmplitude(amplitude), mLacunarity(lacunarity), mPersistence(persistence)
{
    const uint32_t N = 256;
    perm.reserve(N);

    for (uint32_t i = 0; i < N; i++) {
        perm.push_back(i);
    }

    rng::shuffle(perm);
}

uint8_t SimplexNoise::h(int32_t i) {
    return perm[static_cast<uint8_t>(i)];
}

double SimplexNoise::grad(int32_t hash, double x) {
    const int32_t h = hash & 0x0F;  // Convert low 4 bits of hash code
    double grad = 1.0 + (h & 7);    // Gradient value 1.0, 2.0, ..., 8.0
    if ((h & 8) != 0) grad = -grad; // Set a random sign for the gradient
//  double grad = gradients1D[h];    // NOTE : Test of Gradient look-up table instead of the above
    return (grad * x);              // Multiply the gradient with the distance
}


double SimplexNoise::grad(int32_t hash, double x, double y) {
    const int32_t h = hash & 0x3F;  // Convert low 3 bits of hash code
    const double u = h < 4 ? x : y;  // into 8 simple gradient directions,
    const double v = h < 4 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -2.0 * v : 2.0 * v); // and compute the dot product with (x,y).
}


double SimplexNoise::grad(int32_t hash, double x, double y, double z) {
    int h = hash & 15;     // Convert low 4 bits of hash code into 12 simple
    double u = h < 8 ? x : y; // gradient directions, and compute dot product.
    double v = h < 4 ? y : h == 12 || h == 14 ? x : z; // Fix repeats at h = 12 to 15
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

/**
 * 1D Perlin simplex noise
 *
 *  Takes around 74ns on an AMD APU.
 *
 * @param[in] x double coordinate
 *
 * @return Noise value in the range[-1; 1], value of 0 on all integer coordinates.
 */
double SimplexNoise::noise(const double x) {
    double n0, n1;   // Noise contributions from the two "corners"

    // No need to skew the input space in 1D

    // Corners coordinates (nearest integer values):
    int32_t i0 = fastfloor(x);
    int32_t i1 = i0 + 1;
    // Distances to corners (between 0 and 1):
    double x0 = x - i0;
    double x1 = x0 - 1.0;

    // Calculate the contribution from the first corner
    double t0 = 1.0 - x0*x0;
//  if(t0 < 0.0) t0 = 0.0; // not possible
    t0 *= t0;
    n0 = t0 * t0 * grad(h(i0), x0);

    // Calculate the contribution from the second corner
    double t1 = 1.0 - x1*x1;
//  if(t1 < 0.0) t1 = 0.0; // not possible
    t1 *= t1;
    n1 = t1 * t1 * grad(h(i1), x1);

    // The maximum value of this noise is 8*(3/4)^4 = 2.53125
    // A factor of 0.395 scales to fit exactly within [-1,1]
    return 0.395 * (n0 + n1);
}

/**
 * 2D Perlin simplex noise
 *
 *  Takes around 150ns on an AMD APU.
 *
 * @param[in] x double coordinate
 * @param[in] y double coordinate
 *
 * @return Noise value in the range[-1; 1], value of 0 on all integer coordinates.
 */
double SimplexNoise::noise(const double x, const double y) {
    double n0, n1, n2;   // Noise contributions from the three corners

    // Skewing/Unskewing factors for 2D
    static const double F2 = 0.366025403;  // F2 = (sqrt(3) - 1) / 2
    static const double G2 = 0.211324865;  // G2 = (3 - sqrt(3)) / 6   = F2 / (1 + 2 * K)

    // Skew the input space to determine which simplex cell we're in
    const double s = (x + y) * F2;  // Hairy factor for 2D
    const double xs = x + s;
    const double ys = y + s;
    const int32_t i = fastfloor(xs);
    const int32_t j = fastfloor(ys);

    // Unskew the cell origin back to (x,y) space
    const double t = static_cast<double>(i + j) * G2;
    const double X0 = i - t;
    const double Y0 = j - t;
    const double x0 = x - X0;  // The x,y distances from the cell origin
    const double y0 = y - Y0;

    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int32_t i1, j1;  // Offsets for second (middle) corner of simplex in (i,j) coords
    if (x0 > y0) {   // lower triangle, XY order: (0,0)->(1,0)->(1,1)
        i1 = 1;
        j1 = 0;
    } else {   // upper triangle, YX order: (0,0)->(0,1)->(1,1)
        i1 = 0;
        j1 = 1;
    }

    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6

    const double x1 = x0 - i1 + G2;            // Offsets for middle corner in (x,y) unskewed coords
    const double y1 = y0 - j1 + G2;
    const double x2 = x0 - 1.0 + 2.0 * G2;   // Offsets for last corner in (x,y) unskewed coords
    const double y2 = y0 - 1.0 + 2.0 * G2;

    // Work out the hashed gradient indices of the three simplex corners
    const int gi0 = h(i + h(j));
    const int gi1 = h(i + i1 + h(j + j1));
    const int gi2 = h(i + 1 + h(j + 1));

    // Calculate the contribution from the first corner
    double t0 = 0.5 - x0*x0 - y0*y0;
    if (t0 < 0.0) {
        n0 = 0.0;
    } else {
        t0 *= t0;
        n0 = t0 * t0 * grad(gi0, x0, y0);
    }

    // Calculate the contribution from the second corner
    double t1 = 0.5 - x1*x1 - y1*y1;
    if (t1 < 0.0) {
        n1 = 0.0;
    } else {
        t1 *= t1;
        n1 = t1 * t1 * grad(gi1, x1, y1);
    }

    // Calculate the contribution from the third corner
    double t2 = 0.5 - x2*x2 - y2*y2;
    if (t2 < 0.0) {
        n2 = 0.0;
    } else {
        t2 *= t2;
        n2 = t2 * t2 * grad(gi2, x2, y2);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 45.23065 * (n0 + n1 + n2);
}


/**
 * 3D Perlin simplex noise
 *
 * @param[in] x double coordinate
 * @param[in] y double coordinate
 * @param[in] z double coordinate
 *
 * @return Noise value in the range[-1; 1], value of 0 on all integer coordinates.
 */
double SimplexNoise::noise(const double x, const double y, const double z) {
    double n0, n1, n2, n3; // Noise contributions from the four corners

    // Skewing/Unskewing factors for 3D
    static const double F3 = 1.0 / 3.0;
    static const double G3 = 1.0 / 6.0;

    // Skew the input space to determine which simplex cell we're in
    double s = (x + y + z) * F3; // Very nice and simple skew factor for 3D
    int i = fastfloor(x + s);
    int j = fastfloor(y + s);
    int k = fastfloor(z + s);
    double t = (i + j + k) * G3;
    double X0 = i - t; // Unskew the cell origin back to (x,y,z) space
    double Y0 = j - t;
    double Z0 = k - t;
    double x0 = x - X0; // The x,y,z distances from the cell origin
    double y0 = y - Y0;
    double z0 = z - Z0;

    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords
    if (x0 >= y0) {
        if (y0 >= z0) {
            i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; // X Y Z order
        } else if (x0 >= z0) {
            i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; // X Z Y order
        } else {
            i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; // Z X Y order
        }
    } else { // x0<y0
        if (y0 < z0) {
            i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; // Z Y X order
        } else if (x0 < z0) {
            i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; // Y Z X order
        } else {
            i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; // Y X Z order
        }
    }

    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.
    double x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
    double y1 = y0 - j1 + G3;
    double z1 = z0 - k1 + G3;
    double x2 = x0 - i2 + 2.0 * G3; // Offsets for third corner in (x,y,z) coords
    double y2 = y0 - j2 + 2.0 * G3;
    double z2 = z0 - k2 + 2.0 * G3;
    double x3 = x0 - 1.0 + 3.0 * G3; // Offsets for last corner in (x,y,z) coords
    double y3 = y0 - 1.0 + 3.0 * G3;
    double z3 = z0 - 1.0 + 3.0 * G3;

    // Work out the hashed gradient indices of the four simplex corners
    int gi0 = h(i + h(j + h(k)));
    int gi1 = h(i + i1 + h(j + j1 + h(k + k1)));
    int gi2 = h(i + i2 + h(j + j2 + h(k + k2)));
    int gi3 = h(i + 1 + h(j + 1 + h(k + 1)));

    // Calculate the contribution from the four corners
    double t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
    if (t0 < 0) {
        n0 = 0.0;
    } else {
        t0 *= t0;
        n0 = t0 * t0 * grad(gi0, x0, y0, z0);
    }
    double t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
    if (t1 < 0) {
        n1 = 0.0;
    } else {
        t1 *= t1;
        n1 = t1 * t1 * grad(gi1, x1, y1, z1);
    }
    double t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
    if (t2 < 0) {
        n2 = 0.0;
    } else {
        t2 *= t2;
        n2 = t2 * t2 * grad(gi2, x2, y2, z2);
    }
    double t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
    if (t3 < 0) {
        n3 = 0.0;
    } else {
        t3 *= t3;
        n3 = t3 * t3 * grad(gi3, x3, y3, z3);
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return 32.0*(n0 + n1 + n2 + n3);
}


/**
 * Fractal/Fractional Brownian Motion (fBm) summation of 1D Perlin Simplex noise
 *
 * @param[in] octaves   number of fraction of noise to sum
 * @param[in] x         double coordinate
 *
 * @return Noise value in the range[-1; 1], value of 0 on all integer coordinates.
 */
double SimplexNoise::fractal(const size_t octaves, const double x) {
    double output    = 0.;
    double denom     = 0.;
    double frequency = mFrequency;
    double amplitude = mAmplitude;

    for (size_t i = 0; i < octaves; i++) {
        output += (amplitude * noise(x * frequency));
        denom += amplitude;

        frequency *= mLacunarity;
        amplitude *= mPersistence;
    }

    return (output / denom);
}

/**
 * Fractal/Fractional Brownian Motion (fBm) summation of 2D Perlin Simplex noise
 *
 * @param[in] octaves   number of fraction of noise to sum
 * @param[in] x         x double coordinate
 * @param[in] y         y double coordinate
 *
 * @return Noise value in the range[-1; 1], value of 0 on all integer coordinates.
 */
double SimplexNoise::fractal(const size_t octaves, const double x, const double y) {
    double output = 0.;
    double denom  = 0.;
    double frequency = mFrequency;
    double amplitude = mAmplitude;

    for (size_t i = 0; i < octaves; i++) {
        output += (amplitude * noise(x * frequency, y * frequency));
        denom += amplitude;

        frequency *= mLacunarity;
        amplitude *= mPersistence;
    }

    return (output / denom);
}

/**
 * Fractal/Fractional Brownian Motion (fBm) summation of 3D Perlin Simplex noise
 *
 * @param[in] octaves   number of fraction of noise to sum
 * @param[in] x         x double coordinate
 * @param[in] y         y double coordinate
 * @param[in] z         z double coordinate
 *
 * @return Noise value in the range[-1; 1], value of 0 on all integer coordinates.
 */
double SimplexNoise::fractal(const size_t octaves, const double x, const double y, const double z) {
    double output = 0.;
    double denom  = 0.;
    double frequency = mFrequency;
    double amplitude = mAmplitude;

    for (size_t i = 0; i < octaves; i++) {
        output += (amplitude * noise(x * frequency, y * frequency, z * frequency));
        denom += amplitude;

        frequency *= mLacunarity;
        amplitude *= mPersistence;
    }

    return (output / denom);
}

double SimplexNoise::cylinderNoise(const double nx, const double ny) {
    double angle_x = TAU * nx;
    /* In "noise parameter space", we need nx and ny to travel the
       same distance. The circle created from nx needs to have
       circumference=1 to match the length=1 line created from ny,
       which means the circle's radius is 1/2π, or 1/tau */
    return noise(cos(angle_x) / TAU, sin(angle_x) / TAU, ny);
}

// Fractal/Fractional Brownian Motion (fBm) noise summation
double SimplexNoise::cylinderFractal(const size_t octaves, const double nx, const double ny) {
    double angle_x = TAU * nx;
    /* In "noise parameter space", we need nx and ny to travel the
       same distance. The circle created from nx needs to have
       circumference=1 to match the length=1 line created from ny,
       which means the circle's radius is 1/2π, or 1/tau */
    return fractal(octaves, cos(angle_x) / TAU, sin(angle_x) / TAU, ny);
}
