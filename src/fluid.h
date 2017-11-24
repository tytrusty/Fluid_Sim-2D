#ifndef FLUID_H
#define FLUID_H

#define IX(i,j) ((i) + (N+2) * (j))

inline float lerp(float v0, float v1, float t) {
    return (1 - t)*v0 + t*v1;
}

struct Fluid {


};

/**
 * Basic idea advect is to move the parameter through the static velocity
 * field
 */
static void advect (int type, float* p, float* p_prev, float* x_velocity,
        float* y_velocity, float time_step, int N) 
{
    int x_lo, x_hi, y_lo, y_hi;
    float x, y, x_w, y_w;

    // How much in time to step back
    float dt0 = time_step * N;

    for (int i = 1; i <= N; ++i) {
        for (int j = 1; j <= N; ++j) {
            // Backtrace i according to the velocity field's x value
            x = i - dt0 * x_velocity[IX(i,j)];
            if (x < 0.5)          x = 0.5;
            else if (x > N + 0.5) x = N + 0.5;
            // Get lower and upper bound cells
            x_lo  = (int) x;
            x_hi = x_lo + 1;

            // Backtrace j according to velocity field's y value
            y = j - dt0 * y_velocity[IX(i,j)];
            if (y < 0.5)          y = 0.5;
            else if (y > N + 0.5) y = N + 0.5; 
            // Get bounds on j cells
            y_lo  = (int) y;
            y_hi = y_lo + 1;
            
            // Perform advection by linearly interpolating from the
            // values at the backtraced cells
            x_w = x - x_lo; // x parametric weight
            y_w = y - y_lo; // y parametric weight
            
            // Bilinearly interpolating the new scalar value
            p[IX(i,j)] = 
                (1 - x_w) * lerp(p_prev[IX(x_lo, y_lo)], p_prev[IX(x_lo, y_hi)], y_w)
                    + x_w * lerp(p_prev[IX(x_hi, y_lo)], p_prev[IX(x_hi, y_hi)], y_w);
        }
    }
}
#endif // FLUID_H 
