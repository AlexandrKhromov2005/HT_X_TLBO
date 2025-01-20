#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"
#include "lib/stb_image.h"
#include "image_processing.hpp"
#include <vector>
#include <iostream>

std::vector<unsigned char> Image::import_image(const std::string& filepath){
    unsigned char* data = stbi_load(filepath.c_str(), &this->width, &this->height, &this->channels, 0);

    // Копируем данные в вектор
    std::vector<unsigned char> image_vec(data, data + width * height * channels);

    // Освобождаем память, выделенную stb_image
    stbi_image_free(data);

    this->image_vec = image_vec;

    return image_vec;
}

void Image::export_image(const std::string& filepath){
    stbi_write_png(filepath.c_str(), width, height, channels, image_vec.data(), width * channels);
}

void Image::pix_vec_to_layers(){
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

void Image::layers_to_pix_vec(){
    image_vec.clear();

    for(int i = 0; i < r_lay.size(); i++){
        image_vec.push_back(r_lay[i]);
        image_vec.push_back(g_lay[i]);
        image_vec.push_back(b_lay[i]);
    }
}

int main(){
    Image goida;
    
    goida.import_image("goida.png");

    goida.pix_vec_to_layers();   
    goida.layers_to_pix_vec();

    std::cout << "len of pix vec " << goida.image_vec.size() << "\n";
    std::cout << "len of lay vec " << goida.r_lay.size() << "\n";


    goida.export_image("new_goida.png");
}