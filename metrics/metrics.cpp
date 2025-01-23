#include "metrics.hpp"

double channel_mse(const std::vector<unsigned char>& old_img, const std::vector<unsigned char>& new_img) {
    const size_t total_pixels = old_img.size();
    const size_t simd_step = 32;
    __m256d sum = _mm256_setzero_pd();
    size_t i = 0;

    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i <= total_pixels - simd_step; i += simd_step) {
        // Загрузка 32 байт
        __m256i old_data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&old_img[i]));
        __m256i new_data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&new_img[i]));

        // Вычисление разности
        __m256i diff = _mm256_sub_epi8(old_data, new_data);

        // Возведение в квадрат (8-бит -> 16-бит)
        __m256i square = _mm256_maddubs_epi16(diff, diff);

        // Разделение на 128-битные части
        __m128i square_low = _mm256_extracti128_si256(square, 0);
        __m128i square_high = _mm256_extracti128_si256(square, 1);

        // Конвертация 16-бит -> 32-бит
        __m128i square_low_32 = _mm_cvtepi16_epi32(square_low);
        __m128i square_high_32 = _mm_cvtepi16_epi32(square_high);

        // Конвертация в double и суммирование
        __m256d square_low_dbl = _mm256_cvtepi32_pd(square_low_32);
        __m256d square_high_dbl = _mm256_cvtepi32_pd(square_high_32);

        sum = _mm256_add_pd(sum, square_low_dbl);
        sum = _mm256_add_pd(sum, square_high_dbl);
    }

    // Обработка оставшихся пикселей
    double tail_sum = 0.0;
    for (; i < total_pixels; ++i) {
        double diff = static_cast<double>(old_img[i]) - static_cast<double>(new_img[i]);
        tail_sum += diff * diff;
    }

    // Суммирование результатов
    double tmp[4];
    _mm256_storeu_pd(tmp, sum);
    return (tmp[0] + tmp[1] + tmp[2] + tmp[3] + tail_sum) / total_pixels;
}

double image_mse(const Image& original, const Image& distorted) {
    // Устанавливаем количество потоков для OpenMP внутри channel_mse_optimized
    omp_set_num_threads(2); // 2 потока на канал

    // Запускаем асинхронные задачи для каждого канала
    auto future_r = std::async(std::launch::async, channel_mse, 
                              std::ref(original.r_lay), std::ref(distorted.r_lay));
    auto future_g = std::async(std::launch::async, channel_mse, 
                              std::ref(original.g_lay), std::ref(distorted.g_lay));
    auto future_b = std::async(std::launch::async, channel_mse, 
                              std::ref(original.b_lay), std::ref(distorted.b_lay));

    // Дожидаемся результатов и усредняем
    const double mse_r = future_r.get();
    const double mse_g = future_g.get();
    const double mse_b = future_b.get();

    return (mse_r + mse_g + mse_b) / 3.0;
}

double image_psnr(const Image& original, const Image& distorted) {
    const double mse = image_mse(original, distorted);
    return (mse <= 0.0) 
        ? std::numeric_limits<double>::infinity()
        : 10.0 * std::log10(65025.0 / mse); 
}

double channel_nc(const std::vector<unsigned char>& orig, const std::vector<unsigned char>& extr) {
    double sum_prod = 0.0;
    double sum_sq_orig = 0.0;
    double sum_sq_extr = 0.0;

    #pragma omp parallel for reduction(+:sum_prod, sum_sq_orig, sum_sq_extr)
    for (size_t i = 0; i < orig.size(); ++i) {
        const double o = orig[i];
        const double e = extr[i];
        
        sum_prod += o * e;
        sum_sq_orig += o * o;
        sum_sq_extr += e * e;
    }

    const double denominator = sqrt(sum_sq_orig * sum_sq_extr);
    return (denominator > 1e-9) ? (sum_prod / denominator) : 0.0;
}

double image_nc(const WM& original_wm, const WM& extracted_wm) {
    auto future_r = std::async(std::launch::async, channel_nc, 
                             std::ref(original_wm.r_lay), std::ref(extracted_wm.r_lay));
    auto future_g = std::async(std::launch::async, channel_nc,
                             std::ref(original_wm.g_lay), std::ref(extracted_wm.g_lay));
    auto future_b = std::async(std::launch::async, channel_nc,
                             std::ref(original_wm.b_lay), std::ref(extracted_wm.b_lay));

    return (future_r.get() + future_g.get() + future_b.get()) / 3.0;
}