uniform sampler2D fbo_texture;
varying vec2 f_texcoord;

void main(void) {
	vec4 color = texture2D(fbo_texture, f_texcoord);
	gl_FragColor = vec4(color.r, color.g, color.b, 1.0);
}
