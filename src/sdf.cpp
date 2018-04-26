#include "sdf.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "matrix.h"
#include "canon_sdf.h"
#include <ctpl_stl.h>

const point_t sdf_t::size = point_t(80);

sdf_t::sdf_t(depth_map_t depths, bool is_multi){
    this->depths = depths;
    this->is_multi = is_multi;
    
    for (int x = 0; x * l < size.get(0); x++){
	deform_field.push_back(std::vector<std::vector<point_t>>());

        for (int y = 0; y * l < size.get(1); y++){
            deform_field[x].push_back(std::vector<point_t>());

            for (int z = 0; z * l < size.get(2); z++){
                deform_field[x][y].push_back(point_t());
            }
        }
    }

    phi = new function_t<float>([=](point_t p){
        return distance(p);
    });

    psi = new function_t<point_t>([=](point_t p){
        return deformation_at(p);
    });

    psi_u = new function_t<float>([=](point_t p){
        return deformation_at(p).get(0);
    });
    
    psi_v = new function_t<float>([=](point_t p){
        return deformation_at(p).get(1);
    });
    
    psi_w = new function_t<float>([=](point_t p){
        return deformation_at(p).get(2);
    });

    camera.fx = 525;
    camera.fy = 525;
    camera.cx = depths->size() / 2;
    camera.cy = depths->at(0).size() / 2;
}

sdf_t::~sdf_t(){
    delete depths;
    delete phi;
    delete psi;
    delete psi_u;
    delete psi_v;
    delete psi_w;
}

float
sdf_t::distance(point_t p){
    // deform
    point_t v = p + deformation_at(p);

    // project point
    float x;
    float y;
    project(v, &x, &y);

    // in case not in frame
    // TODO: not 100% sure this is the correct way to handle this case
    if (x < 0 || y < 0 || x >= depths->size() || y >= depths->at(0).size()){
	return 1;
    }

    // true signed distance
    float phi_true =  depths->at(x).at(y) - v.get(2);

    // divide by delta
    float d = phi_true / delta;
    
    // clamp to range [-1..1]
    return d / std::max(1.0f, std::abs(d));
}

void
sdf_t::project(point_t p, float * x, float * y){
    float epsilon = 0.00001f; //prevent division by zero
   
    // centre on origin 
    float rx = p.get(0) - size.get(0) / 2;
    float ry = p.get(1) - size.get(1) / 2;

    // perspective projection
    rx *= camera.fx / (p.get(2) + epsilon);
    ry *= camera.fy / (p.get(2) + epsilon);

    // re-centre in image
    *x = rx + camera.cx;
    *y = ry + camera.cy;
}

point_t
sdf_t::deformation_at(point_t p){
    for (int i = 0; i < 3; i++){
	if (p.get(i) < 0 || p.get(i) >= size.get(i)){
            return point_t();
	}
    }

    point_t v = p / l;
    int x = v.get(0);
    int y = v.get(1);
    int z = v.get(2);
    return deform_field[x][y][z];
}

point_t
sdf_t::distance_gradient(point_t p){
    return point_t(
        phi->differentiate(0)(p),
        phi->differentiate(1)(p),
        phi->differentiate(2)(p)
    );
}

point_t
sdf_t::voxel_centre(int x, int y, int z){
    return (point_t(x, y, z) + point_t(0.5f)) * l;
}

void 
sdf_t::fuse(canon_sdf_t * canon, sdf_t * previous, min_params_t * ps){
    // initialise deformation field to that of the previous frame
    for (int x = 0; x < deform_field.size(); x++){
        for (int y = 0; y < deform_field[0].size(); y++){
            for (int z = 0; z < deform_field[0][0].size(); z++){
		deform_field[x][y][z] = previous->deform_field[x][y][z];
	    }
	}
    }

    // rigid component
    bool should_update = true;
    for (int i = 0; should_update; i++){
	std::cout << "Rigid transformation, iteration " << i << "..." << std::endl;

        should_update = false;
        update_rigid(&should_update, canon, ps);
    }
    std::cout << "Rigid transformation converged." << std::endl;

    // non-rigid component
    should_update = true;
    for (int i = 0; should_update; i++){
        std::cout << "Non-rigid transformation, iteration " << i << "..." << std::endl;

        should_update = false;
        update_nonrigid(&should_update, canon, ps);
    }
    std::cout << "Non-rigid transformation converged." << std::endl;
}

void
sdf_t::update_rigid(bool * cont, canon_sdf_t * canon, min_params_t * ps){
    for (int x = 0; x < deform_field.size(); x++){
        for (int y = 0; y < deform_field[0].size(); y++){
            for (int z = 0; z < deform_field[0][0].size(); z++){
                point_t p = voxel_centre(x, y, z);

                point_t e = data_energy(p, canon);
		point_t u = e * ps->eta_rigid;

                if (u.length() > ps->threshold_rigid){
                    *cont = true;
                }

                deform_field[x][y][z] -= u;
	    }
	}
    }      
}

void
sdf_t::update_nonrigid(bool * cont, canon_sdf_t * canon, min_params_t * ps){
    std::vector<std::future<void>> futures;

    for (int x = 0; x < deform_field.size(); x++){
        for (int y = 0; y < deform_field[0].size(); y++){
            for (int z = 0; z < deform_field[0][0].size(); z++){
                point_t p = voxel_centre(x, y, z);
                
                point_t e = energy(p, canon, ps->omega_k, ps->omega_s, ps->gamma, ps->epsilon);
                point_t u = e * ps->eta_nonrigid;
        
                if (u.length() > ps->threshold_nonrigid){
                    *cont = true;
                }

                deform_field[x][y][z] -= u;
	    }
	}
    }      

    for (int i = 0; i < futures.size(); i++){
        futures[i].get();
    } 
}

point_t
sdf_t::energy(point_t v, canon_sdf_t * c, float o_k, float o_s, float gamma, float eps){
     //return 
     //    data_energy(p, c) +
     //    killing_energy(p, gamma) * o_k +
     //    level_set_energy(p, eps) * o_s;
     point_t d = data_energy(v, c);
     point_t k = killing_energy(v, gamma) * o_k;
     point_t l = level_set_energy(v, eps) * o_s;

     return d + k + l;
}

point_t
sdf_t::data_energy(point_t p, canon_sdf_t * canon){
    return distance_gradient(p) * (distance(p) - canon->distance(p));
}

point_t
sdf_t::level_set_energy(point_t p, float epsilon){
    matrix_t h  = matrix_t::hessian(*phi, p);
    point_t g   = distance_gradient(p);
    float alpha = (g.length() - 1) / (g.length() + epsilon);

    return h * g * alpha;
}

point_t
sdf_t::killing_energy(point_t p, float gamma){
    //FIXME: i think this might be broken since it always returns the same value	
    matrix_t j = matrix_t::jacobian(*psi, p);
    std::vector<float> j_v = j.stack();
    std::vector<float> jt_v = j.transpose().stack();

    std::vector<float> v;
    for (int i = 0; i < j_v.size(); i++){
	v.push_back(jt_v[i] + j_v[i] * gamma);
    }

    matrix_t h_u = matrix_t::hessian(*psi_u, p);
    matrix_t h_v = matrix_t::hessian(*psi_v, p);
    matrix_t h_w = matrix_t::hessian(*psi_w, p);
    matrix_t h[3] = { h_u, h_v, h_w };    
    
    point_t result;
    for (int i = 0; i < 9; i++){
	result += point_t(
            h[i / 3].get(i % 3, 0) * v[i],
            h[i / 3].get(i % 3, 1) * v[i],
            h[i / 3].get(i % 3, 2) * v[i]
	);
    }

    return result * 2;
}
