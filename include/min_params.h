#ifndef MIN_PARAMS_H
#define MIN_PARAMS_H

#include <string>
#include <glm/glm.hpp>

typedef glm::vec3 point_t;

struct min_params_t {
    // learning rates for rigid and non-rigid alignment
    float eta;

    // relative weighting of killing condition
    float omega_k;

    // relative weighting of level set condition
    float omega_s;

    // killing condition purity
    float gamma;

    // prevention of division by zero in level set gradient
    float epsilon;

    // threshold for terminating registration in mm
    float threshold;

    bool is_multithreaded;
    int frames;

    int max_iterations;

    point_t size;
    float voxel_length;

    float delta;
    float sdf_eta;

    float near_clip;

    std::string dataset;
};

#endif
