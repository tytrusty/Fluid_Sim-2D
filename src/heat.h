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
    static float radius = 0.01f;
    const float TWO_PI = M_PI * 2.0f;

    static clock_t begin = clock();

    std::vector<glm::vec2> draw_boundary(float x = 0.8, float y = 0.8)
    {
        clock_t end = clock();
        double elapsed = (double(end - begin) / CLOCKS_PER_SEC);
        begin = end;
        radius += (elapsed / config::time_step) * 0.005;

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
