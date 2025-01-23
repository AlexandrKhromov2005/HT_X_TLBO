#ifndef METRICS_HPP
#define METRICS_HPP

#include <vector>
#include <cmath>
#include <limits>
#include <immintrin.h>  // Для AVX2

// Оптимизированная версия с SIMD и OpenMP
double image_mse(const std::vector<unsigned char>& old_img, const std::vector<unsigned char>& new_img);
inline double image_psnr(const std::vector<unsigned char>& old_img, const std::vector<unsigned char>& new_img);

#endif // METRICS_HPP