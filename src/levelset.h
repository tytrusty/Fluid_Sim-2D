#ifndef LEVELSET_H
#define LEVELSET_H

#include "grid.h"

struct LevelSet {
    int N_;

    /**
     * Signed distance grid.
     * < 0 indicates below the surface (in liquid) and
     * > 0 indicates above the surface (out of liquid)
     */
    Fluid_Grid<float> dist_grid; 
    
    LevelSet (int N) : N_(N), dist_grid(N)
    {
        dist_grid.set_all(1.0f); // init out of liquid state
    }

    bool is_liquid(int row, int col) {
        return dist_grid(row, col) < 0.0f;
    }
};

#endif // LEVELSET_H
