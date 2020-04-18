#include <cstdio>

#include <GL/glew.h>
#include <GL/glut.h>

#include "scene_file.h" 

Mesh *SceneFile::load_mesh(const aiMesh *ai_mesh) {
    Mesh *m;
    unsigned int *indices;
    aiFace *ai_face;

    m = new Mesh();

    m->set_name(ai_mesh->mName.data);
    m->set_attrib_data(MESH_ATTR_VERTEX, 3, ai_mesh->mNumVertices,
            (float *)ai_mesh->mVertices);
    m->set_attrib_data(MESH_ATTR_NORMAL, 3, ai_mesh->mNumVertices,
            (float *)ai_mesh->mNormals);
    m->set_attrib_data(MESH_ATTR_TEXCOORD, 3, ai_mesh->mNumVertices,
            (float *)ai_mesh->mTextureCoords[0]);
    m->set_attrib_data(MESH_ATTR_TANGENT, 3, ai_mesh->mNumVertices,
            (float *)ai_mesh->mTangents);


    indices = (unsigned int *)malloc(ai_mesh->mNumFaces * 3 * sizeof(unsigned int));

    if (!indices) {
        fprintf(stderr, "%s: out of memory\n", __FUNCTION__);
        return NULL;
    }

    for (unsigned int i = 0; i < ai_mesh->mNumFaces; i++) {
        ai_face = ai_mesh->mFaces + i;
        indices[i * 3 + 0] = ai_face->mIndices[0];
        indices[i * 3 + 1] = ai_face->mIndices[1];
        indices[i * 3 + 2] = ai_face->mIndices[2];
    }

    m->set_index_data(ai_mesh->mNumFaces * 3, indices);
    free(indices);

    return m;
}

void SceneFile::load_scene(const aiScene *ai_scene) {
    Mesh *m;

    for (unsigned int i = 0; i < ai_scene->mNumMeshes; i++) {
        m = load_mesh(ai_scene->mMeshes[i]);
        if (!m) {
            fprintf(stderr, "parse_mesh failed\n");
            abort();
        }
        meshes.push_back(m);
    }
}

void SceneFile::load(const char *fname) {
    Assimp::Importer imp;
    const aiScene *scene;

    scene = imp.ReadFile(fname, aiProcess_Triangulate |
                aiProcess_GenSmoothNormals |
                aiProcess_CalcTangentSpace |
                aiProcess_JoinIdenticalVertices);

    if (scene) {
        load_scene(scene);
    } else {
        fprintf(stderr, "Import of model %s failed.\n", fname);
    }
}

