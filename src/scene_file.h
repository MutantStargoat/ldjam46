#ifndef MESH_LOADER
#define MESH_LOADER

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh/mesh.h"

class SceneFile {
    Mesh *load_mesh(const aiMesh *);
    void load_scene(const aiScene *);
public:

    std::vector<Mesh *> meshes;
    void load(const char *fpath);
};
#endif
