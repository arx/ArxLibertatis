uniform sampler2D fbo_texture;
varying vec2 f_texcoord;

uniform float gamma = 1.0;
uniform int pnuxEffect = 0;

void main(void) {
	vec3 color = texture2D(fbo_texture, f_texcoord);

	if(pnuxEffect == 1) {
		float power = (color.x + color.y + color.z) * 0.6;
		if(power > 0.745) {
			color.x = power;
			color.y = power * 0.3;
			color.z = power * 0.7;
		} else {
			color.x = power * 0.7;
			color.y = power * 0.5;
			color.z = power;
		}
	} else if(pnuxEffect == 2) {
		float power = (color.x + color.y + color.z) * (1.0/3.0);
		if(power > color.x * 0.75) {
			color.y = color.x * 0.6;
			color.z = color.x * 0.5;
		} else {
			color.y = color.x * 0.3;
			color.z = color.x * 0.1;
		}

		color.x *= 1.3;
		color.y *= 1.5;
		color.z *= 1.5;

		if(power > 0.7843) {
			color.x += (power - 0.7843) * (1.0/5.0);
			color.y += (power - 0.7843) * (1.0/4.0);
			color.z += (power - 0.7843) * (1.0/3.0);
		}
	} else if(pnuxEffect == 3) {
		float power = (color.x + color.y + color.z) * (1.0/3.0) * 1.2;
		color.x = power;
		color.y = power;
		color.z = power;
	}

	gl_FragColor.rgb = pow(color, vec3(1.0 / gamma));
	gl_FragColor.a = 1.0;
}
