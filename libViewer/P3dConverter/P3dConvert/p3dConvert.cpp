/*
 ------------------------------------------------------------------------------
 This file is part of the P3d .blend converter.

 Copyright (c) Nathan Letwory ( nathan@p3d.in / http://p3d.in )

 The converter uses FBT (File Binary Tools) from gamekit.
 http://gamekit.googlecode.com/

 ------------------------------------------------------------------------------
*/
#include "p3dConvert.h"
#include "fbtBlend.h"
#include "Blender.h"
#include <stdio.h>
#include <stdlib.h>

using namespace Blender;

P3dConverter::P3dConverter() {

}

P3dConverter::~P3dConverter() {
    m_pme.clear();
}

int P3dConverter::parse_blend(const char *data, size_t length) {
	bool gzipped = false;

    if (m_fp.parse(data, length, fbtFile::PM_READTOMEMORY, false) != fbtFile::FS_OK) {
        if (m_fp.parse(data, length, fbtFile::PM_COMPRESSED, false) != fbtFile::FS_OK)
		{
            fbtPrintf(" [NOK]\n");
			return 1;
		}
		gzipped = true;
	}
    fbtPrintf(" %s | %d%s[OK]\n", m_fp.getHeader().c_str(), m_fp.getVersion(), gzipped ? " compressed ": "");

    extract_all_geometry();
	
	return 0;
}

void P3dConverter::free_p3d_mesh_data(P3dMesh *pme) {
    Chunk *chunk = pme->m_chunks;
	if(chunk) {
        for(int i = 0; i < pme->m_totchunk; i++, chunk++) {
            delete [] chunk->v;
            delete [] chunk->f;
		}
	}
    delete [] pme->m_chunks;
}

void P3dConverter::extract_geometry(Object *ob) {
    uint32_t totf3 = 0;
    uint32_t totfx = 0;
	totmesh = 0;

    if(ob->type != 1 || !ob->data) throw;

    P3dMesh pme = P3dMesh();

    pme.m_totchunk = 0;
    pme.m_chunks = 0;

	Mesh *me = (Mesh *)ob->data;
	MVert *mvert = me->mvert;

    /* for now only one chunk. */
    pme.m_chunks = new Chunk[1]; //(Chunk *)malloc(sizeof(Chunk));
    pme.m_totchunk = 1;

    Chunk *chunk = pme.m_chunks;
	chunk->totvert = 0;
	chunk->totface = 0;
	chunk->v = 0;
	chunk->f = 0;

	/* create vertex pos buffer */
	chunk->totvert = me->totvert;
    chunk->v = new float[3*me->totvert];
    for(uint32_t i=0, curv = 0; i < chunk->totvert; i++, mvert++) {
        chunk->v[curv++] = mvert->co[0];
        chunk->v[curv++] = mvert->co[1];
        chunk->v[curv++] = mvert->co[2];
        fbtPrintf(" %s vert %d: (%f, %f, %f)\n", ob->id.name+2, i, chunk->v[curv-3], chunk->v[curv-2], chunk->v[curv-1]);
	}

	/* if totface > 0 we have legacy mesh format */
	if(me->totface) {

		/* count tris */
		MFace *mf = me->mface;
        fbtPrintf("LEGACY FACES (%d) in %s\n", me->totface, ob->id.name+2);
        for(uint32_t j=0; j < (uint32_t)me->totface; j++, mf++) {
			if(mf->v4==0) {
				totf3++;
			} else {
				totfx++;
			}
		}
		/* create buffer for tri indices */
		chunk->totface = totf3 + totfx*2;
        chunk->f = new uint32_t[3 * chunk->totface];
		mf = me->mface;
        for(uint32_t j=0, curf=0; j < (uint32_t)me->totface; j++, mf++) {
			if(mf->v4==0) {
                chunk->f[curf] = (uint32_t)mf->v1;
                chunk->f[curf+1] = (uint32_t)mf->v2;
                chunk->f[curf+2] = (uint32_t)mf->v3;
                curf+=3;
                fbtPrintf(" %s triface [LEGACY] %d: [%d, %d, %d]\n", ob->id.name+2, j, chunk->f[curf-3], chunk->f[curf-2], chunk->f[curf-1]);
			} else {
                chunk->f[curf] = (uint32_t)mf->v1;
                chunk->f[curf+1] = (uint32_t)mf->v2;
                chunk->f[curf+2] = (uint32_t)mf->v3;
                chunk->f[curf+3] = (uint32_t)mf->v1;
                chunk->f[curf+4] = (uint32_t)mf->v3;
                chunk->f[curf+5] = (uint32_t)mf->v4;
                curf+=6;
			}
		}
	/* mesh data since bmesh */
	} else if (me->totpoly) {
		/* count tris */
		MPoly *mp = me->mpoly;
        fbtPrintf("FACES (%d) in %s\n", me->totpoly, ob->id.name+2);
        for(uint32_t j=0; j < (uint32_t)me->totpoly; j++, mp++) {
			if(mp->totloop==3) {
				totf3++;
			} else if (mp->totloop == 4) {
				totfx++;
			}
		}
		/* create buffer for tri indices */
		chunk->totface = totf3 + totfx*2;
        chunk->f = new uint32_t[3 * chunk->totface];
		mp = me->mpoly;
		for(int j=0, curf=0; j < me->totpoly; j++, mp++) {
			MLoop *loop = &me->mloop[mp->loopstart];
			if(mp->totloop==3) {
                chunk->f[curf] = (uint32_t)loop->v; loop++;
                chunk->f[curf+1] = (uint32_t)loop->v; loop++;
                chunk->f[curf+2] = (uint32_t)loop->v;
                curf+=3;
                fbtPrintf(" %s triface %d: [%d, %d, %d]\n", ob->id.name+2, j, chunk->f[curf-3], chunk->f[curf-2], chunk->f[curf-1]);
			} else if (mp->totloop==4) {
                chunk->f[curf] = (uint32_t)loop->v; loop++;
                chunk->f[curf+1] = (uint32_t)loop->v; loop++;
                chunk->f[curf+2] = (uint32_t)loop->v; loop++;
                chunk->f[curf+3] = chunk->f[curf];
                chunk->f[curf+4] = chunk->f[curf+1];
                chunk->f[curf+5] = (uint32_t)loop->v;
                curf+=6;
                fbtPrintf(" %s quad found\n", ob->id.name+2);
			}
		}
    }
    m_pme.push_back(pme);
}

size_t P3dConverter::count_mesh_objects() {
	totmesh = 0;
    fbtList& objects = m_fp.m_object;
	for (Object* ob = (Object*)objects.first; ob; ob = (Object*)ob->id.next) {
		if (ob->data && ob->type == 1) {
			totmesh++;
		}
	}

	return totmesh;
}

void P3dConverter::extract_all_geometry() {

    size_t count = count_mesh_objects();

    fbtPrintf("%d mesh object%s found\n", count, count==1?"":"s");
    //P3dMesh *pme = new P3dMesh[*count];

    //P3dMesh *curpme = pme;

    fbtList& objects = m_fp.m_object;
	for (Object* ob = (Object*)objects.first; ob; ob = (Object*)ob->id.next) {
		if (ob->data && ob->type == 1) {
            extract_geometry(ob);
            //m_pme.push_back(curpme);
            //fbtPrintf("%s: %d verts, %d faces\n", ob->id.name+2, curpme.m_chunks[0].totvert, curpme.m_chunks[0].totface);
		}
	}
    fbtPrintf(" @.\n");
}
	
