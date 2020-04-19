#include <stdio.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "opengl.h"
#include "scene_file.h"

static Mesh *load_mesh(const aiMesh *ai_mesh);

#define ASSFLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | \
		aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices)

bool SceneFile::load(const char *fname)
{
	Assimp::Importer imp;
	const aiScene *ai_scene;
	Mesh *m;

	if(!(ai_scene = imp.ReadFile(fname, ASSFLAGS))) {
		fprintf(stderr, "failed to load %s\n", fname);
		return false;

	}

	for(unsigned int i = 0; i < ai_scene->mNumMeshes; i++) {
		if(!(m = load_mesh(ai_scene->mMeshes[i]))) {
			fprintf(stderr, "failed to read mesh %d from %s\n", i, fname);
			continue;
		}
		meshes.push_back(m);
	}

	return true;
}

Mesh *SceneFile::find_mesh(char *name) {
	Mesh *found;

	found = NULL;
	for (unsigned int i = 0; i < meshes.size(); i++) {
		if (!strcmp(meshes[i]->get_name(), name)) {
			found = meshes[i];
			break;
		}
	}

	return found;
}


static Mesh *load_mesh(const aiMesh *ai_mesh)
{
	Mesh *m;
	unsigned int *indices;
	aiFace *ai_face;

	m = new Mesh;
	m->set_name(ai_mesh->mName.data);
	m->set_attrib_data(MESH_ATTR_VERTEX, 3, ai_mesh->mNumVertices,
			(float *)ai_mesh->mVertices);
	m->set_attrib_data(MESH_ATTR_NORMAL, 3, ai_mesh->mNumVertices,
			(float *)ai_mesh->mNormals);
	m->set_attrib_data(MESH_ATTR_TEXCOORD, 3, ai_mesh->mNumVertices,
			(float *)ai_mesh->mTextureCoords[0]);
	m->set_attrib_data(MESH_ATTR_TANGENT, 3, ai_mesh->mNumVertices,
			(float *)ai_mesh->mTangents);

	indices = m->set_index_data(ai_mesh->mNumFaces * 3, 0);

	for (unsigned int i = 0; i < ai_mesh->mNumFaces; i++) {
		ai_face = ai_mesh->mFaces + i;
		indices[i * 3 + 0] = ai_face->mIndices[0];
		indices[i * 3 + 1] = ai_face->mIndices[1];
		indices[i * 3 + 2] = ai_face->mIndices[2];
	}

	return m;
}
