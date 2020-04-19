#ifndef MESH_LOADER_H_
#define MESH_LOADER_H_

#include "mesh.h"

class SceneFile {
public:
    std::vector<Mesh*> meshes;

    bool load(const char *fpath);
	Mesh *find_mesh(char *name);
};

#endif	// MESH_LOADER_H_
