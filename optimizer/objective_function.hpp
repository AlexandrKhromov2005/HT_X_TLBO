#ifndef OBJECTIVE_FUNCTION_HPP
#define OBJECTIVE_FUNCTION_HPP

#include "metrics/metrics.hpp"

struct PFM { 
    Image src_image;                 
    WM src_wm;                       
    std::vector<Image> attacked;     
    std::vector<WM> extracted_wms;   
    std::vector<double> attack_weights;   
};

class Optimizer {
public:
    std::vector<PFM> packs; 
    double calculateObjectiveFunction();
};
#endif // OBJECTIVE_FUNCTION_HPP
