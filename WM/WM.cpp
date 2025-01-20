#include "WM.hpp"

void WM::POB() {

    auto processLayer = [&](std::vector<WMPixel>& layer, std::vector<unsigned char>& key) {
        for (size_t i = 0; i < size; ++i) {
            unsigned char lowBits = layer[i].lowBits;
            key[i] = R_LUT[lowBits];
            layer[i].lowBits = LOW_BITS_LUT[lowBits];
        }
    };

    std::thread threadRLay(processLayer, std::ref(RLay), std::ref(r_b_key));
    std::thread threadGLay(processLayer, std::ref(GLay), std::ref(g_b_key));
    std::thread threadBLay(processLayer, std::ref(BLay), std::ref(b_b_key));

    threadRLay.join();
    threadGLay.join();
    threadBLay.join();
}