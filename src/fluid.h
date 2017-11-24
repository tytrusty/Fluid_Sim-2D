#ifndef FLUID_H
#define FLUID_H

#define IX(i,j) ((i) + (N+2) * (j))

inline float lerp(float v0, float v1, float t) {
    return (1 - t)*v0 + t*v1;
}

struct Fluid {


};

static void adjust_bounds(int type, float* p, int N) 
{
    // type = 0 : density
    // type = 1 : p value
    // type = 2 : y value
    
    // Handling edges
    for (int i = 1; i <= N; ++i) {
       p[IX(0,   i  )] = (type == 1) ? -p[IX(1,i)] : p[IX(1,i)]; 
       p[IX(N+1, i  )] = (type == 1) ? -p[IX(N,i)] : p[IX(N,i)]; 
       p[IX(i,   0  )] = (type == 2) ? -p[IX(i,1)] : p[IX(i,1)]; 
       p[IX(i,   N+1)] = (type == 2) ? -p[IX(i,N)] : p[IX(i,N)]; 
    }

    // Handling corners -- average out the two nearest
    p[IX(0,   0  )] = 0.5 * (p[IX(1, 0  )] + p[IX(0,   1)]); 
    p[IX(0,   N+1)] = 0.5 * (p[IX(1, N+1)] + p[IX(0,   N)]); 
    p[IX(N+1, 0  )] = 0.5 * (p[IX(N, 0  )] + p[IX(N+1, 1)]); 
    p[IX(N+1, N+1)] = 0.5 * (p[IX(N, N+1)] + p[IX(N+1, N)]); 

}

/**
 * Uses Gauss-Seidel relaxation to solve linear system of equation 
 */
static void gauss_seidel (int type, float* p, float* p_prev, float a, float c,
        int N)
{
    for (int step = 0; step < 20; ++step) {
        for (int i = 0; i <= N; ++i) {
            for (int j = 0; j <= N; ++j) {
                p[IX(i,j)] = (p_prev[IX(i,j)] + a 
                        * (p[IX(i-1,j)] + p[IX(i+1,j)] 
                        +  p[IX(i,j-1)] + p[IX(i,j+1)])) / c;
            }
        }
    }
    // Adjust the boundaries of the array after changing values
    adjust_bounds(type, p, N);

}
/**
 * Solving for diffusion AKA the change in concentration due to flow out of
 * cell and flow from neighbors. Need a stable method, so solution is to
 * use the initial value and find the value when diffused back in
 * time yields our initial.
 *
 */
static void diffuse (int type, float* p, float* p_prev, float rate,
        float time_step, int N) 
{
    float a = time_step * rate * N * N;
    gauss_seidel (type, p, p_prev, a, 1 + 4 * a, N);
}

/**
 *
 *
 */
static void project (float* x, float* y, float* p, float* div, int N) 
{
    for (int i = 1; i <= N; ++i) {
        for (int j = 1; j <= N; ++j) {
            div[IX(i,j)] = (x[IX(i+1,j)] - x[IX(i-1,j)]
                          + y[IX(i, j+1)] - y[IX(i, j -1)]) * -0.5f / N;
            p[IX(i,j)] = 0;
        }
    }
    adjust_bounds(0, div, N);
    adjust_bounds(0, p, N);
    gauss_seidel (0, p, div, 1, 4, N);
    
    for (int i = 1; i <= N; ++i) {
        for (int j = 1; j <= N; ++j) {
            x[IX(i,j)] -=  0.5f * N * (p[IX(i+1,j)] - p[IX(i-1,j)]);
            y[IX(i,j)] -=  0.5f * N * (p[IX(i,j+1)] - p[IX(i,j-1)]);
        }
    }
    adjust_bounds(1, x, N);
    adjust_bounds(2, y, N);
}

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
            // Backtrace i according to the velocity field's p value
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
    // Adjust the boundaries of the array after changing values
    adjust_bounds(type, p, N);
}

#endif // FLUID_H 
