#ifndef LEVELSET_H
#define LEVELSET_H

#include <glm/glm.hpp>
#include <vector>
#include "grid.h"

struct LevelSet {
    int N_;
    float volume_;

    /**
     * Signed distance grid.
     * < 0 indicates below the surface (in liquid) and
     * > 0 indicates above the surface (out of liquid)
     */
    Fluid_Grid<float> dist_grid; 
    
    LevelSet (int N) : N_(N), volume_(0.0f), dist_grid(N)
    {
        dist_grid.set_all(1.0f); // init out of liquid state
    }

    bool is_liquid(int row, int col) {
        return dist_grid(row, col) < 0.0f;
    }

    /**
     * Run marching cubes on given cell 
     * @returns The number of vertices discovered by algorithm
     */
    int marching_cubes(int row, int col, glm::vec2 vertices[8]) 
    {
        int vertex_cnt = 0;
        float cell_ratio = 1 / (float)N_;

        // Four corners of the cell
        int idx[4][2] =
        {
            {row    , col    },
            {row + 1, col    },
            {row + 1, col + 1},
            {row    , col + 1}
        };

        for (int i = 0; i < 4; ++i) {
            // First check if cell is in liquid 
            if (dist_grid(idx[i][0], idx[i][1]) < 0.0f) {
                vertices[vertex_cnt][0] = idx[i][0] * cell_ratio;
                vertices[vertex_cnt][1] = idx[i][1] * cell_ratio;
                ++vertex_cnt;
            }
            
            // Now check for surface intersections
            // If signs are opposite then we know that the surface
            // crosses between these two cells. 
            int sign = dist_grid(idx[i][0], idx[i][1]) 
                    *  dist_grid(idx[(i+1)%4][0], idx[(i+1)%4][1]);
            
            if (sign < 0.0f) {
                float dist0 = dist_grid(idx[i][0], idx[i][1]);
                float dist1 = dist_grid(idx[(i+1)%4][0], idx[(i+1)%4][1]);

                // Get interpolation weight. Subtracting because sign is < 0
                float p0_weight = dist0 / (dist0 - dist1); 

                // Getting fractional points and interpolating using the weight
                glm::vec2 p0(idx[i][0] * cell_ratio, idx[i][1] * cell_ratio);
                glm::vec2 p1(idx[(i+1)%4][0] * cell_ratio, idx[(i+1)%4][1] * cell_ratio);

                // Adding new vertex
                vertices[vertex_cnt][0] = (1 - p0_weight) * p0[0] + (p0_weight) * p1[0];
                vertices[vertex_cnt][1] = (1 - p0_weight) * p0[1] + (p0_weight) * p1[1];
                ++vertex_cnt;
            }

        }
        return vertex_cnt;
    }

    /** Basic area of a polygon computation */
    inline float calc_volume(glm::vec2 vertices[8], int cnt) {
        float volume = 0.0f;
        for (int i = 0; i < cnt; ++i) {
            // x1*y2 - x2*y1 + ...
            volume += (vertices[i][1] * vertices[(i+1)%cnt][0])
                    - (vertices[i][0] * vertices[(i+1)%cnt][1]); 
        }
        return volume / 2.0f;
    }

    /**
     * For each signed distance cell, run my blazingly hyper speed marching
     * squares algorithm to determine the surface vertices
     */
    void extract_surface(bool render = true) 
    {
        glm::vec2 vertices[8]; 

        // Reset global volume
        volume_ = 0.0f;

        for (int r = 1; r <= N_; ++r) {
            for (int c = 1; c <= N_; ++c) {
                int vertex_cnt = marching_cubes(r, c, vertices); 
                volume_ += calc_volume(vertices, vertex_cnt);
                if (render) {
                    render_liquid(vertices);
                }
            }
        }
    }

    /** Draw Liquid */
    void render_liquid(glm::vec2 vertices[8]) {

    }

};

#endif // LEVELSET_H
