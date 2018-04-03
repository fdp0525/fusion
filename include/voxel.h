#ifndef VOXEL_H
#define VOXEL_H

struct min_params_t {
    // learning rate
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
};

class sdf_t;

class voxel_t {
public:
    voxel_t(sdf_t * sdf, point_t p);

private:
    sdf_t * sdf;
    point_t p;
    point_t u;

    bool update(sdf_t * canon, min_params_t * ps);

    // energy functions used for gradient descent
    point_t energy_gradient(float omega_k, float omega_s, float gamma, float epsilon);
    point_t data_gradient(sdf_t * canon);
    point_t killing_gradient(float gamma);
    point_t level_set_gradient(float epsilon);
};

#endif