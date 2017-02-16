#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec2 texcoord;

uniform mat4 fullTransformMatrix;

out vec2 Texcoord;

void main()
{
	vec4 v = vec4(position, 1.0);
	gl_Position = fullTransformMatrix * v;

	Texcoord = texcoord;
}