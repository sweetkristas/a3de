uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
attribute vec3 a_position;

void main()
{
	//mat4 mvp_matrix = projection_matrix * view_matrix * model_matrix;
	//gl_Position = mvp_matrix * a_position;
	//gl_Position.xyz = a_position.xyz;
	//gl_Position.w = 1.0;
	mat4 mvp_matrix = projection_matrix * view_matrix * model_matrix;
	gl_Position = mvp_matrix * vec4(a_position, 1.0);
}
