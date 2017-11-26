R"zzz(
#version 330 core
in vec2 UV;
uniform sampler2D textureSampler;
out vec4 fragment_color;
void main() {
    float density = texture(textureSampler, UV).r;
    fragment_color = vec4(0.0, 1.0, 0.0, density);
        
}
)zzz"
