#ifndef FLUID_H
#define FLUID_H

#include <iostream>
#include <string.h>

#define FOR_EVERY(N) for(int k=0; k < (N+2)*(N+2); ++k) {int i=k%(N); int j=k/(N);
#define END_FOR }

inline float lerp(float v0, float v1, float t) {
    return (1 - t)*v0 + t*v1;
}

inline void swap(float* v0, float* v1) {
    float* tmp = v0;
    v0 = v1;
    v1 = tmp;
}

/**
 * Difference grid types require different handling
 * for boundary conditions
 */
enum Grid_Type
{
    Density,
    X_Velocity,
    Y_Velocity, 
    None
}; 

template <typename T>
struct Fluid_Grid {
    T* array_;
    Grid_Type type_;
    int N_;

    Fluid_Grid(int N, Grid_Type type = None) : N_(N), type_(type) {
        array_ = new T[(N+2)*(N+2)];
        
        // Zero out array 
        set_all(0);
    } 

    /** Zero out the internal array */
    void reset() {
        // Zero out array 
        set_all(0);
    }

    /** Resize the internal array, then zero out new array */
    void resize(int N) {
        N_ = N;
        delete[] array_;
        array_ = new T[(N+2)*(N+2)];
        reset();
    }

    /** Set all values of the array to some value, v */
    void set_all(T v) {
        memset(array_, v, (N_+2)*(N_+2)*sizeof(*array_));
    }

    Fluid_Grid(const Fluid_Grid&) = delete;
    Fluid_Grid& operator = (const Fluid_Grid&) = delete;

    ~Fluid_Grid() {
        delete[] array_;
    }

    /** 2D access operator */
    T& operator () (int i, int j) { 
        return array_[i + (N_+2) * j]; 
    }    
};


inline void swap(Fluid_Grid<float>& v0, Fluid_Grid<float>& v1) {
    float* tmp = v0.array_;
    v0.array_ = v1.array_;
    v1.array_ = tmp;
}

struct Fluid_Sim {
    int N_;                 // simulation dimension
    float viscosity_;       // velocity diffusion rate
    float diffusion_;       // density diffusion rate
    float time_step_;       // time between simulation steps
    bool enable_gravity_;   // is gravity enabled
    bool enable_heat_;      // is heat diffusion enabled
    const int solver_steps = 30; // linear equation solver iterations
    Fluid_Grid<float> x, x_old,
                      y, y_old,
                      density, density_old,
                      viscosity_grid;

    /** Constructor */
    Fluid_Sim (int N, float viscosity, float diffusion, float time_step);

    /** Run a single time step of the simulation */
    void simulation_step();
    void reset();
    void resize(int N);

    void add_external_forces(Fluid_Grid<float>& target, 
            Fluid_Grid<float>& source);
    
    void add_gravity(Fluid_Grid<float>& y); 
    void add_heat(Fluid_Grid<float>& viscosity); 

    void adjust_bounds(Fluid_Grid<float>& grid);

    void gauss_seidel(Fluid_Grid<float>& grid, Fluid_Grid<float>& grid_prev,
            float a, float c);

    void diffuse(Fluid_Grid<float>& grid, Fluid_Grid<float>& grid_prev, 
            float rate);

    void gauss_seidel_viscosity(Fluid_Grid<float>& grid, Fluid_Grid<float>& grid_prev,
            Fluid_Grid<float>& viscosity);

    void diffuse_viscosity(Fluid_Grid<float>& grid, Fluid_Grid<float>& grid_prev, 
            Fluid_Grid<float>& viscosity);

    void project(Fluid_Grid<float>& x, Fluid_Grid<float>& y, Fluid_Grid<float>& p,
            Fluid_Grid<float>& div);
        
    void advect(Fluid_Grid<float>& grid, Fluid_Grid<float>& grid_prev,
        Fluid_Grid<float>& x_velocity, Fluid_Grid<float>& y_velocity);

    void debug_print () {
        printf("----- Density -----\n");
        for (int i = 1; i <= N_; ++i) {
            for (int j = 1; j <= N_; ++j) {
                printf("%.3f ", density(i,j));
            }
            printf("\n");
        }
        printf("\n");
    }

};
#endif // FLUID_H 
