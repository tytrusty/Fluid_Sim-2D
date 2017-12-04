#ifndef CONFIG_H
#define CONFIG_H
namespace config
{
    static int N = 50;
    static float viscosity = 0.0f; // 100.0005f;  // kinematic viscosity
    static float diffusion = 0.0f; // 0.00001f; // density diffusion rate
    static float time_step = 0.125f;

    static void increment_time_step() { time_step *= 2; }
    static void decrement_time_step() { time_step /= 2; }
    static void increase_resolution() { N += 50; }
    static void decrease_resolution() { N -= 50; }
    static void increase_viscosity() { viscosity *= 2; }
    static void decrease_viscosity() { viscosity /= 2; }
}
#endif // CONFIG_H
