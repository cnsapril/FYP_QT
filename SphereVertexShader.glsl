#version 430

in layout(location=0) vec3 position;

uniform mat4 fullMatrix;

void main()
{
	vec4 v = vec4(position, 1.0);
	gl_Position = fullMatrix * v;
}