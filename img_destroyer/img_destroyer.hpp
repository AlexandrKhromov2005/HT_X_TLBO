#ifndef IMG_DESTROYER_HPP
#define IMG_DESTROYER_HPP

#include "lib/stb_image.h"
#include "lib/stb_image_write.h"
#include <vector>
#include <array>
#include <string>
#include <stdexcept>

// Структуры пикселей
struct Pixel {
    unsigned char r, g, b;
    Pixel() : r(0), g(0), b(0) {}
    Pixel(unsigned char red, unsigned char green, unsigned char blue) 
        : r(red), g(green), b(blue) {}
};

struct YCbCrPixel {
    double Y, Cb, Cr;
    YCbCrPixel() : Y(0), Cb(0), Cr(0) {}
    YCbCrPixel(double y, double cb, double cr) : Y(y), Cb(cb), Cr(cr) {}
};

// Блок 8x8
template <typename T>
using Block8x8 = std::array<std::array<T, 8>, 8>;

class ImageDestroyer {
private:
    unsigned char* m_image_data = nullptr;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
    
    std::vector<Block8x8<Pixel>> m_rgb_blocks;    // Блоки RGB
    std::vector<Block8x8<YCbCrPixel>> m_ycbcr_blocks; // Блоки YCbCr

    void readImage(const std::string& filename);
    void writeImage(const std::string& filename, int quality) const;
    void pixelsToImage(const std::vector<Block8x8<Pixel>>& blocks);

    // Вспомогательные функции для работы с блоками
    void splitIntoBlocks();
    void mergeFromBlocks(const std::vector<Block8x8<Pixel>>& blocks);

    //Субдискретизация 4:2:0
    void subsampling();

public:
    explicit ImageDestroyer(const std::string& input_file);
    ~ImageDestroyer();

    // Основные методы
    void adjustBrightness(float factor);
    void save(const std::string& output_file, int quality = 85) const;

    // Преобразования
    void convertToYCbCr();    // RGB → YCbCr
    void convertToRGB();      // YCbCr → RGB

    // Геттеры
    const auto& getRGBBlocks() const { return m_rgb_blocks; }
    const auto& getYCbCrBlocks() const { return m_ycbcr_blocks; }

    // Запрет копирования
    ImageDestroyer(const ImageDestroyer&) = delete;
    ImageDestroyer& operator=(const ImageDestroyer&) = delete;

    // jpeg атаки с quality factor 70 80 90
    void jpeg_attack(const int qf);
};

#endif // IMG_DESTROYER_HPP