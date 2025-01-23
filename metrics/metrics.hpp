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

#endif // METRICS_HPP