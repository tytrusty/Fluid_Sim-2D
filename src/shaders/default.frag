R"zzz(
#version 330 core
in vec2 UV;
uniform sampler2D textureSampler;
out vec4 fragment_color;
void main() {
    float density = texture(textureSampler, UV).r;
    float factor = log2(density*.75 + 1.0f);
    float b = 1.5f * factor;
    float g = 1.5 * factor * factor * factor;
    float r = b;
    fragment_color = vec4(r, g, b, 1.0);
    // if (density > 0.66)
    //     fragment_color = vec4(1.0, 0.0, 0.0, density); 
    // else if (density > 0.33)
    //     fragment_color = vec4(1.0, 0.0, 1.0, density); 
    // else 
    //     fragment_color = vec4(0.0, 0.0, 1.0, density); 
    // fragment_color = vec4(1.0, 0.0, 0.0, density); 
}
)zzz"
