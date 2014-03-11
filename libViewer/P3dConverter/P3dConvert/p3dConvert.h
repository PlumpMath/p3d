/*
 ------------------------------------------------------------------------------
 This file is part of the P3d .blend converter.

 Copyright (c) Nathan Letwory ( nathan@p3d.in / http://p3d.in )

 The converter uses FBT (File Binary Tools) from gamekit.
 http://gamekit.googlecode.com/

 ------------------------------------------------------------------------------
*/
#ifndef P3DCONVERT_H
#define P3DCONVERT_H

#include "fbtBlend.h"
#include "Blender.h"

#include "P3dVector.h"

#include <cstdlib>
#include <cstdint>

using namespace Blender;

class Chunk {
public:
    uint32_t totvert;
    uint32_t totface;
	float *v; /* verts, stride 3 */
    uint32_t *f; /* face indices, stride 3 */
};

class P3dMesh{
public:
	uint16_t totchunk;
	Chunk *chunks;
};

class P3dConverter {
public:
    P3dConverter();
    ~P3dConverter();

    P3dMesh *extract_all_geometry(size_t *count);
    int parse_blend(const char *path);
private:
    void free_p3d_mesh_data(P3dMesh *pme);
    int extract_geometry(Object *ob, P3dMesh *pme);
    void cleanup();
    int count_mesh_objects();

    P3dVector<P3dMesh> m_pme;
    fbtBlend m_fp;
    size_t totmesh;
};

#endif
