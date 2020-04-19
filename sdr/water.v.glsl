varying vec3 vpos, norm, wviewdir, wnorm;
varying vec3 ldir;

void main()
{
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;

	vpos = (gl_ModelViewMatrix * gl_Vertex).xyz;
	norm = gl_NormalMatrix * gl_Normal;
	ldir = gl_NormalMatrix * vec3(0.0, 1.0, 0.0);

	// the water is in world space
	vec3 viewpos = (gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
	wviewdir = viewpos - gl_Vertex.xyz;
	wnorm = gl_Normal;
}
