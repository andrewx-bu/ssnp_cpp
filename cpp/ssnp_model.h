#ifndef SSNP_MODEL_H
#define SSNP_MODEL_H

#include <vector>
#include <complex>

std::vector<float> scatter_factor(const std::vector<float>& n, float res_z = 0.1, float dz = 1.0, float n0 = 1.0);

std::vector<std::vector<std::complex<float>>> c_gamma(const std::vector<float>& res, const std::vector<int>& shape);

std::pair<std::vector<std::vector<std::vector<std::complex<double>>>>, std::vector<std::vector<std::vector<std::complex<double>>>>>
diffract(
    const std::vector<std::vector<std::complex<double>>>& uf,
    const std::vector<std::vector<std::complex<double>>>& ub,
    const std::vector<float>& res = {0.1f, 0.1f, 0.1f},
    float dz = 1);

// Returns flattened Tensor
std::vector<std::vector<bool>> binary_pupil(
    const std::vector<int>& shape, 
    float na, 
    const std::vector<float>& res = {0.1f, 0.1f, 0.1f});

std::vector<std::vector<std::vector<std::complex<double>>>> tilt(
    const std::vector<int>& shape,
    const std::vector<double>& angles,
    double NA = 0.65,
    const std::vector<double>& res = {0.1f, 0.1f, 0.1f},
    bool trunc = true);

#endif
