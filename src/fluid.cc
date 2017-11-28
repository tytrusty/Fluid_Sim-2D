#include <iostream>
#include "fluid.h"

Fluid_Sim::Fluid_Sim (int N, float viscosity, float diffusion, float time_step)
   : N_(N), viscosity_(viscosity), diffusion_(diffusion), time_step_(time_step),
     enable_gravity_(false), enable_heat_(false),
     x(N, X_Velocity), x_old(N, X_Velocity), 
     y(N, Y_Velocity), y_old(N, Y_Velocity), 
     density(N, Density), density_old(N, Density),
     viscosity_grid(N)
{
    viscosity_grid.set_all(viscosity_);
}

    #include <ctime>
void Fluid_Sim::simulation_step()
{
    // --------- Velocity Solver --------- //
    // Assuming external forces currently stored in x_old and y_old
    add_external_forces(x, x_old);
    add_external_forces(y, y_old);

    if (enable_gravity_) {
        add_gravity(x);
    }

    swap(x, x_old); swap(y, y_old);

    // Viscous heat diffusion
    if (enable_heat_) {
        // If heat is enabled, we will have a non-uniform viscosity
        add_heat(viscosity_grid);
        diffuse_viscosity(x, x_old, viscosity_grid);
        diffuse_viscosity(y, y_old, viscosity_grid);
    } else {
        diffuse(x, x_old, viscosity_);
        diffuse(y, y_old, viscosity_);
    }

    // Enforce incompressibility
    project(x, y, x_old, y_old);
    swap(x, x_old); swap(y, y_old);
   
    // Self-Advection -- aka move velocity field along the velocity field
    advect(x, x_old, x_old, y_old);
    advect(y, y_old, x_old, y_old);

    // Enforce incompressibility, again
    project(x, y, x_old, y_old);

    // --------- Density Solver --------- //
    add_external_forces(density, density_old);
    //printf("Adding external forces\n");
    swap(density, density_old);
    diffuse(density, density_old, diffusion_);
    swap(density, density_old);
    advect(density, density_old, x, y);

    memset(density_old.array_, 0, (N_+2)*(N_+2)*sizeof(*density_old.array_));
}

void Fluid_Sim::reset() 
{
    x.reset();
    x_old.reset();
    y.reset();
    y_old.reset();
    density.reset();
    density_old.reset();
}

void Fluid_Sim::resize(int N) 
{
    N_ = N;
    x.resize(N);
    x_old.resize(N);
    y.resize(N);
    y_old.resize(N);
    density.resize(N);
    density_old.resize(N);
}

void Fluid_Sim::add_external_forces(Fluid_Grid<float>& target,
        Fluid_Grid<float>& source)
{
    for (int i = 0; i < (N_+2)*(N_+2); ++i) {
        target.array_[i] += source.array_[i] * time_step_;  
    }   
}
 
void Fluid_Sim::add_gravity(Fluid_Grid<float>& y) {
    for (int i = 1; i <= N_; ++i) {
        for (int j = 1; j <= N_; ++j) {
            y(i, j) += -9.8f * time_step_;
        }
    }
}

void Fluid_Sim::add_heat(Fluid_Grid<float>& viscosity) 
{
    for (int i = 1; i <= N_; ++i) {
        for (int j = 1; j <= N_; ++j) {
            viscosity(i, j) += -9.8f * time_step_;
        }
    }
}
void Fluid_Sim::adjust_bounds(Fluid_Grid<float>& grid)
{
    // Handling edges
    for (int i = 1; i <= N_; ++i) {
        grid(0,    i) = (grid.type_ == X_Velocity) ? -grid(1, i)  : grid(1,  i); 
        grid(N_+1, i) = (grid.type_ == X_Velocity) ? -grid(N_, i) : grid(N_, i); 
        grid(i,    0) = (grid.type_ == Y_Velocity) ? -grid(i, 1)  : grid(i,  1); 
        grid(i, N_+1) = (grid.type_ == Y_Velocity) ? -grid(i, N_) : grid(i, N_); 
    }

    // Handling corners -- average out the two nearest
    grid(0,       0) = 0.5 * (grid(1,     0) + grid(0,     1));
    grid(0,    N_+1) = 0.5 * (grid(1,  N_+1) + grid(0,    N_));
    grid(N_+1,    0) = 0.5 * (grid(N_,    0) + grid(N_+1,  1));
    grid(N_+1, N_+1) = 0.5 * (grid(N_, N_+1) + grid(N_+1, N_));
}
 
void Fluid_Sim::gauss_seidel(Fluid_Grid<float>& grid, 
        Fluid_Grid<float>& grid_prev, float a, float c)
{
    for (int step = 0; step < solver_steps; ++step) {
        for (int i = 1; i <= N_; ++i) {
            for (int j = 1; j <= N_; ++j) {
                grid(i, j) = (grid_prev(i,j) + a * (grid(i-1,j) + grid(i+1,j) 
                        + grid(i,j-1) + grid(i,j+1))) / c;
            }
        }
    }
    // Adjust the boundaries of the array after changing values
    adjust_bounds(grid);
}
 
void Fluid_Sim::gauss_seidel_viscosity(Fluid_Grid<float>& grid, 
        Fluid_Grid<float>& grid_prev, Fluid_Grid<float>& viscosity)
{
    for (int step = 0; step < solver_steps; ++step) {
        for (int i = 1; i <= N_; ++i) {
            for (int j = 1; j <= N_; ++j) {
                float a = time_step_ * viscosity(i, j) * N_ * N_;
                float c = 1 + 4 * a; 
                grid(i, j) = (grid_prev(i,j) + a * (grid(i-1,j) + grid(i+1,j) 
                        + grid(i,j-1) + grid(i,j+1))) / c;
            }
        }
    }
    // Adjust the boundaries of the array after changing values
    adjust_bounds(grid);
}

void Fluid_Sim::diffuse_viscosity(Fluid_Grid<float>& grid, Fluid_Grid<float>& grid_prev,
        Fluid_Grid<float>& viscosity)
{
    gauss_seidel_viscosity(grid, grid_prev, viscosity);
}

void Fluid_Sim::diffuse(Fluid_Grid<float>& grid, Fluid_Grid<float>& grid_prev,
        float rate)
{
    float a = time_step_ * rate * N_ * N_;
    float c = 1 + 4*a;
    gauss_seidel (grid, grid_prev, a, c);
}
 
void Fluid_Sim::project(Fluid_Grid<float>& x, Fluid_Grid<float>& y, 
        Fluid_Grid<float>& p, Fluid_Grid<float>& div)
{
    for (int i = 1; i <= N_; ++i) {
        for (int j = 1; j <= N_; ++j) {
            div(i,j) = (x(i+1,j) - x(i-1,j) + y(i, j+1) - y(i, j -1)) * -0.5f / N_;
            p(i,j) = 0;
        }
    }
    adjust_bounds(div);
    adjust_bounds(p);
    gauss_seidel (p, div, 1, 4);
    
    for (int i = 1; i <= N_; ++i) {
        for (int j = 1; j <= N_; ++j) {
            x(i,j) -=  0.5f * N_ * (p(i+1,j) - p(i-1,j));
            y(i,j) -=  0.5f * N_ * (p(i,j+1) - p(i,j-1));
        }
    }
    adjust_bounds(x);
    adjust_bounds(y);
}
 
void Fluid_Sim::advect(Fluid_Grid<float>& grid, Fluid_Grid<float>& grid_prev,
        Fluid_Grid<float>& x_velocity, Fluid_Grid<float>& y_velocity)
{
    int x_lo, x_hi, y_lo, y_hi;
    float x, y, x_w, y_w;

    // How much in time to step back
    float dt0 = time_step_ * N_;

    for (int i = 1; i <= N_; ++i) {
        for (int j = 1; j <= N_; ++j) {
            // Backtrace i according to the velocity field's x value
            x = i - dt0 * x_velocity(i,j);
            if (x < 0.5)           x = 0.5;
            else if (x > N_ + 0.5) x = N_ + 0.5;

            // Get lower and upper bound cells
            x_lo  = (int) x;
            x_hi = x_lo + 1;

            // Backtrace j according to velocity field's y value
            y = j - dt0 * y_velocity(i,j);
            if (y < 0.5)           y = 0.5;
            else if (y > N_ + 0.5) y = N_ + 0.5; 

            // Get bounds on j cells
            y_lo  = (int) y;
            y_hi = y_lo + 1;
            
            // Perform advection by linearly interpolating from the
            // values at the backtraced cells
            x_w = x - x_lo; // x parametric weight
            y_w = y - y_lo; // y parametric weight
            
            // Bilinearly interpolating the new scalar value
            grid(i,j) = 
                (1 - x_w) * lerp(grid_prev(x_lo, y_lo), grid_prev(x_lo, y_hi), y_w)
                    + x_w * lerp(grid_prev(x_hi, y_lo), grid_prev(x_hi, y_hi), y_w);
        }
    }
    // Adjust the boundaries of the array after changing values
    adjust_bounds(grid);
} 
