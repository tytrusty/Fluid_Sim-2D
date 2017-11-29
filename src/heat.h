#ifndef HEAT_H
#define HEAT_H

#include <glm/glm.hpp>
#include <vector>
#include <ctime>
#include "math.h"
#include "config.h"

namespace heat
{
    const int circle_vertices = 100;
    const float TWO_PI = M_PI * 2.0f;
    static float radius = 0.01f;
    static float x = 0.8;
    static float y = 0.8;
    static clock_t begin = clock();

    /**
     * I'm not smart enough to do proper heat diffusion, so I just
     * made a circle expand over time ... 
     * @param filled Indicates whether circle should be filled or not
     */
    std::vector<glm::vec2> draw_boundary(bool filled = false)
    {
        // Get new radius
        clock_t end = clock();
        double elapsed = (double(end - begin) / CLOCKS_PER_SEC);
        begin = end;
        radius += (elapsed / config::time_step) * 0.0005;

        // Generate vertices to be used for a line loop
        std::vector<glm::vec2> boundary;
        for (int i = 0; i <= circle_vertices; ++i) {
            glm::vec2 vertex;
            vertex[0] = x + (radius * std::cos(i * TWO_PI / circle_vertices)); 
            vertex[1] = y + (radius * std::sin(i * TWO_PI / circle_vertices)); 
            boundary.push_back(vertex);
        }
        return boundary;
    }
}

#endif // HEAT_H
