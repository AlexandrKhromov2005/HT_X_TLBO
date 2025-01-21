#ifndef WM_HPP
#define WM_HPP

#include <vector>
#include <string>
#include <thread>
#include <utility>
#include <stdexcept>
#include "image_proc/image_processing.hpp"

constexpr unsigned char R_LUT[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
constexpr unsigned char LOW_BITS_LUT[16] = {0, 1, 2, 0, 3, 1, 2, 0, 4, 3, 4, 1, 5, 2, 3, 0};
    constexpr unsigned char REV_LOW_BITS_LUT[5][4] = {
        {0, 3, 7, 15},  // Для ключа = 0
        {1, 2, 4, 8},   // Для ключа = 1
        {5, 6, 9, 12},  // Для ключа = 2
        {10, 11, 13},   // Для ключа = 3
        {14}            // Для ключа = 4
    };

struct WMPixel  {
    unsigned char highBits;
    unsigned char lowBits;
};

class WM : public Image{
    private:
        std::vector<WMPixel> RLay;
        std::vector<WMPixel> GLay;
        std::vector<WMPixel> BLay;
        unsigned char a_key[6];
        std::vector<unsigned char> r_b_key;
        std::vector<unsigned char> g_b_key;
        std::vector<unsigned char> b_b_key;

    public:
        WM();
        ~WM();
        void AffineTransformation();
        void revAffineTransformation();
        void POB();
        void revPOB();
};

#endif // WM_HPP
