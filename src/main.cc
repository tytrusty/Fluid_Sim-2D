#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

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

void
ErrorCallback(int error, const char* description)
{
    std::cerr << "GLFW Error: " << description << "\n";
}

int g_current_button;
bool g_mouse_pressed;
bool isPrevSet = false;
double prev_x, prev_y;

void
MousePosCallback(GLFWwindow* window, double mouse_x, double mouse_y)
{
    if (!g_mouse_pressed)
        return;

    if (g_current_button == GLFW_MOUSE_BUTTON_LEFT) {
    } else if (g_current_button == GLFW_MOUSE_BUTTON_RIGHT) {
    } else if (g_current_button == GLFW_MOUSE_BUTTON_MIDDLE) {
    }
    prev_x = mouse_x;
    prev_y = mouse_y;
}

void
MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    g_mouse_pressed = (action == GLFW_PRESS);
    g_current_button = button;
    isPrevSet = false;
}

int main(int argc, char* argv[])
{
    std::string window_title = "Menger";
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

    std::vector<glm::vec4> obj_vertices;
    std::vector<glm::uvec3> obj_faces;
    
    glm::vec4 min_bounds = glm::vec4(std::numeric_limits<float>::max());
    glm::vec4 max_bounds = glm::vec4(-std::numeric_limits<float>::max());
   
    for (uint32_t i = 0; i < obj_vertices.size(); ++i) {
        min_bounds = glm::min(obj_vertices[i], min_bounds);
        max_bounds = glm::max(obj_vertices[i], max_bounds);
    }
    std::cout << "min_bounds = " << glm::to_string(min_bounds) << "\n";
    std::cout << "max_bounds = " << glm::to_string(max_bounds) << "\n";

    // Setup our VAO array.
    CHECK_GL_ERROR(glGenVertexArrays(kNumVaos, &g_array_objects[0]));

    // Switch to the VAO for Geometry.
    CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kGeometryVao]));

    // Generate buffer objects
    CHECK_GL_ERROR(glGenBuffers(kNumVbos, &g_buffer_objects[kGeometryVao][0]));

    // Setup vertex data in a VBO.
    CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, g_buffer_objects[kGeometryVao][kVertexBuffer]));
    // NOTE: We do not send anything right now, we just describe it to OpenGL.
    CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
                sizeof(float) * obj_vertices.size() * 4, nullptr,
                GL_STATIC_DRAW));
    CHECK_GL_ERROR(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
    CHECK_GL_ERROR(glEnableVertexAttribArray(0));

    // Setup element array buffer.
    CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_buffer_objects[kGeometryVao][kIndexBuffer]));
    CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                sizeof(uint32_t) * obj_faces.size() * 3,
                &obj_faces[0], GL_STATIC_DRAW));

    // Setup vertex shader.
    GLuint vertex_shader_id = 0;
    const char* vertex_source_pointer = vertex_shader;
    CHECK_GL_ERROR(vertex_shader_id = glCreateShader(GL_VERTEX_SHADER));
    CHECK_GL_ERROR(glShaderSource(vertex_shader_id, 1, &vertex_source_pointer, nullptr));
    glCompileShader(vertex_shader_id);
    CHECK_GL_SHADER_ERROR(vertex_shader_id);

    // Setup geometry shader.
    GLuint geometry_shader_id = 0;
    const char* geometry_source_pointer = geometry_shader;
    CHECK_GL_ERROR(geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER));
    CHECK_GL_ERROR(glShaderSource(geometry_shader_id, 1, &geometry_source_pointer, nullptr));
    glCompileShader(geometry_shader_id);
    CHECK_GL_SHADER_ERROR(geometry_shader_id);

    // Setup fragment shader.
    GLuint fragment_shader_id = 0;
    const char* fragment_source_pointer = fragment_shader;
    CHECK_GL_ERROR(fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER));
    CHECK_GL_ERROR(glShaderSource(fragment_shader_id, 1, &fragment_source_pointer, nullptr));
    glCompileShader(fragment_shader_id);
    CHECK_GL_SHADER_ERROR(fragment_shader_id);

    // Let's create our program.
    GLuint program_id = 0;
    CHECK_GL_ERROR(program_id = glCreateProgram());
    CHECK_GL_ERROR(glAttachShader(program_id, vertex_shader_id));
    CHECK_GL_ERROR(glAttachShader(program_id, fragment_shader_id));
    CHECK_GL_ERROR(glAttachShader(program_id, geometry_shader_id));

    // Bind attributes.
    CHECK_GL_ERROR(glBindAttribLocation(program_id, 0, "vertex_position"));
    CHECK_GL_ERROR(glBindFragDataLocation(program_id, 0, "fragment_color"));
    glLinkProgram(program_id);
    CHECK_GL_PROGRAM_ERROR(program_id);

    // Get the uniform locations.
    GLint projection_matrix_location = 0;
    CHECK_GL_ERROR(projection_matrix_location =
            glGetUniformLocation(program_id, "projection"));
    GLint view_matrix_location = 0;
    CHECK_GL_ERROR(view_matrix_location =
            glGetUniformLocation(program_id, "view"));
    GLint light_position_location = 0;
    CHECK_GL_ERROR(light_position_location =
            glGetUniformLocation(program_id, "light_position"));
    glm::vec4 light_position = glm::vec4(10.0f, 10.0f, 10.0f, 1.0f);

    float aspect = 0.0f;
    float theta = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        // Setup some basic window stuff.
        glfwGetFramebufferSize(window, &window_width, &window_height);
        glViewport(0, 0, window_width, window_height);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LESS);

        // Switch to the Geometry VAO.
        CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kGeometryVao]));

        // Compute the projection matrix.
        aspect = static_cast<float>(window_width) / window_height;
        glm::mat4 projection_matrix =
            glm::perspective(glm::radians(45.0f), aspect, 0.0001f, 1000.0f);
        // Compute the view matrix
        glm::mat4 view_matrix;// = g_camera.get_view_matrix();

        // Send vertices to the GPU.
        CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
                                    g_buffer_objects[kGeometryVao][kVertexBuffer]));
        CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
                                    sizeof(float) * obj_vertices.size() * 4,
                                    &obj_vertices[0], GL_STATIC_DRAW));
        // Setup element array buffer.
        CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_buffer_objects[kGeometryVao][kIndexBuffer]));
        CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                    sizeof(uint32_t) * obj_faces.size() * 3,
                    &obj_faces[0], GL_STATIC_DRAW));

        // Use our program.
        CHECK_GL_ERROR(glUseProgram(program_id));

        // Pass uniforms in.
        CHECK_GL_ERROR(glUniformMatrix4fv(projection_matrix_location, 1, GL_FALSE,
                    &projection_matrix[0][0]));
        CHECK_GL_ERROR(glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE,
                    &view_matrix[0][0]));
        CHECK_GL_ERROR(glUniform4fv(light_position_location, 1, &light_position[0]));

        // Draw our triangles.
        CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, obj_faces.size() * 3, GL_UNSIGNED_INT, 0));

        // Poll and swap.
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
