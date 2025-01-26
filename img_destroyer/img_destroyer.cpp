#include "img_destroyer.hpp"
#include <algorithm>
#include <stdexcept>

// Конструктор
ImageDestroyer::ImageDestroyer(const std::string& input_file) {
    readImage(input_file);
    splitIntoBlocks(); // Автоматическое разбиение на блоки при загрузке
}

// Деструктор
ImageDestroyer::~ImageDestroyer() {
    stbi_image_free(m_image_data);
}

// Чтение изображения
void ImageDestroyer::readImage(const std::string& filename) {
    m_image_data = stbi_load(
        filename.c_str(),
        &m_width,
        &m_height,
        &m_channels,
        0
    );

    if (!m_image_data) {
        throw std::runtime_error("Failed to load image: " + filename);
    }

    if (m_channels != 3) {
        stbi_image_free(m_image_data);
        throw std::runtime_error("Only 3-channel RGB images are supported");
    }
}

// Сохранение изображения
void ImageDestroyer::save(const std::string& output_file, int quality) const {
    writeImage(output_file, quality);
}

// Запись изображения
void ImageDestroyer::writeImage(const std::string& filename, int quality) const {
    const bool success = stbi_write_jpg(
        filename.c_str(),
        m_width,
        m_height,
        m_channels,
        m_image_data,
        quality
    );

    if (!success) {
        throw std::runtime_error("Failed to write image: " + filename);
    }
}

// Корректировка яркости (теперь работает с блоками)
void ImageDestroyer::adjustBrightness(float factor) {
    for (auto& block : m_rgb_blocks) {
        for (auto& row : block) {
            for (auto& pixel : row) {
                pixel.r = std::clamp(static_cast<int>(pixel.r * factor), 0, 255);
                pixel.g = std::clamp(static_cast<int>(pixel.g * factor), 0, 255);
                pixel.b = std::clamp(static_cast<int>(pixel.b * factor), 0, 255);
            }
        }
    }
    mergeFromBlocks(m_rgb_blocks); // Обновляем m_image_data
}

// Разбиение изображения на блоки 8x8
void ImageDestroyer::splitIntoBlocks() {
    const int block_rows = (m_height + 7) / 8;
    const int block_cols = (m_width + 7) / 8;
    m_rgb_blocks.resize(block_rows * block_cols);

    for (int by = 0; by < block_rows; ++by) {
        for (int bx = 0; bx < block_cols; ++bx) {
            Block8x8<Pixel>& block = m_rgb_blocks[by * block_cols + bx];
            for (int y = 0; y < 8; ++y) {
                for (int x = 0; x < 8; ++x) {
                    const int src_y = std::min(by * 8 + y, m_height - 1);
                    const int src_x = std::min(bx * 8 + x, m_width - 1);
                    const int index = (src_y * m_width + src_x) * m_channels;
                    block[y][x] = Pixel(
                        m_image_data[index],
                        m_image_data[index + 1],
                        m_image_data[index + 2]
                    );
                }
            }
        }
    }
}

// Сборка изображения из блоков
void ImageDestroyer::mergeFromBlocks(const std::vector<Block8x8<Pixel>>& blocks) {
    const size_t required_size = m_width * m_height * m_channels;
    if (m_image_data) {
        stbi_image_free(m_image_data);
        m_image_data = nullptr;
    }

    m_image_data = static_cast<unsigned char*>(malloc(required_size));
    if (!m_image_data) {
        throw std::runtime_error("Memory allocation failed");
    }

    const int block_rows = (m_height + 7) / 8;
    const int block_cols = (m_width + 7) / 8;

    for (int by = 0; by < block_rows; ++by) {
        for (int bx = 0; bx < block_cols; ++bx) {
            const Block8x8<Pixel>& block = blocks[by * block_cols + bx];
            for (int y = 0; y < 8; ++y) {
                for (int x = 0; x < 8; ++x) {
                    const int dst_y = by * 8 + y;
                    const int dst_x = bx * 8 + x;
                    if (dst_y >= m_height || dst_x >= m_width) continue;
                    const int index = (dst_y * m_width + dst_x) * m_channels;
                    m_image_data[index]     = block[y][x].r;
                    m_image_data[index + 1] = block[y][x].g;
                    m_image_data[index + 2] = block[y][x].b;
                }
            }
        }
    }
}

// Преобразование RGB → YCbCr (работает с блоками)
void ImageDestroyer::convertToYCbCr() {
    m_ycbcr_blocks.resize(m_rgb_blocks.size());
    for (size_t i = 0; i < m_rgb_blocks.size(); ++i) {
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                const Pixel& p = m_rgb_blocks[i][y][x];
                YCbCrPixel ycbcr;
                ycbcr.Y  = 0.299 * p.r + 0.587 * p.g + 0.114 * p.b;
                ycbcr.Cb = 128 + (-0.168736 * p.r - 0.331264 * p.g + 0.5 * p.b);
                ycbcr.Cr = 128 + (0.5 * p.r - 0.418688 * p.g - 0.081312 * p.b);
                m_ycbcr_blocks[i][y][x] = ycbcr;
            }
        }
    }
}

// Преобразование YCbCr → RGB (работает с блоками)
void ImageDestroyer::convertToRGB() {
    if (m_ycbcr_blocks.empty()) {
        throw std::runtime_error("YCbCr blocks are empty");
    }

    for (size_t i = 0; i < m_ycbcr_blocks.size(); ++i) {
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                const YCbCrPixel& ycbcr = m_ycbcr_blocks[i][y][x];
                double r = ycbcr.Y + 1.402 * (ycbcr.Cr - 128);
                double g = ycbcr.Y - 0.344136 * (ycbcr.Cb - 128) - 0.714136 * (ycbcr.Cr - 128);
                double b = ycbcr.Y + 1.772 * (ycbcr.Cb - 128);
                m_rgb_blocks[i][y][x] = Pixel(
                    static_cast<unsigned char>(std::clamp(r, 0.0, 255.0)),
                    static_cast<unsigned char>(std::clamp(g, 0.0, 255.0)),
                    static_cast<unsigned char>(std::clamp(b, 0.0, 255.0))
                );
            }
        }
    }
    mergeFromBlocks(m_rgb_blocks); // Обновляем изображение
}

//Субдискретизация 4:2:0
void ImageDestroyer::subsampling(){
    for(Block8x8<YCbCrPixel> block : m_ycbcr_blocks){
        for(size_t i = 0; i < 8; i+=2){
            for(size_t j = 0; j < 8; j+=2){
                double avg_cr = block[i][j].Cr + block[i+1][j].Cr + block[i][j+1].Cr + block[i+1][j+1].Cr;
                double avg_cb = block[i][j].Cb + block[i+1][j].Cb + block[i][j+1].Cb + block[i+1][j+1].Cb;
                avg_cr /= 4.0;
                avg_cb /= 4.0;
                block[i][j].Cr = avg_cr ;
                block[i+1][j].Cr = avg_cr;
                block[i][j+1].Cr = avg_cr;
                block[i+1][j+1].Cr = avg_cr;
                block[i][j].Cb = avg_cb;
                block[i+1][j].Cb = avg_cb;
                block[i][j+1].Cb = avg_cb;
                block[i+1][j+1].Cb = avg_cb;
            }
        }
    }
}

