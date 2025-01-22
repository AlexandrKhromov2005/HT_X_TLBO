#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"
#include "lib/stb_image.h"
#include "lib/md5.hpp"
#include "image_processing.hpp"
#include <vector>
#include <iostream>

std::vector<unsigned char> Image::import_image(const std::string& filepath) {
    unsigned char* data = stbi_load(filepath.c_str(), &this->width, &this->height, &this->channels, 0);

    // Копируем данные в вектор
    std::vector<unsigned char> image_vec(data, data + width * height * channels);

    // Освобождаем память, выделенную stb_image
    stbi_image_free(data);

    this->image_vec = image_vec;

    return image_vec;
}

void Image::export_image(const std::string& filepath) {
    stbi_write_png(filepath.c_str(), width, height, channels, image_vec.data(), width * channels);
}

void Image::pix_vec_to_layers() {
    // Очищаем слои перед заполнением
    this->r_lay.clear();
    this->g_lay.clear();
    this->b_lay.clear();

    // Проходим по вектору и разделяем на каналы
    for (size_t i = 0; i < this->image_vec.size(); i += 3) {
        this->r_lay.push_back(this->image_vec[i]);     // Красный канал
        this->g_lay.push_back(this->image_vec[i + 1]); // Зеленый канал
        this->b_lay.push_back(this->image_vec[i + 2]); // Синий канал
    }
}

void Image::layers_to_pix_vec() {
    image_vec.clear();

    for (int i = 0; i < r_lay.size(); i++) {
        image_vec.push_back(r_lay[i]);
        image_vec.push_back(g_lay[i]);
        image_vec.push_back(b_lay[i]);
    }
}

void Image::process_channel_to_blocks(
    const std::vector<unsigned char>& channel,
    std::vector<Block>& channel_blocks
) {
    const size_t blocks_x = width / 4;
    const size_t blocks_y = height / 4;
    const size_t total_blocks = blocks_x * blocks_y;

    channel_blocks.resize(total_blocks);

    for (size_t i = 0; i < channel.size(); ++i) {
        size_t x_global = i % width;
        size_t y_global = i / width;
        size_t x_in_block = x_global % 4;
        size_t y_in_block = y_global % 4;
        size_t block_x = x_global / 4;
        size_t block_y = y_global / 4;
        size_t n = block_y * blocks_x + block_x;

        channel_blocks[n][y_in_block][x_in_block] = channel[i];
    }
}

void Image::lay_to_blocks() {
    std::thread thread_r([this]() { 
        process_channel_to_blocks(r_lay, r_lay_blocks); 
    });
    
    std::thread thread_g([this]() { 
        process_channel_to_blocks(g_lay, g_lay_blocks); 
    });
    
    std::thread thread_b([this]() { 
        process_channel_to_blocks(b_lay, b_lay_blocks); 
    });

    thread_r.join();
    thread_g.join();
    thread_b.join();
}

void Image::blocks_to_lay() {
    r_lay.clear();
    g_lay.clear();
    b_lay.clear();
    
    const size_t total_pixels = width * height;
    r_lay.resize(total_pixels);
    g_lay.resize(total_pixels);
    b_lay.resize(total_pixels);

    const size_t blocks_per_row = width / 4;
    const size_t blocks_per_col = height / 4;

    auto process_blocks_to_channel = [&](
        const std::vector<Block>& channel_blocks,
        std::vector<unsigned char>& channel
    ) {
        for (size_t block_idx = 0; block_idx < channel_blocks.size(); ++block_idx) {
            const size_t block_y = block_idx / blocks_per_row;
            const size_t block_x = block_idx % blocks_per_row;

            for (size_t y_in_block = 0; y_in_block < 4; ++y_in_block) {
                for (size_t x_in_block = 0; x_in_block < 4; ++x_in_block) {
                    const size_t x_global = block_x * 4 + x_in_block;
                    const size_t y_global = block_y * 4 + y_in_block;
                    
                    const size_t pixel_idx = y_global * width + x_global;

                    channel[pixel_idx] = channel_blocks[block_idx][y_in_block][x_in_block];
                }
            }
        }
    };

    process_blocks_to_channel(r_lay_blocks, r_lay);
    process_blocks_to_channel(g_lay_blocks, g_lay);
    process_blocks_to_channel(b_lay_blocks, b_lay);
}

void Image::hadamard_trans() {
    std::array<std::array<double, 4>, 4> hadamard_matrix = {{
        {1,  1,  1,  1},
        {1, -1,  1, -1},
        {1,  1, -1, -1},
        {1, -1, -1,  1}
    }};

    for(int i = 0; i < this->r_lay_blocks.size(); i++){
        Block_hadamard temp_r;
        Block_hadamard temp_g;
        Block_hadamard temp_b;

        for(int j = 0; j < 4; j++){
            for(int k = 0; k < 4; k++){
                temp_r[j][k] = static_cast<double>(this->r_lay_blocks[i][j][k]);
                temp_g[j][k] = static_cast<double>(this->g_lay_blocks[i][j][k]);
                temp_b[j][k] = static_cast<double>(this->b_lay_blocks[i][j][k]);

            }
        }

        temp_r = multiply_matrices(hadamard_matrix, temp_r);
        temp_g = multiply_matrices(hadamard_matrix, temp_g);
        temp_b = multiply_matrices(hadamard_matrix, temp_b);


        r_hadam_blocks.push_back(temp_r);
        g_hadam_blocks.push_back(temp_g);
        b_hadam_blocks.push_back(temp_b);
    }
}

void Image::rev_hadamard_trans() {
    std::array<std::array<double, 4>, 4> hadamard_matrix = {{
        {0.25,  0.25,  0.25,  0.25},
        {0.25, -0.25,  0.25, -0.25},
        {0.25,  0.25, -0.25, -0.25},
        {0.25, -0.25, -0.25,  0.25}
    }};

    // Очищаем блоки перед заполнением
    r_lay_blocks.clear();
    g_lay_blocks.clear();
    b_lay_blocks.clear();

    for (int i = 0; i < this->r_hadam_blocks.size(); i++) {
        Block_hadamard temp_r;
        Block_hadamard temp_g;
        Block_hadamard temp_b;

        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                temp_r[j][k] = this->r_hadam_blocks[i][j][k];
                temp_g[j][k] = this->g_hadam_blocks[i][j][k];
                temp_b[j][k] = this->b_hadam_blocks[i][j][k];
            }
        }

        temp_r = multiply_matrices(hadamard_matrix, temp_r);
        temp_g = multiply_matrices(hadamard_matrix, temp_g);
        temp_b = multiply_matrices(hadamard_matrix, temp_b);

        Block temp_r_uchar;
        Block temp_g_uchar;
        Block temp_b_uchar;
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                temp_r_uchar[j][k] = static_cast<unsigned char>(temp_r[j][k]);
                temp_g_uchar[j][k] = static_cast<unsigned char>(temp_g[j][k]);
                temp_b_uchar[j][k] = static_cast<unsigned char>(temp_b[j][k]);
            }
        }

        r_lay_blocks.push_back(temp_r_uchar);
        g_lay_blocks.push_back(temp_g_uchar);
        b_lay_blocks.push_back(temp_b_uchar);
    }
}

// Функция для умножения двух матриц
std::array<std::array<double, 4>, 4> Image::multiply_matrices(const std::array<std::array<double, 4>, 4>& matrix1, const std::array<std::array<double, 4>, 4>& matrix2) {
    
    // Проверка на возможность умножения
    if (matrix1.empty() || matrix2.empty() || matrix1[0].size() != matrix2.size()) {
        std::cout << matrix1[0].size() << " --- " <<  matrix2.size() << "\n";
        for (int i = 0; i < matrix1.size(); i++){
            for (int j = 0; j < matrix1[0].size(); j++){
                std::cout << matrix1[i][j] << " ";
            }
            std::cout << "\n";
        }
        throw std::invalid_argument("Невозможно умножить матрицы: несовместимые размеры");
    }

    // Размеры результирующей матрицы
    size_t rows1 = matrix1.size();       // Количество строк в первой матрице
    size_t cols2 = matrix2[0].size();    // Количество столбцов во второй матрице
    size_t commonSize = matrix2.size();  // Общий размер (столбцы первой матрицы и строки второй)

    // Инициализация результирующей матрицы нулями
    std::array<std::array<double, 4>, 4> result = {{
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
    }};

    // Умножение матриц
    for (size_t i = 0; i < rows1; ++i) {
        for (size_t j = 0; j < cols2; ++j) {
            for (size_t k = 0; k < commonSize; ++k) {
                result[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }

    return result;
}


void Image::md5_coordinate_generation(){
    
    std::vector<uint16_t> result;
    uint8_t digest[16];

    for (size_t i = 0; i < this->r_lay_blocks.size(); ++i) {
        // Преобразуем блок в массив байт для вычисления хэша MD5
        uint8_t block_data[16];
        int index = 0;
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                block_data[index++] = this->r_lay_blocks[i][y][x];
            }
        }

        // Вычисляем хэш MD5 для текущего блока
        md5(block_data, 16, digest);

        // Проверяем условие для выбора блока (например, хэш начинается с 'a')
        if (digest[0] == 0xa || digest[0] == 0xb|| digest[0] == 0xc|| digest[0] == 0xd|| digest[0] == 0xe|| digest[0] == 0xf) {
            result.push_back(i);
        }
    }
    
    this->r_blocks_coordinates = result;
    result.clear();

    for (size_t i = 0; i < this->g_lay_blocks.size(); ++i) {
        // Преобразуем блок в массив байт для вычисления хэша MD5
        uint8_t block_data[16];
        int index = 0;
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                block_data[index++] = this->g_lay_blocks[i][y][x];
            }
        }

        // Вычисляем хэш MD5 для текущего блока
        md5(block_data, 16, digest);

        // Проверяем условие для выбора блока (например, хэш начинается с 'a')
        if (digest[0] == 0xa || digest[0] == 0xb|| digest[0] == 0xc|| digest[0] == 0xd|| digest[0] == 0xe|| digest[0] == 0xf) {
            result.push_back(i);
        }
    }
    
    this->g_blocks_coordinates = result;
    result.clear();
    
    for (size_t i = 0; i < this->b_lay_blocks.size(); ++i) {
        // Преобразуем блок в массив байт для вычисления хэша MD5
        uint8_t block_data[16];
        int index = 0;
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                block_data[index++] = this->b_lay_blocks[i][y][x];
            }
        }

        // Вычисляем хэш MD5 для текущего блока
        md5(block_data, 16, digest);

        // Проверяем условие для выбора блока (например, хэш начинается с 'a')
        if (digest[0] == 0xa || digest[0] == 0xb|| digest[0] == 0xc|| digest[0] == 0xd|| digest[0] == 0xe|| digest[0] == 0xf) {
            result.push_back(i);
        }
    }
    
    this->b_blocks_coordinates = result;
    result.clear();
}