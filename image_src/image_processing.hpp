#ifndef IMAGE_PROCESSING_HPP
#define IMAGE_PROCESSING_HPP
#include <string>
#include <vector>
#include <array>
#include <vector>
#include <iostream>
#include <thread>

using Block = std::array<std::array<unsigned char, 4>, 4>;
using Block_hadamard = std::array<std::array<double, 4>, 4>;

class Image {
public:
    std::vector<unsigned char> image_vec;
        
    std::vector<unsigned char> r_lay;
    std::vector<unsigned char> g_lay;
    std::vector<unsigned char> b_lay;
    
    std::vector<Block> r_lay_blocks;
    std::vector<Block> g_lay_blocks;
    std::vector<Block> b_lay_blocks;

    std::vector<Block_hadamard> r_hadam_blocks;
    std::vector<Block_hadamard> g_hadam_blocks;
    std::vector<Block_hadamard> b_hadam_blocks;

    std::vector<uint16_t> r_blocks_coordinates;
    std::vector<uint16_t> g_blocks_coordinates;
    std::vector<uint16_t> b_blocks_coordinates;

    int size;

    const std::string filepath; 
    int width, height, channels;

    std::vector<unsigned char> import_image(const std::string& filepath);
    void export_image(const std::string& filepath);

    void pix_vec_to_layers();
    void layers_to_pix_vec();

    void hadamard_trans();
    void rev_hadamard_trans();

    void md5_coordinate_generation();

    std::vector<unsigned char> embed_wm();
    std::vector<unsigned char>read_wm();

    void lay_to_blocks();
    void blocks_to_lay();

    std::array<std::array<double, 4>, 4> multiply_matrices(const std::array<std::array<double, 4>, 4>& matrix1, const std::array<std::array<double, 4>, 4>& matrix2);

private:
    void process_channel_to_blocks(
        const std::vector<unsigned char>& channel,
        std::vector<Block>& channel_blocks
    );
};
#endif // IMAGE_PROCESSING_HPP