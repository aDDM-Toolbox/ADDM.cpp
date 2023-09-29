#ifndef STATS_H
#define STATS_H

#include <boost/math/distributions/normal.hpp>

/**
 * @brief Compute the probability density function provided mean and standard deviation. 
 * 
 * @param mean Mean of the distribution.
 * @param sigma Standard Deviation of the distribution.
 * @param x Value to compute the PDF at. 
 * @return double containing the computed PDF. 
 */
inline double probabilityDensityFunction(float mean, float sigma, float x) {
    boost::math::normal_distribution<double> dist(mean, sigma);
    double pdf = boost::math::pdf(dist, x);
    return pdf; 
}

/**
 * @brief Compute the cumulative density function provided mean and standard deviation. 
 * 
 * @param mean Mean of the distribution. 
 * @param sigma Standard deviation of the distribution. 
 * @param x Value to compute the CDF at. 
 * @return double containing the computed CDF. 
 */
inline double cumulativeDensityFunction(float mean, float sigma, float x) {
    boost::math::normal_distribution<double> dist(mean, sigma);
    double cdf = boost::math::cdf(dist, x);
    return cdf;
}

#endif