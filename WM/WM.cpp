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

WM::WM() {
    RLay.resize(r_lay.size());
    GLay.resize(g_lay.size());
    BLay.resize(b_lay.size());

    auto processLayer = [](const std::vector<unsigned char>& layer, std::vector<WMPixel>& targetLayer) {
        for (size_t i = 0; i < layer.size(); ++i) {
            targetLayer[i].lowBits = layer[i] & 0b00001111;          
            targetLayer[i].highBits = (layer[i] & 0b11110000) >> 4; 
        }
    };

    std::thread threadRLay(processLayer, std::ref(r_lay), std::ref(RLay));
    std::thread threadGLay(processLayer, std::ref(g_lay), std::ref(GLay));
    std::thread threadBLay(processLayer, std::ref(b_lay), std::ref(BLay));

    threadRLay.join();
    threadGLay.join();
    threadBLay.join();
}

WM::~WM() {
    auto mergeLayer = [](const std::vector<WMPixel>& sourceLayer, std::vector<unsigned char>& targetLayer) {
        for (size_t i = 0; i < sourceLayer.size(); ++i) {
            targetLayer[i] = (sourceLayer[i].highBits << 4) | (sourceLayer[i].lowBits & 0b00001111);
        }
    };

    std::thread threadRLay(mergeLayer, std::ref(RLay), std::ref(r_lay));
    std::thread threadGLay(mergeLayer, std::ref(GLay), std::ref(g_lay));
    std::thread threadBLay(mergeLayer, std::ref(BLay), std::ref(b_lay));

    threadRLay.join();
    threadGLay.join();
    threadBLay.join();
}