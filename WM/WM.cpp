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

void WM::AffineTransformation() {
    auto transformLayer = [this](std::vector<unsigned char>& layer) {
        std::vector<unsigned char> new_layer;
        new_layer.resize(layer.size());

        for (size_t i = 0; i < layer.size(); ++i) {
            size_t x = i % width;
            size_t y = i / width;
            size_t new_x = (a_key[0] * x + a_key[1] * y + a_key[4]) % width;
            size_t new_y = (a_key[2] * x + a_key[3] * y + a_key[5]) % height;
            size_t new_index = new_x + new_y * width;  
            
            if (new_index < new_layer.size()) {
                new_layer[new_index] = layer[i];
            }
        }
        
        layer = std::move(new_layer);
    };

    std::thread threadR(transformLayer, std::ref(r_lay));
    std::thread threadG(transformLayer, std::ref(g_lay));
    std::thread threadB(transformLayer, std::ref(b_lay));

    threadR.join();
    threadG.join();
    threadB.join();
}


void WM::revAffineTransformation() {
    auto inverseTransformLayer = [this](std::vector<unsigned char>& layer) {
        std::vector<unsigned char> original_layer;
        original_layer.resize(layer.size());

        int det = (a_key[0] * a_key[3] - a_key[1] * a_key[2]) % width;
        if (det == 0) {
            throw std::runtime_error("Matrix is not invertible");
        }

        int det_inv = -1;
        for (int i = 1; i < width; ++i) {
            if ((det * i) % width == 1) {
                det_inv = i;
                break;
            }
        }
        if (det_inv == -1) {
            throw std::runtime_error("Determinant has no inverse modulo width");
        }

        int inv_a0 = (a_key[3] * det_inv) % width;
        int inv_a1 = (-a_key[1] * det_inv) % width;
        int inv_a2 = (-a_key[2] * det_inv) % width;
        int inv_a3 = (a_key[0] * det_inv) % width;

        for (size_t new_index = 0; new_index < layer.size(); ++new_index) {
            size_t new_x = new_index % width;
            size_t new_y = new_index / width;

            int adjusted_x = (new_x - a_key[4]) % width;
            int adjusted_y = (new_y - a_key[5]) % height;

            int x = (inv_a0 * adjusted_x + inv_a1 * adjusted_y) % width;
            int y = (inv_a2 * adjusted_x + inv_a3 * adjusted_y) % height;

            if (x < 0) x += width;
            if (y < 0) y += height;

            size_t original_index = x + y * width;

            if (original_index < original_layer.size()) {
                original_layer[original_index] = layer[new_index];
            }
        }

        layer = std::move(original_layer);
    };

    std::thread threadR(inverseTransformLayer, std::ref(r_lay));
    std::thread threadG(inverseTransformLayer, std::ref(g_lay));
    std::thread threadB(inverseTransformLayer, std::ref(b_lay));

    threadR.join();
    threadG.join();
    threadB.join();
}