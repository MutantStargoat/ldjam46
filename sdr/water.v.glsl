varying vec3 vpos, vnorm, wviewdir, wnorm;

void main()
{
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;

	vpos = (gl_ModelViewMatrix * gl_Vertex).xyz;
	vnorm = gl_NormalMatrix * gl_Normal;

	vec3 wpos = gl_Vertex.xyz;
	wnorm = gl_Normal.xyz;
	vec3 cam_pos = (gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
	wviewdir = cam_pos - wpos;
}
