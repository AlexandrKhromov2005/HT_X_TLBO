#ifndef IMAGE_PROCESSING_HPP
#include <string>
#include <vector>

class Image {
    public:
        std::vector<unsigned char> image_vec;
        
        std::vector<unsigned char> r_lay;
        std::vector<unsigned char> g_lay;
        std::vector<unsigned char> b_lay;  
        
        int size;

        const std::string filepath; 
        int width, height, channels;

        std::vector<unsigned char> import_image(const std::string& filepath);
        void export_image(const std::string& filepath);

        void pix_vec_to_layers();
        void layers_to_pix_vec();

        std::vector<unsigned char> hadamard_trans();
        std::vector<int> md5_coordinate_generation();
        std::vector<unsigned char> lay_to_block();
        std::vector<unsigned char> embed_wm();
        std::vector<unsigned char>read_wm();
};
#endif // IMAGE_PROCESSING_HPP