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

#include <cstdlib>
#include <cstdint>

#include "fbtBlend.h"
#include "Blender.h"

#include "P3dVector.h"

using namespace Blender;

class Chunk {
public:
    Chunk(){ }
    ~Chunk() {
        delete [] v;
        delete [] f;
        v = 0; f = 0; totvert = 0; totface = 0;
    }

    uint32_t totvert;
    uint32_t totface;
	float *v; /* verts, stride 3 */
    uint32_t *f; /* face indices, stride 3 */
};

class P3dMesh{
public:
    uint16_t m_totchunk;
    Chunk *m_chunks;
};

class P3dConverter {
public:
    P3dConverter();
    ~P3dConverter();

    int parse_blend(const char *path, size_t length);

    size_t object_count() {
        return m_pme.size();
    }
    P3dMesh &operator[](size_t i) {
        return m_pme[i];
    }

private:
    void extract_all_geometry();
    void free_p3d_mesh_data(P3dMesh *pme);
    void extract_geometry(Object *ob);
    size_t count_mesh_objects();

    P3dVector<P3dMesh> m_pme;
    fbtBlend m_fp;
    size_t totmesh;
};

#endif
