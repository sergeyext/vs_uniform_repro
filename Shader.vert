#version 330

// Tune array size according to your GL_MAX_VERTEX_UNIFORM_VECTORS
uniform vec4 vs[1000];
uniform sampler2D tex;

void main()
{
	gl_Position = vs[42];
	//gl_Position += texture(tex, vs[0].xy);
}
