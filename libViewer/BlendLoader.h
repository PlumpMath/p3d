#ifndef BLENDLOADER_H
#define BLENDLOADER_H

#include <cstdlib>
#include <cstdint>
#include <cfloat>

#include "BaseLoader.h"
#include "PlatformAdapter.h"
#include "P3dMap.h"

#include "p3dConvert.h"

#include "glwrapper.h"

static P3dLogger logger("BlendLoader", P3dLogger::LOG_DEBUG);

#define STRIDE 3
#define UVSTRIDE 2
/** Class to pass around data buffers from Blender files */
class BlendData {
public:
	BlendData()
	{
	}
	~BlendData()
	{
		clearBlendData();
	}

	/** Prepare Blender data for further use in viewer */
	void initBlendData(P3dConverter &converter){
		uint32_t vs_index = 0;
		uint32_t uv_index = 0;
		uint32_t fs_index = 0;
		logger.debug("Collating data of %u mesh%s", converter.object_count(), converter.object_count()!=1?"es":"");
		allocateMemory(converter.totvert(), converter.totface(), converter.totuv());
		for(uint32_t i = 0; i < converter.object_count(); i++) {
			auto mesh = converter[i];
			collateData(
				vs_index, mesh->totvert, mesh->v,
				uv_index, mesh->totuv, mesh->uv,
				fs_index, mesh->totface, mesh->f);

			vs_index += mesh->totvert;
			uv_index += mesh->totuv;
			fs_index += mesh->totface;
		}
	}

	void clearBlendData() {
		isloaded = false;
		if(verts) delete [] verts;
		if(faces) delete [] faces;
		if(uvs) delete [] uvs;
		verts = nullptr;
		faces = nullptr;
		uvs = nullptr;
		totvert = 0;
		totface = 0;
		vertbytes = 0;
		facebytes = 0;
	}

	size_t size() const { return vertbytes + facebytes; }

	bool isLoaded() { return isloaded; }

	uint32_t totvert = 0;
	float *verts = nullptr;
	size_t vertbytes = 0;

	uint32_t totface = 0;
	uint32_t *faces = nullptr;
	size_t facebytes = 0;

	uint32_t totuv = 0;
	float *uvs = nullptr;
	size_t uvbytes = 0;

	bool isloaded = false;
private:
	void allocateMemory(uint32_t total_vertices, uint32_t total_faces, uint32_t total_uvs) {
		totvert = total_vertices;
		totface = total_faces;
		totuv = total_uvs;

		if(verts) delete [] verts;
		if(faces) delete [] faces;
		if(uvs) delete [] uvs;


		if(total_uvs>0) {
			uvs = new float[totvert*UVSTRIDE];
			uvbytes = totuv * sizeof(float) * UVSTRIDE;
		}
		verts = new float[totvert*STRIDE];
		vertbytes = totvert * sizeof(float) * STRIDE;

		faces = new uint32_t[totface*STRIDE];
		facebytes = totface * sizeof(uint32_t) * STRIDE;
	}

	/** Data is expected to be triangulated before being passed in here. */
	void collateData(uint32_t vs_start, uint32_t vs_count, float *vs, uint32_t uv_start, uint32_t uv_count, float *uv, uint32_t fs_start, uint32_t fs_count, uint32_t *fs) {
		isloaded = false;

		float *v, *vnew;
		float *uv_, *uvnew;
		uint32_t *f, *fnew;
		unsigned int i;
		vnew = &verts[vs_start*STRIDE];
		for(i=0, v = vs; i < vs_count*STRIDE; i++, v++, vnew++) {
			*vnew = *v;
		}

		if(uv) {
			uvnew = &uvs[uv_start*UVSTRIDE];
			for(i=0, uv_ = uv; i < uv_count*UVSTRIDE; i++, uv_++, uvnew++){
				*uvnew = *uv_;
			}
		}

		fnew = &faces[fs_start*STRIDE];
		for(i=0, f = fs; i < fs_count*STRIDE; i++, f++, fnew++) {
			/*  adjust face index, since vert and uv data is being collated, these
			 * indices point into collated array
			 */
			*fnew = *f+vs_start;
		}
	}
};

class BlendLoader : public BaseLoader
{
public:
	BlendLoader() {}
	virtual ~BlendLoader() {}

	bool load(const char *data, size_t length);

private:
	uint32_t reindexType(uint32_t &chunk, VertexType vtype, const BlendData *blendData,
						 uint16_t *new_faces);
	void copyVertData(uint32_t vertOffset, P3dMap<VertexIndex, uint32_t>* vertexMap, const BlendData& data,
					  GLfloat* new_norm, GLfloat* new_uv, GLfloat* new_pos);
	void nextChunk(uint32_t &chunk, BaseLoader::VertexType vtype, uint32_t new_offset,
				   uint32_t vertOffset, bool firstOfType = false);

	bool m_loaded = false;

	uint32_t m_pos_count = 0;
	uint32_t m_norm_count = 0;
	uint32_t m_tex_count = 0;

	uint32_t m_f3_count[4];
	uint32_t m_f4_count[4];

	uint32_t m_pos_start = 0;
	uint32_t m_norm_start = 0;
	uint32_t m_tex_start = 0;

	uint32_t m_f3_start[4];
	uint32_t m_f4_start[4];

	uint16_t m_mat_count = 0;

	// bounding box
	float m_maxX = FLT_MIN;
	float m_minX = FLT_MAX;
	float m_maxY = FLT_MIN;
	float m_minY = FLT_MAX;
	float m_maxZ = FLT_MIN;
	float m_minZ = FLT_MAX;

	// new data
	P3dVector<MeshChunk> m_chunks;

	P3dMap<uint32_t, P3dMap<VertexIndex, uint32_t>*> m_vertex_maps;

	uint32_t m_new_index_count[4];
	uint32_t m_new_f3_start[4];
	uint32_t m_new_f4_start[4];

	size_t m_total_index_count = 0;

	uint32_t m_new_pos_count = 0;
	uint32_t m_new_norm_count = 0;
	uint32_t m_new_empty_norm_count = 0;
	uint32_t m_new_uv_count = 0;
};

#endif // BLENDLOADER_H
