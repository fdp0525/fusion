#include "fusion.h"

int main(){
    fusion_t f;

    min_params_t ps;
    ps.eta = 0.1f;
    ps.omega_k = 0.5f;
    ps.omega_s = 0.5f;
    ps.gamma = 0.1f;
    ps.epsilon = 0.00005f;
    ps.threshold = 0.1f;
    ps.mode = fusion_mode::CPU_MULTITHREAD;
    ps.camera_fx = 525;
    ps.camera_fy = 525;
    ps.voxel_length = 2; 
 
    f.fusion(&ps);
    
    return 0;
}
