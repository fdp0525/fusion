#include "fusion.h"

#include "min_params.h"
#include <iostream>
#include "point.h"

#include "CImg.h"
using namespace cimg_library;

fusion_t::fusion_t(){
    
}

fusion_t::~fusion_t(){

}

sdf_t *
fusion_t::get_sdf(std::string filename){
    std::cout << "loading depth map: " << filename << std::endl; 

    CImg<unsigned char> image(filename.c_str());

    depth_map_t depths = new std::vector<std::vector<unsigned char>>(image.width());
    for (int x = 0; x < image.width(); x++){
        depths->push_back(std::vector<unsigned char>(image.height()));
        
        for (int y = 0; y < image.height(); y++){
            depths->at(x).push_back(*image.data(x, y, 0, 0));
        }
    }

    sdf_t * sdf = new sdf_t(depths, point_t(), 1);

    return sdf; 
}

void
fusion_t::load_filenames(std::vector<std::string> * fns){
    for (int i = 0; i < 551; i++){
        std::string padding = i < 10 ? "0" : "";
        if (i < 100){
            padding = "0" + padding;
        }
        fns->push_back("../data/umbrella/depth/frame-000" + padding + std::to_string(i) + ".depth.png");
    }
}

void
fusion_t::fusion(){
    min_params_t ps;

    // set defaults
    ps.eta = 0.1f;
    ps.omega_k = 0.5f;
    ps.omega_s = 0.2f;
    ps.gamma = 0.1f;
    ps.epsilon = 0.00005f;
    ps.threshold = 0.1f;

    std::vector<std::string> filenames;
    load_filenames(&filenames);
 
    for (int i = 0; i < filenames.size(); i++){
        sdf_t * sdf = get_sdf(filenames[i]);
        delete sdf;
    }
}
