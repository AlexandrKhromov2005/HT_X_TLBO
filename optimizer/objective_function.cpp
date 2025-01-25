#include "objective_function.hpp"

double Optimizer::calculateObjectiveFunction() {
    double total_F = 0.0;
    const size_t n = packs.size();
    if (n == 0) return 0.0;

    #pragma omp parallel for reduction(+:total_F)
    for (size_t i = 0; i < n; ++i) {
        const PFM& pack = packs[i];
        const size_t m = pack.attack_weights.size();

        double psnr = image_psnr(pack.src_image, pack.src_image);
        double ssim = image_ssim(pack.src_image, pack.src_image);
        double nc = image_nc(pack.src_wm, pack.src_wm);
        double ber = image_ber(pack.src_wm, pack.src_wm);
        
        const double omega = (psnr / 100.0) * ssim * nc * (1 - ber);

        double sum_attacks = 0.0;
        for (size_t j = 0; j < m; ++j) {
            const Image& attacked_img = pack.attacked[j];
            const WM& extracted_wm = pack.extracted_wms[j];

            double nc_j = image_nc(pack.src_wm, extracted_wm);
            double ber_j = image_ber(pack.src_wm, extracted_wm);

            sum_attacks += pack.attack_weights[j] * nc_j * (1 - ber_j);
        }

        total_F += omega * sum_attacks;
    }

    return total_F / (n * packs[0].attack_weights.size()); 
}