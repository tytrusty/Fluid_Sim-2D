#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "config.h"
#include "fluid.h"

// OpenGL library includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <debuggl.h>
using namespace std;

const char* vertex_shader =
#include "shaders/default.vert"
;

const char* geometry_shader =
#include "shaders/default.geom"
;

const char* fragment_shader =
#include "shaders/default.frag"
;

int window_width = 800, window_height = 600;

// VBO and VAO descriptors.
enum { kVertexBuffer, kIndexBuffer, kNumVbos };

// These are our VAOs.
enum { kGeometryVao, kFloorVao, kNumVaos };

GLuint g_array_objects[kNumVaos];  // This will store the VAO descriptors.
GLuint g_buffer_objects[kNumVaos][kNumVbos];  // These will store VBO descriptors.

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

float vertices[] = {
     0.0f,  1.0f, // Vertex 1 (X, Y)
     1.0f, -1.0f, // Vertex 2 (X, Y)
    -1.0f, -1.0f  // Vertex 3 (X, Y)
};

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
MousePosCallback(GLFWwindow* window, double mouse_x, double mouse_y)
{
    prev_x = current_x;
    prev_y = current_y;
    current_x = mouse_x;
    current_y = mouse_y;
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
    int i = (int) ((current_x / (double) window_width) * config::N + 1);
    int j = (int) ((current_y / (double) window_height) * config::N + 1);
    i = glm::clamp(i, 1, config::N);
    j = glm::clamp(j, 1, config::N);

    // If dragging the mouse, influence the velocity field
    // If clicking mouse add density AKA add dye
    if (add_velocity) {
        fluid_sim.x_old(i, j) = (current_x - prev_x) * 10.0f;
        fluid_sim.y_old(i, j) = (current_y - prev_y) * 10.0f;
    } else if (add_density) {
        fluid_sim.density_old(i, j) = 100.0f;
    }
    //fluid_sim.simulation_step();
    //fluid_sim.debug_print();
}

void
MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    g_mouse_pressed = (action == GLFW_PRESS);
    g_current_button = button;
}

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
    glfwSwapInterval(1);

    const GLubyte* renderer = glGetString(GL_RENDERER);  // get renderer string
    const GLubyte* version = glGetString(GL_VERSION);    // version as a string
    std::cout << "Renderer: " << renderer << "\n";
    std::cout << "OpenGL version supported:" << version << "\n";

    // Setup VBO
    GLuint vbo;
    glGenBuffers(1, &vbo);              // generate 1 buffer (for quad)
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // switch to vbo 
    CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW));

	// Setting up VBO for texture
    GLuint uv_vbo;
    glGenBuffers(1, &uv_vbo);              // generate buffer (for texture))
    glBindBuffer(GL_ARRAY_BUFFER, uv_vbo); // switch to vbo 
    CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(tex_coords), 
		tex_coords, GL_STATIC_DRAW));

    // Setup vertex shader
    const char* vertex_source_pointer = vertex_shader;
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader_id, 1, &vertex_source_pointer, nullptr);
    glCompileShader(vertex_shader_id);
    CHECK_GL_SHADER_ERROR(vertex_shader_id);
    
    // Setup fragment shader.
    const char* fragment_source_pointer = fragment_shader;
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader_id, 1, &fragment_source_pointer, nullptr);
    glCompileShader(fragment_shader_id);
    CHECK_GL_SHADER_ERROR(fragment_shader_id);

    // Create shader program.
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);
    CHECK_GL_PROGRAM_ERROR(program_id);

	// Setup Vertex Array Object
    GLuint vao;
    CHECK_GL_ERROR(glGenVertexArrays(1, &vao));
    CHECK_GL_ERROR(glBindVertexArray(vao));

    // Bind attributes.
    CHECK_GL_ERROR(glBindFragDataLocation(program_id, 0, "fragment_color"));

    // GLint position_attr = 0;
    // CHECK_GL_ERROR(position_attr = glGetAttribLocation(program_id, "vertex_position"));
    // CHECK_GL_ERROR(glVertexAttribPointer(position_attr, 2, GL_FLOAT, GL_FALSE, 0, 0));
    // CHECK_GL_ERROR(glEnableVertexAttribArray(position_attr));

    // GLint uv_attr = 0;
    // CHECK_GL_ERROR(uv_attr = glGetAttribLocation(program_id, "vertex_position"));
    // //CHECK_GL_ERROR(uv_attr = glGetAttribLocation(program_id, "uv_coord"));
	// std::cout << "UV_ATTR: " << uv_attr << std::endl;
    // CHECK_GL_ERROR(glVertexAttribPointer(uv_attr, 2, GL_FLOAT, GL_FALSE, 0, 0));
    // CHECK_GL_ERROR(glEnableVertexAttribArray(uv_attr));

	GLuint texture_id = glGetUniformLocation(program_id, "textureSampler");
	GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
		config::N, config::N, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);	

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(program_id);

    // Get the uniform locations.
    // GLint projection_matrix_location = 0;
    //CHECK_GL_ERROR(projection_matrix_location =
    //        glGetUniformLocation(program_id, "projection"));

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
        glOrtho(0.0f, window_width, window_height, 0.0f, 0.0f, 1.0f);

        fluid_sim.simulation_step();
        
        // Passing in texture
		glActiveTexture(GL_TEXTURE0);
        std::cout << "DATA: " << fluid_sim.density.array_ << std::endl;
        for (int i = 0; i < (config::N) * (config::N) ; ++i) {
            printf("%.3f ", fluid_sim.density.array_[i]);
        }
        printf("\n");
    	glTexImage2D(GL_TEXTURE_2D, 0, 0, 0, config::N, config::N, GL_RGBA,
			GL_UNSIGNED_BYTE, fluid_sim.density.array_ );
    	glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(texture_id, 0);
       
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
        
        // Poll and swap.
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
