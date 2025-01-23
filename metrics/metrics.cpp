#include "metrics.hpp"
#include <omp.h>

double image_mse(const std::vector<unsigned char>& old_img, const std::vector<unsigned char>& new_img) {
    const size_t total_pixels = old_img.size();
    const size_t simd_step = 32; 
    __m256d sum = _mm256_setzero_pd();
    size_t i = 0;

    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i <= total_pixels - simd_step; i += simd_step) {
        // Загрузка 32 байт
        __m256i old_data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&old_img[i]));
        __m256i new_data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&new_img[i]));

        // Разность пикселей
        __m256i diff = _mm256_sub_epi8(old_data, new_data);

        // Возведение в квадрат (8-бит -> 16-бит)
        __m256i square = _mm256_maddubs_epi16(diff, diff);

        // Разделение на 128-битные части
        __m128i square_low = _mm256_extracti128_si256(square, 0);
        __m128i square_high = _mm256_extracti128_si256(square, 1);

        // Конвертация 16-бит -> 32-бит
        __m128i square_low_32 = _mm_cvtepi16_epi32(square_low);  
        __m128i square_high_32 = _mm_cvtepi16_epi32(square_high);

        // Конвертация 32-битных int -> double
        __m256d square_low_dbl = _mm256_cvtepi32_pd(square_low_32);
        __m256d square_high_dbl = _mm256_cvtepi32_pd(square_high_32);

        // Суммирование
        sum = _mm256_add_pd(sum, square_low_dbl);
        sum = _mm256_add_pd(sum, square_high_dbl);
    }

    // Обработка оставшихся пикселей
    double tail_sum = 0.0;
    for (; i < total_pixels; ++i) {
        double diff = static_cast<double>(old_img[i]) - static_cast<double>(new_img[i]);
        tail_sum += diff * diff;
    }

    // Суммирование всех компонент
    double tmp[4];
    _mm256_storeu_pd(tmp, sum);
    double total = tmp[0] + tmp[1] + tmp[2] + tmp[3] + tail_sum;

    return total / total_pixels;
}

inline double image_psnr(const std::vector<unsigned char>& old_img, const std::vector<unsigned char>& new_img) {
    const double mse = image_mse(old_img, new_img);
    
    if (mse <= 0.0) {
        return std::numeric_limits<double>::infinity(); // Бесконечность, если изображения идентичны
    }
    
    constexpr double max_pixel_value = 255.0;
    return 10.0 * std::log10((max_pixel_value * max_pixel_value) / mse);
}