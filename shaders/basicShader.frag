#version 330 core

in vec3 fcolor;

out vec4 FragColor;

uniform int porta;

void main() {
	vec3 color;
	if(porta == 1)
		color = vec3(1., 0., 0.);
	else
		color = fcolor;
		
	FragColor = vec4(color, 1);
}

