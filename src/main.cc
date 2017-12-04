#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/io.hpp>

#include "config.h"
#include "fluid.h"
#include "heat.h"

// OpenGL library includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <debuggl.h>
 
using namespace std;

const char* vertex_shader =
#include "shaders/default.vert"
;

const char* fragment_shader =
#include "shaders/default.frag"
;

const char* heat_vertex_shader =
#include "shaders/heat.vert"
;

const char* heat_fragment_shader =
#include "shaders/heat.frag"
;

int window_width = 800, window_height = 800;


Fluid_Sim fluid_sim(config::N, config::viscosity, config::diffusion, 
        config::time_step);

float quad[] =
{
    -1.0f,  1.0f, 
    -1.0f, -1.0f,
     1.0f,  1.0f,
     1.0f, -1.0f
};

float tex_coords[] =
{
    0.0f, 1.0f, 
    0.0f, 0.0f,
    1.0f, 1.0f,
    1.0f, 0.0f
};

GLfloat red[] = {1.0f, 0.0f, 0.0f, 1.0f };
GLfloat blue[] = {0.2f, 0.2f, 1.0f, 1.0f };
GLfloat field[] = {0.6f, 0.2f, 1.0f, 1.0f };

bool show_velocity = false;
bool show_heat     = false;

std::vector<glm::vec2> generate_velocity_field()
{
    std::vector<glm::vec2> vector_field;
    int length = 3; // line length

    for (int row = 0; row < window_height; row += 5) {
        for (int col = 0; col < window_width; col += 5) {
            // Getting simulation-space coordinates
            int i = (int) ((row / (double) window_height) * (config::N + 2));
            int j = (int) ((col / (double) window_width)  * (config::N + 2));
            glm::vec2 p0 = glm::vec2(j/(float)config::N, 
                                     i/(float)config::N); 
            float dx = fluid_sim.y(i,j);
            float dy = fluid_sim.x(i,j);
            glm::vec2 p1 = glm::vec2((j + dx * length) / (double)config::N,
                                     (i + dy * length) / (double)config::N);
            p0[0] = p0[0] * 2 - 1;
            p0[1] = p0[1] * 2 - 1;
            p1[0] = p1[0] * 2 - 1;
            p1[1] = p1[1] * 2 - 1;
            vector_field.push_back(p0);
            vector_field.push_back(p1);
        }
    }
    return vector_field;

}

void
ErrorCallback(int error, const char* description)
{
    std::cerr << "GLFW Error: " << description << "\n";
}

int g_current_button;
bool g_mouse_pressed;
double prev_x, prev_y;
double current_x, current_y;

void
KeyCallback(GLFWwindow* window,
            int key,
            int scancode,
            int action,
            int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    else if (key == GLFW_KEY_S && mods == GLFW_MOD_CONTROL && action == GLFW_RELEASE) {
    } else if (key == GLFW_KEY_W && action != GLFW_RELEASE) {
    } else if (key == GLFW_KEY_S && action != GLFW_RELEASE) {
    } else if (key == GLFW_KEY_A && action != GLFW_RELEASE) {
    } else if (key == GLFW_KEY_V && action != GLFW_RELEASE) {
        std::cout << "Toggling Velocity Field" << std::endl;
        show_velocity = !show_velocity;
    } else if (key == GLFW_KEY_G && action != GLFW_RELEASE) {
        std::cout << "Toggling gravity" << std::endl;
        fluid_sim.enable_gravity_ = !fluid_sim.enable_gravity_;
    } else if (key == GLFW_KEY_H && action != GLFW_RELEASE) {
        std::cout << "Toggling heat diffusion" << std::endl;
        fluid_sim.enable_heat_ = !fluid_sim.enable_heat_;
        show_heat = !show_heat;
    } else if (key == GLFW_KEY_R && action != GLFW_RELEASE) {
        std::cout << "Resetting Simulation!" << std::endl;
        fluid_sim.reset();
    } else if (key == GLFW_KEY_LEFT && action != GLFW_RELEASE) {
        config::decrement_time_step();
        fluid_sim.time_step_ = config::time_step;
        std::cout << "time_step decrease: " << config::time_step << std::endl;
    } else if (key == GLFW_KEY_RIGHT && action != GLFW_RELEASE) {
        config::increment_time_step();
        fluid_sim.time_step_ = config::time_step;
        std::cout << "time_step increase: " << config::time_step << std::endl;
    } else if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE) {
        config::decrease_resolution();
        fluid_sim.resize(config::N);
        std::cout << "resolution decrease: " << config::N << std::endl;
    } else if (key == GLFW_KEY_UP && action != GLFW_RELEASE) {
        config::increase_resolution();
        fluid_sim.resize(config::N);
        std::cout << "resolution increase: " << config::N << std::endl;
    } else if (key == GLFW_KEY_C && action != GLFW_RELEASE) {
    } else if (key == GLFW_KEY_LEFT_BRACKET && action != GLFW_RELEASE) {
        config::decrease_viscosity();
        fluid_sim.viscosity_grid.set_all(config::viscosity);
        std::cout << "viscosity decrease: " << config::viscosity << std::endl;
    } else if (key == GLFW_KEY_RIGHT_BRACKET && action != GLFW_RELEASE) {
        config::increase_viscosity();
        fluid_sim.viscosity_grid.set_all(config::viscosity);
        std::cout << "viscosity increase: " << config::viscosity << std::endl;
    }
}

void
MousePosCallback(GLFWwindow* window, double mouse_x, double mouse_y)
{
    prev_x = current_x;
    prev_y = current_y;
    current_x = mouse_x;
    current_y = window_height - mouse_y;
    float delta_x = current_x - prev_x;
    float delta_y = current_y - prev_y;
    if (sqrt(delta_x * delta_x + delta_y * delta_y) < 1e-15)
        return;
    if (!g_mouse_pressed)
        return;
    bool add_velocity = g_mouse_pressed 
        && g_current_button == GLFW_MOUSE_BUTTON_LEFT;
    bool add_density = g_mouse_pressed 
        && g_current_button == GLFW_MOUSE_BUTTON_RIGHT;
   
    // Converting screen coordinates to fluid grid coordinates
    int i = (int) ((current_y / (double) window_height) * config::N + 1);
    int j = (int) ((current_x / (double) window_width) * config::N + 1);
    i = glm::clamp(i, 1, config::N);
    j = glm::clamp(j, 1, config::N);
    // std::cout << "blah: i: " << i << " j: " << j << " sgndist : " << fluid_sim.level_set_.dist_grid(i,j) << std::endl;

    // If dragging the mouse, influence the velocity field
    // If clicking mouse add density AKA add dye
    if (add_velocity) {
        fluid_sim.x_old(i, j) = (current_y - prev_y) * 10.0f;
        fluid_sim.y_old(i, j) = (current_x - prev_x) * 10.0f;
    } else if (add_density) {
        // fluid_sim.density_old(i, j) = 100.0f;
        int radius = 1;
        for (int x = max(i - radius, 0); x < min(i + radius, config::N); ++x) {
            for (int y = max(j - radius, 0); y < min(j + radius, config::N); ++y) {
                fluid_sim.density_old(x, y) = 250.0f;  
            }
        }
    }

}

void
MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    g_mouse_pressed = (action == GLFW_PRESS);
    g_current_button = button;
}

#include <ctime>
int main(int argc, char* argv[])
{
    std::string window_title = "Fluid";
    if (!glfwInit()) exit(EXIT_FAILURE);
    glfwSetErrorCallback(ErrorCallback);

    // Ask an OpenGL 3.3 core profile context 
    // It is required on OSX and non-NVIDIA Linux
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(window_width, window_height,
            &window_title[0], nullptr, nullptr);
    CHECK_SUCCESS(window != nullptr);
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    CHECK_SUCCESS(glewInit() == GLEW_OK);
    glGetError();  // clear GLEW's error for it
    glfwSetCursorPosCallback(window, MousePosCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetKeyCallback(window, KeyCallback);
    glfwSwapInterval(1);

    const GLubyte* renderer = glGetString(GL_RENDERER);  // get renderer string
    const GLubyte* version = glGetString(GL_VERSION);    // version as a string
    std::cout << "Renderer: " << renderer << "\n";
    std::cout << "OpenGL version supported:" << version << "\n";

    // Heat boundary 
    std::vector<glm::vec2> boundary = fluid_sim.heat_boundary_.draw_boundary();

    // Vector field
    std::vector<glm::vec2> vector_field = generate_velocity_field();

    // Initialize liquid
    // std::vector<glm::vec2> liquid;
    //fluid_sim.level_set_.extract_surface(liquid);

    // Setup VBO
    GLuint vbo;
    glGenBuffers(1, &vbo);              // generate 1 buffer (for quad)
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // switch to vbo 
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    // Setting up VBO for texture
    GLuint uv_vbo;
    glGenBuffers(1, &uv_vbo);              // generate buffer (for texture))
    glBindBuffer(GL_ARRAY_BUFFER, uv_vbo); // switch to vbo 
    glBufferData(GL_ARRAY_BUFFER, sizeof(tex_coords),
            tex_coords, GL_STATIC_DRAW);

    // Setting up VBO for heat boundary
    GLuint heat_vbo;
    glGenBuffers(1, &heat_vbo);  // generate buffer (for heat boundary);
    glBindBuffer(GL_ARRAY_BUFFER, heat_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * boundary.size() * 2,
            &boundary[0], GL_STATIC_DRAW);

    // Setting up VBO for vector field 
    GLuint vector_vbo;
    glGenBuffers(1, &vector_vbo);  // generate buffer (for vector field);
    glBindBuffer(GL_ARRAY_BUFFER, vector_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vector_field.size() * 2,
            &vector_field[0], GL_STATIC_DRAW);


    // Setting up VBO for vector field 
    GLuint liquid_vbo;
    glGenBuffers(1, &liquid_vbo);  // generate buffer (for liquid);
    glBindBuffer(GL_ARRAY_BUFFER, liquid_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float),
            NULL, GL_STATIC_DRAW);
    
    // Setup vertex shader
    const char* vertex_source_pointer = vertex_shader;
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader_id, 1, &vertex_source_pointer, nullptr);
    glCompileShader(vertex_shader_id);
    CHECK_GL_SHADER_ERROR(vertex_shader_id);

    // Setup fluid fragment shader.
    const char* fragment_source_pointer = fragment_shader;
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader_id, 1, &fragment_source_pointer, nullptr);
    glCompileShader(fragment_shader_id);
    CHECK_GL_SHADER_ERROR(fragment_shader_id);

    // Setup heat vertex shader.
    const char* heat_vertex_source_pointer = heat_vertex_shader;
    GLuint heat_vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(heat_vertex_shader_id, 1, &heat_vertex_source_pointer, 
            nullptr);
    glCompileShader(heat_vertex_shader_id);
    CHECK_GL_SHADER_ERROR(heat_vertex_shader_id);

    // Setup heat fragment shader.
    const char* heat_fragment_source_pointer = heat_fragment_shader;
    GLuint heat_fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(heat_fragment_shader_id, 1, &heat_fragment_source_pointer, 
            nullptr);
    glCompileShader(heat_fragment_shader_id);
    CHECK_GL_SHADER_ERROR(heat_fragment_shader_id);
    
    // Create shader programs
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);
    CHECK_GL_PROGRAM_ERROR(program_id);

    // Heat Boundary Program
    GLuint heat_program_id = glCreateProgram();
    glAttachShader(heat_program_id, heat_vertex_shader_id);
    glAttachShader(heat_program_id, heat_fragment_shader_id);
    glLinkProgram(heat_program_id);
    CHECK_GL_PROGRAM_ERROR(heat_program_id);

    // Vector Field Program
    GLuint velocity_program_id = glCreateProgram();
    glAttachShader(velocity_program_id, heat_vertex_shader_id);
    glAttachShader(velocity_program_id, heat_fragment_shader_id);
    glLinkProgram(velocity_program_id);
    CHECK_GL_PROGRAM_ERROR(velocity_program_id);
    
    GLuint liquid_program_id = glCreateProgram();
    glAttachShader(liquid_program_id, heat_vertex_shader_id);
    glAttachShader(liquid_program_id, heat_fragment_shader_id);
    glLinkProgram(liquid_program_id);
    CHECK_GL_PROGRAM_ERROR(liquid_program_id);

    // Setup Vertex Array Object
    GLuint vao; // vao for dye
    CHECK_GL_ERROR(glGenVertexArrays(1, &vao));
    CHECK_GL_ERROR(glBindVertexArray(vao));

    GLuint heat_vao; // vao for heat boundary
    CHECK_GL_ERROR(glGenVertexArrays(1, &heat_vao));
    CHECK_GL_ERROR(glBindVertexArray(heat_vao));

    GLuint velocity_vao; // vao for velocity vector field
    CHECK_GL_ERROR(glGenVertexArrays(1, &velocity_vao));
    CHECK_GL_ERROR(glBindVertexArray(velocity_vao));

    GLuint liquid_vao; // vao for liquid 
    CHECK_GL_ERROR(glGenVertexArrays(1, &liquid_vao));
    CHECK_GL_ERROR(glBindVertexArray(liquid_vao));

    // Bind fragment attributes.
    CHECK_GL_ERROR(glBindFragDataLocation(program_id, 0, "fragment_color")); 
    CHECK_GL_ERROR(glBindFragDataLocation(heat_program_id, 0, "fragment_color")); 
    CHECK_GL_ERROR(glBindFragDataLocation(velocity_program_id, 0, "fragment_color")); 
    CHECK_GL_ERROR(glBindFragDataLocation(liquid_program_id, 0, "fragment_color")); 

    // Setup color uniform
    GLuint heat_color_id     = glGetUniformLocation(heat_program_id, "color");
    GLuint velocity_color_id = glGetUniformLocation(velocity_program_id, "color");
    GLuint liquid_color_id   = glGetUniformLocation(liquid_program_id, "color");

    // Send density texture data
    GLuint texture_id = glGetUniformLocation(program_id, "textureSampler");
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
        config::N, config::N, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);    

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window)) {
        // Setup some basic window stuff.
        glfwGetFramebufferSize(window, &window_width, &window_height);
        glViewport(0, 0, window_width, window_height);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LESS);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, window_width, 0, window_height, 0, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        clock_t beg = clock();

        bool show_liquid = 1;
        // RENDER LIQUID //
        if (show_liquid) 
        {
            glUseProgram(liquid_program_id);
            glBindVertexArray(liquid_vao);
            //liquid.clear();
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, liquid_vbo);
            glVertexAttribPointer(
                        0, 
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        0,
                        (void*)0
            );
            glUniform4fv(liquid_color_id, 1, blue);
            fluid_sim.level_set_.extract_surface();
//            glDrawArrays(GL_TRIANGLE_FAN, 0, liquid.size());
        }

        // RENDER HEAT BOUNDARY //
        if (show_heat) 
        {
            glUseProgram(heat_program_id);
            glBindVertexArray(heat_vao);
            boundary = fluid_sim.heat_boundary_.draw_boundary();
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, heat_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * boundary.size() * 2,
                    &boundary[0], GL_STATIC_DRAW);
            glVertexAttribPointer(
                        0, 
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        0,
                        (void*)0
            );
            glUniform4fv(heat_color_id, 1, red);
            glDrawArrays(GL_LINE_LOOP, 0, boundary.size());
        } else {
            // Ensure that boundary is updated even if invisible
            fluid_sim.heat_boundary_.update_boundary(); 
        }

        // RENDER VECTOR FIELDS //
        if (show_velocity) 
        {
            glUseProgram(velocity_program_id);
            glBindVertexArray(velocity_vao);
            vector_field = generate_velocity_field();
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vector_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vector_field.size() * 2,
                    &vector_field[0], GL_STATIC_DRAW);
            glVertexAttribPointer(
                        0, 
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        0,
                        (void*)0
            );
            glUniform4fv(velocity_color_id, 1, field);
            glDrawArrays(GL_LINES, 0, vector_field.size());
        }

        
        // RENDER FLUID //
        fluid_sim.simulation_step();
        // fluid_sim.debug_print(fluid_sim.viscosity_grid);

        glUseProgram(program_id); 

        // Passing in texture
        glActiveTexture(GL_TEXTURE0);
        int pixels[config::N][config::N];
        float sum = 0.0;
        // _Pragma("omp parallel for")
        for (int i = 1; i <= config::N; ++i) {
            for (int j = 1; j <= config::N; ++j) {
                pixels[i-1][j-1] = min((int)fluid_sim.density(i, j), 255); 
                sum += fluid_sim.x(i,j);
            }
        }
        //std::cout << "density sum:  " << sum << std::endl;
        // sum = 0.0f;
        // for (int i = 0; i <= config::N+1; ++i) {
        //     for (int j = 0; j <= config::N+1; ++j) {
        //         sum += fluid_sim.density(i,j);
        //     }
        // }
        // std::cout << "density sum 2:  " << sum << std::endl;

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, config::N, config::N, GL_RGBA,
                GL_UNSIGNED_BYTE, pixels);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(texture_id, 0);
 
        glBindVertexArray(vao);       

        // Passing in vertex values
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(
                    0, 
                    2,
                    GL_FLOAT,
                    GL_FALSE,
                    0,
                    (void*)0
        );

        // Passing in per-vertex uv values
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
        glVertexAttribPointer(
                    1, 
                    2,
                    GL_FLOAT,
                    GL_FALSE,
                    0,
                    (void*)0
        );
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 8); 

        clock_t end = clock();
        // std::cout << ((end-beg)/(double)CLOCKS_PER_SEC) << std::endl;
        // Poll and swap.
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
