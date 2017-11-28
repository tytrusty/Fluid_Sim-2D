#ifndef HEAT_H
#define HEAT_H

#include <glm/glm.hpp>
#include <vector>
#include "math.h"
namespace heat
{
    const int circle_vertices = 100;
    static float radius = 0.1;
    const int x = 10;
    const int y = 10;
    const float TWO_PI = M_PI * 2.0f;
    std::vector<glm::vec2> draw_boundary(float x = 0.5, float y = 0.5)
    {
        std::vector<glm::vec2> boundary;
        //boundary.push_back(glm::vec2(x, y));
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
