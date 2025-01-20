#include "WM.hpp"

void WM::POB() {
    constexpr unsigned char b_key_lut[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
    constexpr unsigned char lowBits_lut[16] = {0, 1, 2, 0, 3, 1, 2, 0, 4, 3, 4, 1, 5, 2, 3, 0};

    auto processLayer = [&](std::vector<WMPixel>& layer, std::vector<unsigned char>& key) {
        for (size_t i = 0; i < size; ++i) {
            unsigned char lowBits = layer[i].lowBits;
            key[i] = b_key_lut[lowBits];
            layer[i].lowBits = lowBits_lut[lowBits];
        }
    };

    std::thread threadRLay(processLayer, std::ref(RLay), std::ref(r_b_key));
    std::thread threadGLay(processLayer, std::ref(GLay), std::ref(g_b_key));
    std::thread threadBLay(processLayer, std::ref(BLay), std::ref(b_b_key));

    threadRLay.join();
    threadGLay.join();
    threadBLay.join();
}