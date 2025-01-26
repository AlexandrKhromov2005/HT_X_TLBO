#ifndef DCT_HPP
#define DCT_HPP

#include <array>

template <typename T>
using Block8x8 = std::array<std::array<T, 8>, 8>;

namespace DCT {
    void forwardDCT(Block8x8<double>& block);
    void inverseDCT(Block8x8<double>& block);
}

#endif // DCT_HPP