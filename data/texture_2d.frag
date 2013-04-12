uniform sampler2D u_tex_map;
varying vec2 v_texcoord;

void main()
{
	gl_FragColor = texture2D(u_tex_map, v_texcoord);
}
