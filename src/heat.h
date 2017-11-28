#ifndef HEAT_H
#define HEAT_H

#include <glm/glm.hpp>
#include <vector>
#include "math.h"
namespace heat
{
    const int circle_vertices = 20;
    static float radius = 10.0;
    const int x = 50;
    const int y = 50;
    std::vector<glm::vec2> draw_boundary()
    {
        std::vector<glm::vec2> boundary;
        boundary.push_back(glm::vec2(x, y));
        for (int i = 0; i < circle_vertices; ++i) {
            glm::vec2 vertex;
            vertex[0] = x + (radius * glm::cos(i * M_2_PI / circle_vertices)); 
            vertex[1] = y + (radius * glm::sin(i * M_2_PI / circle_vertices)); 
            boundary.push_back(vertex);
        }
        return boundary;
    }
}

#endif // HEAT_H
