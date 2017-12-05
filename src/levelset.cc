#include "levelset.h"
#include <iostream>

void LevelSet::add_object(int width, int height) {
    int r_begin = 0; //std::max((N_ - height) / 2, 1);
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


int LevelSet::marching_cubes(int row, int col, glm::vec2 vertices[8]) 
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
            vertices[vertex_cnt][0] = (idx[i][0] * cell_ratio * 2) - 1.0f;
            vertices[vertex_cnt][1] = (idx[i][1] * cell_ratio * 2) - 1.0f;
            ++vertex_cnt;
        }
        
        // Now check for surface intersections
        // If signs are opposite then we know that the surface
        // crosses between these two cells. 
        float sign = dist_grid(idx[i][0], idx[i][1]) 
                *  dist_grid(idx[(i+1)%4][0], idx[(i+1)%4][1]);
    
        if (sign < 0.0f) {
            //std::cout << "idx[i][0] " << idx[i][0] << " [i][1] : " << idx[i][1] << std::endl;
            //std::cout << "idx[i+1][0] " << idx[(i+1)%4][0] << " [i][1] : " << idx[(i+1)%4][1] << std::endl;
            float dist0 = dist_grid(idx[i][0], idx[i][1]);
            float dist1 = dist_grid(idx[(i+1)%4][0], idx[(i+1)%4][1]);

            // Get interpolation weight. Subtracting because sign is < 0
            float p0_weight = dist0 / (dist0 - dist1); 

            // Getting fractional points and interpolating using the weight
            glm::vec2 p0(idx[i][0]             * cell_ratio, idx[i][1] * cell_ratio);
            glm::vec2 p1(idx[(i+1)%4][0] * cell_ratio, idx[(i+1)%4][1] * cell_ratio);
            //std::cout << "p0: " << p0 << std::endl;
            //std::cout << "p1: " << p1 << std::endl;

            // Adding new vertex
            vertices[vertex_cnt][0] = (1.0f - p0_weight) * p0[0] + (p0_weight) * p1[0];
            vertices[vertex_cnt][1] = (1.0f - p0_weight) * p0[1] + (p0_weight) * p1[1];
            vertices[vertex_cnt][0] = (vertices[vertex_cnt][0] * 2.0f) - 1.0f;
            vertices[vertex_cnt][1] = (vertices[vertex_cnt][1] * 2.0f) - 1.0f;
            //  std::cout << "vertex isec: x" << vertices[vertex_cnt][0] << std::endl;
            //  std::cout << "vertex isec: y " << vertices[vertex_cnt][1] << std::endl;
            ++vertex_cnt;
        }
    }

     if (0 && vertex_cnt) {
         std::cout << "considering cell: row: " << row << " col: " << col << std::endl; 
          for (int i = 0; i < vertex_cnt; ++i ) {
              std::cout << "vertex: " << i << " i: " << vertices[i][0] << std::endl;
              std::cout << "vertex: " << i << " j: " << vertices[i][1] << std::endl;
          }
     }

    return vertex_cnt;
}

void LevelSet::extract_surface(bool show_wireframe) 
{
    // Reset global volume
    volume_prev_ = volume_;
    volume_ = 0.0f;

    glm::vec2 vertices[8];
    for (int r = 1; r <= N_; ++r) {
        for (int c = 1; c <= N_; ++c) {
            int vertex_cnt = marching_cubes(r, c, vertices); 
            if (vertex_cnt) {
                // So god damn hacky lol
                for (int i = 0 ; i < vertex_cnt; ++i) {
                    float tmp = vertices[i][0];
                    vertices[i][0] = vertices[i][1];
                    vertices[i][1] = tmp;
                }
                glBufferData(GL_ARRAY_BUFFER, vertex_cnt * sizeof(float) * 2,
                    vertices, GL_STATIC_DRAW);
                if (show_wireframe) {
                    glDrawArrays(GL_LINE_LOOP, 0, vertex_cnt);
                } else {
                    glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_cnt);
                }
            }
            volume_ += calc_volume(vertices, vertex_cnt);
        }
    }
}
