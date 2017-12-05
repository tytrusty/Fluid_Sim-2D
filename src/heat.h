#ifndef HEAT_H
#define HEAT_H

#include <glm/glm.hpp>
#include <vector>
#include "grid.h"
#include "math.h"

class heat
{
private:
    const int circle_vertices = 100;
    const float TWO_PI = M_PI * 2.0f;
    float radius_ = 0.01f;
    const float x = 0.8;
    const float y = 0.8;
    float expansion = 0.0005f;
    const float visc_rate = 0.25f;

public: 
    /** Half the boundary's expansion rate */
    void decrease_rate() { expansion /= 2.0f; }
    /** Double the boundary's expansion rate */
    void increase_rate() { expansion *= 2.0f; }

    /**
     * I'm not smart enough to do proper heat diffusion, so I just
     * made a circle expand over time ... 
     * @param filled Indicates whether circle should be filled or not
     */
    std::vector<glm::vec2> draw_boundary(bool filled = false)
    {
        // Set new radius
        radius_ += expansion; 

        // Generate vertices to be used for a line loop
        std::vector<glm::vec2> boundary;
        for (int i = 0; i <= circle_vertices; ++i) {
            glm::vec2 vertex;
            vertex[0] = x + (radius_ * std::cos(i * TWO_PI / circle_vertices)); 
            vertex[1] = y + (radius_ * std::sin(i * TWO_PI / circle_vertices)); 
            boundary.push_back(vertex);
        }
        return boundary;
    }

    void update_boundary() 
    {
        radius_ += expansion;
    }

    void apply_heat(Fluid_Grid<float>& viscosity)
    {
        float grid_x, grid_y, dx, dy, dist;
        int N = viscosity.N_;
        for (int i = 1; i <= N; ++i) {
             for (int j = 1; j <= N; ++j) {
                 grid_x = (j / (float)N) * 2.0f - 1;
                 grid_y = (i / (float)N) * 2.0f - 1;  
                 dx = x - grid_x;
                 dy = y - grid_y;
                 dist = std::sqrt(dx*dx + dy*dy);
                 if (dist < radius_) {
                    float visc = viscosity(i,j) - visc_rate; 
                    viscosity(i,j) = std::max(0.0f, visc); 
                    // std::cout << "viscosity: " << viscosity(i,j) << std::endl;
                 }
             }
        }
    }

    float radius() { return radius_; }
};

#endif // HEAT_H
