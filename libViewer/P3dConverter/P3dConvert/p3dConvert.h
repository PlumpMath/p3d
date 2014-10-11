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

class P3dMesh{
public:
	P3dMesh() {}
	~P3dMesh() {}

	uint32_t totvert = 0;
	uint32_t totface = 0;
	uint32_t totuv = 0;
	float *v = nullptr; /* verts, stride 3 */
	uint32_t *f = nullptr; /* face indices, stride 3 */
	float *uv = nullptr; /* totverts * 2 */
};

class P3dConverter {
public:
	P3dConverter();
	~P3dConverter();

	/** Parse the .blend at path. */
	int parse_blend(const char* path, size_t length);

	/** P3dMesh count in converter instance. */
	size_t object_count() {
		return m_pme.size();
	}

	/** Retrieve ith P3dMesh from converter instance. */
	P3dMesh* operator[](size_t i) {
		return m_pme[i];
	}

	/** Combined vertex count of all P3dMeshes. */
	uint32_t totvert() {
		uint32_t t = 0;
		for(auto pme : m_pme) {
			t += pme->totvert;
		}

		return t;
	}

	/** Combined UV count of all P3dMeshes. */
	uint32_t totuv() {
		uint32_t t = 0;
		for(auto pme : m_pme) {
			t += pme->totuv;
		}

		return t;
	}

	/** Combined face count of all P3dMeshes. */
	uint32_t totface() {
		uint32_t t = 0;
		for(auto pme : m_pme) {
			t += pme->totface;
		}

		return t;
	}

private:
	/** copy vertex data from loop. */
	void loop_data(MLoopUV* mpuv, P3dMesh* mesh, int curf, MLoop* loop, MLoopUV* luv);

	/** Start extracting all geometry from read blend. */
	void extract_all_geometry();

	/** Extract geometry from given Blender Object. */
	void extract_geometry(Object *ob);

	/** Determine count of mesh objects. */
	size_t count_mesh_objects();

	/** P3dVector holding all extracted P3dMeshes. */
	P3dVector<P3dMesh*> m_pme;

	/** Handle to .blend file. */
	fbtBlend m_fp;

	char* uvname = nullptr;
};

#endif
