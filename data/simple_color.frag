varying vec2 tex_coord;
uniform sampler2D u_tex0;

void main()
{
	gl_FragColor = texture2D(u_tex0, tex_coord);
}
