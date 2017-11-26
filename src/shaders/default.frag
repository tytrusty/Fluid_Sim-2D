R"zzz(
#version 330 core
in vec2 UV;
uniform sampler2D textureSampler;
out vec4 fragment_color;
void main() {
    float density = texture(textureSampler, UV).y;
    if (texture(textureSampler, UV) == vec4(0.0, 0.0, 0.0, 0.0))
        fragment_color = vec4(1.0, 0.0, 0.0, 1.0);
    else
        fragment_color = vec4(0.0, 1.0, 0.0, 1.0);
        
}
)zzz"
