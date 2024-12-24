#ifndef SSNP_MODEL_H
#define SSNP_MODEL_H

#include <vector>

std::vector<float> scatter_factor(const std::vector<float>& n, float res_z = 0.1, float dz = 1.0, float n0 = 1.0);

#endif