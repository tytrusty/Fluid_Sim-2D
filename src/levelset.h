#ifndef LEVELSET_H
#define LEVELSET_H

#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <vector>
#include "grid.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <debuggl.h>

/** Basic area of a polygon computation */
inline float calc_volume(glm::vec2 vertices[8], int vertex_cnt) {
    float volume = 0.0f;
    for (int i = 0; i < vertex_cnt; ++i) {
        // x1*y2 - x2*y1 + ...
        volume += (vertices[i][1] * vertices[(i+1)%vertex_cnt][0])
                - (vertices[i][0] * vertices[(i+1)%vertex_cnt][1]); 
    }
    return volume / 2.0f;
}

struct LevelSet {
    int N_;
    float volume_, volume_prev_;

    /**
     * Signed distance grid.
     * < 0 indicates below the surface (in liquid) and
     * > 0 indicates above the surface (out of liquid)
     */
    Fluid_Grid<float> dist_grid; 
    Fluid_Grid<float> dist_grid_old; 
    
    LevelSet (int N) : N_(N), volume_(0.0f), dist_grid(N), dist_grid_old(N)
    {
        dist_grid.set_all(1.0f); // init out of liquid state
    }

    bool is_liquid(int row, int col) {
        return dist_grid(row, col) < 0.0f;
    }

    void reset() {
        dist_grid.set_all(1.0f);
        add_object(N_/2, N_/2);
    }

    /** Add an object to levelset */
    void add_object(int width, int height);

    /**
     * Run marching cubes on given cell 
     * @returns The number of vertices discovered by algorithm
     */
    int marching_cubes(int row, int col, glm::vec2 vertices[8]);

    /**
     * For each signed distance cell, run my blazingly hyper speed marching
     * squares algorithm to determine the surface vertices
     */
    void extract_surface(bool show_wireframe); 
};

#endif // LEVELSET_H
