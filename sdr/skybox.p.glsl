uniform samplerCube envmap;

void main()
{
	gl_FragColor = textureCube(envmap, gl_TexCoord[0].xyz);
}
