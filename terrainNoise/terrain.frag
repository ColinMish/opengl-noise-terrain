// Minimal fragment shader

#version 420

in vec4 fcolour;
out vec4 outputColor;
//layout(location = 0) out float fragmentdepth;

void main()
{
	outputColor = fcolour;
	//fragmentdepth = gl_FragCoord.z;
}