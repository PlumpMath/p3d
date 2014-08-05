#ifndef BLENDLOADER_H
#define BLENDLOADER_H

#include <cstdlib>
#include <cstdint>

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
		totvert = 0;
		totface = 0;
		vertbytes = 0;
		facebytes = 0;
		verts = nullptr;
		faces = nullptr;
		uvs = nullptr;
		isloaded = false;
	}
	~BlendData()
	{
		clearBlendData();
	}

	/** Data is expected to be triangulated before being passed in here. */
	void initBlendData(uint32_t tv, uint32_t tf, float *vs, float *uv, uint32_t *fs) {
		isloaded = false;
		if(verts) delete [] verts;
		if(faces) delete [] faces;
		if(uvs) delete [] uvs;

		totvert = tv;
		totface = tf;

		vertbytes = totvert * sizeof(float) * STRIDE;
		facebytes = totface * sizeof(uint32_t) * STRIDE;

		if(uv) uvs = new float[totvert*UVSTRIDE];
		verts = new float[totvert*STRIDE];
		faces = new uint32_t[totface*STRIDE];

		float *v, *vnew;
		float *uv_, *uvnew;
		uint32_t *f, *fnew;
		unsigned int i;
		for(i=0, v = vs, vnew = verts; i < tv*STRIDE; i++, v++, vnew++) {
			*vnew = *v;
		}

		if(uv) {
			for(i=0, uv_ = uv, uvnew = uvs; i < tv*UVSTRIDE; i++, uv_++, uvnew++){
				*uvnew = *uv_;
			}
		}


		for(i=0, f = fs, fnew = faces; i < tf*STRIDE; i++, f++, fnew++) {
			*fnew = *f;
		}

		if(totvert>0 && verts!=NULL) {
			isloaded = true;
		} else {
			clearBlendData();
		}
	}

	void initBlendData(P3dConverter &converter){
		logger.debug("Adding %u meshes", converter.object_count());
		for(uint32_t i = 0; i < converter.object_count(); i++) {
			P3dMesh &mesh = converter[0];
			for(uint32_t j = 0; j < mesh.m_totchunk; j++) {
				Chunk &chunk = mesh.m_chunks[j];
				initBlendData(chunk.totvert, chunk.totface, chunk.v, chunk.uv, chunk.f);
			}
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

	uint32_t totvert;
	float *verts;
	size_t vertbytes;

	uint32_t totface;
	uint32_t *faces;
	size_t facebytes;

	float *uvs;

	bool isloaded;
};

class BlendLoader : public BaseLoader
{
public:
	BlendLoader();
	virtual ~BlendLoader();

	bool load(const char *data, size_t length);

private:
	uint32_t reindexType(uint32_t &chunk, VertexType vtype, const BlendData *blendData,
						 uint16_t *new_faces);
	void copyVertData(uint32_t vertOffset, P3dMap<VertexIndex, uint32_t>* vertexMap, const BlendData& data,
					  GLfloat* new_norm, GLfloat* new_uv, GLfloat* new_pos);
	void nextChunk(uint32_t &chunk, BaseLoader::VertexType vtype, bool in_f4, uint32_t new_offset,
				   uint32_t vertOffset, bool firstOfType = false);

	bool m_loaded;

	uint32_t m_pos_count;
	uint32_t m_norm_count;
	uint32_t m_tex_count;

	uint32_t m_f3_count[4];
	uint32_t m_f4_count[4];

	uint32_t m_pos_start;
	uint32_t m_norm_start;
	uint32_t m_tex_start;

	uint32_t m_f3_start[4];
	uint32_t m_f4_start[4];

	uint16_t m_mat_count;

	// bounding box
	float m_maxX;
	float m_minX;
	float m_maxY;
	float m_minY;
	float m_maxZ;
	float m_minZ;

	// new data
	P3dVector<MeshChunk> m_chunks;

	P3dMap<uint32_t, P3dMap<VertexIndex, uint32_t>*> m_vertex_maps;

	uint32_t m_new_index_count[4];
	uint32_t m_new_f3_start[4];
	uint32_t m_new_f4_start[4];

	size_t m_total_index_count;

	uint32_t m_new_pos_count;
	uint32_t m_new_norm_count;
	uint32_t m_new_empty_norm_count;
	uint32_t m_new_uv_count;
};

#endif // BLENDLOADER_H
