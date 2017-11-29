#ifndef CONFIG_H
#define CONFIG_H
namespace config
{
    static int N = 144;
    static float viscosity = 0.0005f;  // kinematic viscosity
    static float diffusion = 0.00001f; // density diffusion rate
    static float time_step = 0.125f;

    void increment_time_step() { time_step *= 2; }
    void decrement_time_step() { time_step /= 2; }
    void increase_resolution() { N += 50; }
    void decrease_resolution() { N -= 50; }
    void increase_viscosity() { viscosity *= 2; }
    void decrease_viscosity() { viscosity /= 2; }
}
#endif // CONFIG_H
