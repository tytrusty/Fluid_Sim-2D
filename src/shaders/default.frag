R"zzz(
#version 330 core
in vec2 UV;
uniform sampler2D textureSampler;
out vec4 fragment_color;
void main() {
    float density = texture(textureSampler, UV).r;
	// if (density == 255.0f)
	// 	fragment_color = vec4(0.0, 1.0, 0.0, 1.0);
	// else if (density > 1.0f)	
	// 	fragment_color = vec4(0.0, 1.0, 1.0, 1.0);
	// else
      	fragment_color = vec4(1.0, 1.0, 0.0, density);
     
}
)zzz"
