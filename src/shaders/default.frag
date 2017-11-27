R"zzz(
#version 330 core
in vec2 UV;
uniform sampler2D textureSampler;
out vec4 fragment_color;
void main() {
    float density = texture(textureSampler, UV).r;
    float factor = log2(density*.80 + 1.0f);
    float r = 1.5f * factor;
    float g = 1.5 * factor * factor;
    float b = 0;
    fragment_color = vec4(r, g, b, 1.0);
}
)zzz"
