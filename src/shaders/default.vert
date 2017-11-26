R"zzz(
#version 330 core
layout(location = 0) in vec2 vertex_position;
layout(location = 1) in vec2 uv_coord; 
out vec2 UV;
void main() {
	gl_Position = vec4(vertex_position, 0.0, 1.0);
	UV = uv_coord;
}
)zzz"
