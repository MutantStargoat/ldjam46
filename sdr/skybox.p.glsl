uniform samplerCube envmap;

void main()
{
	vec3 color = textureCube(envmap, gl_TexCoord[0].xyz).xyz;

#ifdef FB_NOT_SRGB
	gl_FragColor.xyz = pow(color, vec3(1.0 / 2.2));
#else
	gl_FragColor.xyz = color;
#endif
	gl_FragColor.w = 1.0;
}
