#ifndef WM_HPP
#define WM_HPP

#include <vector>
#include <string>
#include <thread>

constexpr unsigned char B_KEY_LUT[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
constexpr unsigned char LOW_BITS_LUT[16] = {0, 1, 2, 0, 3, 1, 2, 0, 4, 3, 4, 1, 5, 2, 3, 0};

struct WMPixel {
    unsigned char highBits;
    unsigned char lowBits;
};

class WM {
    private:
        std::vector<WMPixel> RLay;
        std::vector<WMPixel> GLay;
        std::vector<WMPixel> BLay;
        size_t size;
        unsigned char a_key[6];
        std::vector<unsigned char> r_b_key;
        std::vector<unsigned char> g_b_key;
        std::vector<unsigned char> b_b_key;

    public:
        WM(std::string &filename);
        void affineTransformation();
        void POB();
};

#endif // WM_HPP
