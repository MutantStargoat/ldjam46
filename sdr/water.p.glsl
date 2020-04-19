#version 120

uniform samplerCube envmap;

varying vec3 vpos, norm, wviewdir, wnorm;
varying vec3 ldir;

float fast_fresnel(vec3 i, vec3 n, vec3 fresnel_val)
{
	return fresnel_val.z + pow(1.0 - dot(i, n), fresnel_val.x) * fresnel_val.y;
}

void main()
{
	vec3 v = -normalize(vpos);
	vec3 n = normalize(norm);
	vec3 l = normalize(ldir);
	vec3 wvdir = normalize(wviewdir);
	vec3 wn = normalize(wnorm);

	vec3 r = reflect(v, n);
	vec3 env_tc = transpose(gl_NormalMatrix) * r;
	env_tc.z = -env_tc.z;
	vec3 envcol = textureCube(envmap, env_tc).xyz;

	//float fr = fast_fresnel(-v, n, vec3(3.5, 1.0, 0.0));

	gl_FragColor.xyz = envcol;
	gl_FragColor.w = 1.0;
}
