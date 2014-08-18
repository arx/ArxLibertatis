uniform sampler2D fbo_texture;
varying vec2 f_texcoord;

uniform float gamma = 1.0;

void main(void) {
	vec3 color = texture2D(fbo_texture, f_texcoord);
	gl_FragColor.rgb = pow(color, vec3(1.0 / gamma));
	gl_FragColor.a = 1.0;
}
