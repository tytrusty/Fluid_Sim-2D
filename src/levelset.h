#ifndef LEVELSET_H
#define LEVELSET_H

#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <vector>
#include "grid.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <debuggl.h>

struct LevelSet {
    int N_;
    float volume_;

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

    /** Add an object to levelset */
    void add_object(int width, int height) {
        int r_begin = 1; //std::max((N_ - height) / 2, 1);
        int r_end   = std::min(r_begin + height, N_);
        int c_begin = std::max((N_ - width) / 2, 1);
        int c_end   = std::min(c_begin + width, N_);
        std::cout << "row begin: " << r_begin << std::endl;
        std::cout << "row end: " << r_end << std::endl;
        for (int r = r_begin; r <= r_end; ++r) {
            for (int c = c_begin; c <= c_end; ++c) {
                dist_grid(r, c) = -1.0f; // in liquid
            }
        }
        // dist_grid.debug_print();
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
                glm::vec2 new_vertex;
                vertices[vertex_cnt][1] = (idx[i][0] * cell_ratio * 2) - 1.0f;
                vertices[vertex_cnt][0] = (idx[i][1] * cell_ratio * 2) - 1.0f;
                ++vertex_cnt;
            }
            
            // Now check for surface intersections
            // If signs are opposite then we know that the surface
            // crosses between these two cells. 
            float sign = dist_grid(idx[i][0], idx[i][1]) 
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
                glm::vec2 vertices[vertex_cnt];
                vertices[vertex_cnt][0] = (1 - p0_weight) * p0[0] + (p0_weight) * p1[0];
                vertices[vertex_cnt][1] = (1 - p0_weight) * p0[1] + (p0_weight) * p1[1];
                vertices[vertex_cnt][1] = (vertices[vertex_cnt][0] * 2) - 1.0f;
                vertices[vertex_cnt][0] = (vertices[vertex_cnt][1] * 2) - 1.0f;
                ++vertex_cnt;
            }
        }

        if (vertex_cnt) {
            // std::cout << "considering cell: row: " << row << " col: " << col << std::endl; 
            // for (int i = 0; i < vertex_cnt; ++i ) {
            //     std::cout << "v_" << i << " i: " << vertices[i][0] << std::endl;
            //     std::cout << "v_" << i << " j: " << vertices[i][1] << std::endl;
            // }
        }

        return vertex_cnt;
    }

    /** Basic area of a polygon computation */
    inline float calc_volume(std::vector<glm::vec2> vertices, int start, int end) {
        float volume = 0.0f;
        int i_next;
        for (int i = start; i < end; ++i) {
            // x1*y2 - x2*y1 + ...
            i_next = i+1; 
            if (i_next == end) i_next = start; 

            volume += (vertices[i][1] * vertices[i_next][0])
                    - (vertices[i][0] * vertices[i_next][1]); 
        }
        return volume / 2.0f;
    }

    /**
     * For each signed distance cell, run my blazingly hyper speed marching
     * squares algorithm to determine the surface vertices
     */
    void extract_surface() 
    {
        // Reset global volume
        volume_ = 0.0f;

        glm::vec2 vertices[8];
        for (int r = 1; r <= N_; ++r) {
            for (int c = 1; c <= N_; ++c) {
                int vertex_cnt = marching_cubes(r, c, vertices); 
                if (vertex_cnt) {
                    glBufferData(GL_ARRAY_BUFFER, vertex_cnt * sizeof(float) * 2,
                        vertices, GL_STATIC_DRAW);
                    glDrawArrays(GL_LINES, 0, vertex_cnt * sizeof(float) * 2);
                }
                // volume_ += calc_volume(vertices, start, end);
                // std::cout << "vertices.size()" << start << " for (i,j) : " << r << ", " << c << std::endl;
            }
        }
    }
};

#endif // LEVELSET_H
