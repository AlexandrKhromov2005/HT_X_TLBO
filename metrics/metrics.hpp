#ifndef METRICS_HPP
#define METRICS_HPP

#include <vector>
#include <cmath>
#include <limits>
#include <immintrin.h>  // Для AVX2
#include "image_src/image_processing.hpp"
#include "WM/WM.hpp"
#include <omp.h>
#include <future>

double image_mse(const Image& original, const Image& distorted);
double image_psnr(const Image& original, const Image& distorted);
double image_nc(const WM& original_wm, const WM& extracted_wm);
double image_ber(const WM& original_wm, const WM& extracted_wm);
double image_ssim(const Image& original, const Image& distorted);
constexpr int WINDOW_SIZE = 8; // Фиксированный размер окна 8x8
constexpr double C1 = (0.01 * 255) * (0.01 * 255);
constexpr double C2 = (0.03 * 255) * (0.03 * 255);

#endif // METRICS_HPP