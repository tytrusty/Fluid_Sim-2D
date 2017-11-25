#include "fluid.h"
#include <iostream>

Fluid_Sim::Fluid_Sim (int N, float viscosity, float diffusion, float time_step)
   : N_(N), viscosity_(viscosity), diffusion_(diffusion), time_step_(time_step),
     x(N, X_Velocity), x_old(N, X_Velocity), 
     y(N, Y_Velocity), y_old(N, Y_Velocity), 
     density(N, Density), density_old(N, Density),
     pixels(N)
{}

void Fluid_Sim::simulation_step()
{

}

void Fluid_Sim::add_external_forces(Fluid_Grid<float> target,
        Fluid_Grid<float> source)
{

}
 
 
void Fluid_Sim::adjust_bounds(Fluid_Grid<float> grid)
{

}
 
void Fluid_Sim::gauss_seidel(Fluid_Grid<float> grid, 
        Fluid_Grid<float> grid_prev, float a, float c)
{

}
 
void Fluid_Sim::diffuse(Fluid_Grid<float> grid, Fluid_Grid<float> grid_prev,
        float rate)
{

}
 
void Fluid_Sim::project(Fluid_Grid<float> x, Fluid_Grid<float> y, 
        Fluid_Grid<float> p, Fluid_Grid<float> div)
{

}
 
void Fluid_Sim::advect(Fluid_Grid<float> grid, Fluid_Grid<float> grid_prev,
        Fluid_Grid<float> x, Fluid_Grid<float> y)
{

} 
