uniform samplerCube envmap;

varying vec3 vpos, vnorm, wviewdir, wnorm;
varying float wrad;

// these must match the maximum size of the field in scr_game.cc
#define FOG_NEAR 38.0
#define FOG_FAR 50.0

const vec3 depthcol = vec3(0.22, 0.25, 0.3) * 0.08;

/* schlick's approximation: [5.0, 1.0, R(0deg)] */
float hack_fresnel(vec3 i, vec3 n, vec3 fresnel_val)
{
	return fresnel_val.z + pow(1.0 - dot(i, n), fresnel_val.x) * fresnel_val.y;
}

void main()
{
	vec3 v = -normalize(vpos);
	vec3 n = normalize(vnorm);
	vec3 wn = normalize(wnorm);
	vec3 wv = normalize(wviewdir);
	vec3 wr = reflect(wv, wn);

	vec3 refl_col = textureCube(envmap, -wr).rgb;

	float envblend = clamp((wrad - FOG_NEAR) / (FOG_FAR - FOG_NEAR), 0.0, 1.0);
	vec3 env_color = textureCube(envmap, -normalize(wviewdir)).rgb;

	float fr = clamp(hack_fresnel(v, n, vec3(5.0, 2.0, 0.01)), 0.0, 1.0);

	vec3 color = mix(depthcol, refl_col, fr);

	vec3 out_color = mix(color, env_color, envblend);

#ifdef FB_NOT_SRGB
	gl_FragColor.xyz = pow(out_color, vec3(1.0 / 2.2));
#else
	gl_FragColor.xyz = out_color;
#endif
	gl_FragColor.w = 1.0;
}
