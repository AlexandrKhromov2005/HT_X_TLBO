#include "image_processing.hpp"

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
/*int main(){
    Image goida;
    
    goida.import_image("goida.png");

    goida.pix_vec_to_layers();   
    goida.layers_to_pix_vec();

    std::cout << "len of pix vec " << goida.image_vec.size() << "\n";
    std::cout << "len of lay vec " << goida.r_lay.size() << "\n";


    goida.export_image("new_goida.png");
}*/