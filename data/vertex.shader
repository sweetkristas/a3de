uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
attribute vec4 a_position;
attribute vec2 a_texcoord;
varying vec2 v_texcoord;

void main()
{
	mat4 mvp_matrix = model_matrix * view_matrix * projection_matrix;
	v_texcoord = a_texcoord;
	gl_Position = mvp_matrix * a_position;
}
