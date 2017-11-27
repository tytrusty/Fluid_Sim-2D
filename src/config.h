#ifndef CONFIG_H
#define CONFIG_H
namespace config
{
    static int N =  100;
    const float viscosity = 0.0005f;//0.99999f;
    const float diffusion = 0.00001f; //100.999f;
    static float time_step = 0.125f;

    void increment_time_step() { time_step *= 2; }
    void decrement_time_step() { time_step /= 2; }
    void increase_resolution() { N += 50; }
    void decrease_resolution() { N -= 50; }
}
#endif // CONFIG_H
