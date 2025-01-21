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

void WM::revPOB() {

    auto restoreLayer = [&](std::vector<WMPixel>& layer, const std::vector<unsigned char>& key) {
        for (size_t i = 0; i < size; ++i) {
            unsigned char keyValue = key[i]; // Значение ключа (r_b_key, g_b_key, b_b_key)
            unsigned char compressedLowBits = layer[i].lowBits; // Сжатое значение lowBits

            if (keyValue < 5 && compressedLowBits < sizeof(REV_LOW_BITS_LUT[keyValue]) / sizeof(REV_LOW_BITS_LUT[keyValue][0])) {
                layer[i].lowBits = REV_LOW_BITS_LUT[keyValue][compressedLowBits];
            } else {
                layer[i].lowBits = 0; 
            }
        }
    };

    std::thread threadRLay(restoreLayer, std::ref(RLay), std::ref(r_b_key));
    std::thread threadGLay(restoreLayer, std::ref(GLay), std::ref(g_b_key));
    std::thread threadBLay(restoreLayer, std::ref(BLay), std::ref(b_b_key));

    threadRLay.join();
    threadGLay.join();
    threadBLay.join();
}