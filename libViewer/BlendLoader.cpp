#include "BlendLoader.h"
#include "ModelLoader.h"

static BlendLoader blendLoader;
static RegisterLoader registerBlendLoader(&blendLoader, ".blend", 0);

/********** BLENDER DATA ***************************/

bool BlendLoader::load(const char *data, size_t length)
{
	P3dConverter converter;
	BlendData blendData;

	m_modelLoader->clear();
	m_chunks.clear();

	logger.debug("Ready for  parsing blend\n");
	converter.parse_blend(data, length);
	logger.debug("Done parsing blend\n");
	logger.debug("Initing blend data\n");
	blendData.initBlendData(converter);
	logger.debug("Done initing blend data\n");

	uint64_t start = PlatformAdapter::currentMillis();
	uint32_t chunk = 0;

	uint16_t* new_faces = new uint16_t[blendData.totface*STRIDE];

	/* initialize counters and indices */
	m_new_pos_count = 0;
	m_new_norm_count = 0;
	m_new_uv_count = 0;
	m_total_index_count = 0;

	/* CANDIDATE FOR REMOVAL
	 * all types start at 0, because they have separate data arrays in BlendData */
	for(int vtype = 0; vtype < 4; vtype++)
	{
		m_new_index_count[vtype] = 0;
		m_new_f3_start[vtype] = 0;
		m_new_f4_start[vtype] = 0;
	}

	/* for now we do only VT_POS */
	m_new_index_count[VT_POS] = blendData.totface * 3;
	m_new_f3_start[VT_POS] = 0;
	m_total_index_count += m_new_index_count[VT_POS];

	/* reindex VT_POS */
	reindexType(chunk, VT_POS, &blendData, new_faces);

	GLfloat* new_pos = new GLfloat[blendData.totvert * STRIDE];
	GLfloat* new_uv = new GLfloat[blendData.totuv * UVSTRIDE];
	GLfloat* new_norm = new GLfloat[blendData.totvert * STRIDE];

	if(new_pos==NULL || new_uv==NULL || new_norm==NULL) return false;

	logger.debug("GLfloat buffers allocated");

	for(chunk = 0; chunk < m_chunks.size(); ++chunk)
	{
		logger.debug("chunk: %d", chunk);
		logger.debug(" index count: %d", m_chunks[chunk].indexCount);
		logger.debug(" vert count: %d", m_chunks[chunk].vertCount);
		logger.debug(" f3 offset: %d", m_chunks[chunk].f3Offset);
		logger.debug(" f4 offset: %d", m_chunks[chunk].f4Offset);
		logger.debug(" material: %d", m_chunks[chunk].material);
	}

	logger.debug("----------");

	for(auto item: m_vertex_maps)
	{
		logger.debug("vertex bank:");
		logger.debug(" offset: %d", item.first);
		logger.debug(" count: %d", item.second->size());
		copyVertData(item.first, item.second, blendData, new_norm, new_uv, new_pos);
		item.second->dumpBucketLoad();
		delete item.second;
	}

	logger.debug("data copied");
	m_vertex_maps.clear();

	m_modelLoader->createModel(m_new_pos_count, m_new_norm_count, m_new_empty_norm_count, m_new_uv_count,
				new_pos, new_norm, new_uv, m_total_index_count,
				new_faces, m_chunks.size(), m_chunks.data());

	delete [] new_norm;
	delete [] new_uv;
	delete [] new_pos;

	delete [] new_faces;

	m_loaded = true;

	m_modelLoader->setBoundingBox(m_minX, m_maxX, m_minY, m_maxY, m_minZ, m_maxZ);
	m_modelLoader->setIsLoaded(m_loaded);

	logger.debug("reindex took %lldms", PlatformAdapter::durationMillis(start));

	return m_loaded;
}

void BlendLoader::reindexType(uint32_t &chunk, BlendLoader::VertexType vtype, const BlendData *blendData,
								  uint16_t* new_faces)
{
	uint32_t pos_offset;
	uint16_t mat = 0;
	uint32_t face;
	uint32_t fcount;
	uint32_t vert;
	uint32_t verts;
	uint32_t new_offset;

	uint64_t start = PlatformAdapter::currentMillis();

	P3dMap<VertexIndex, uint32_t>* vertexMap = nullptr;

	fcount = blendData->totface;

	// tris
	pos_offset = 0;

	VertexIndex index;
	index.type = vtype;
	uint16_t new_index;
	new_offset = 0; //TODO: should be passed in?

	for(face = 0; face < fcount; ++face)
	{
		if(face == 0)
		{
			nextChunk(chunk, vtype, new_offset, m_new_pos_count / 3, true);
			m_chunks[chunk].material = mat;
			m_chunks[chunk].hasUvs = blendData->uvs != nullptr;
			vertexMap = m_vertex_maps[m_chunks[chunk].vertOffset];
		}
		else if(mat != m_chunks[chunk].material)
		{
			nextChunk(chunk, vtype, new_offset, m_chunks[chunk].vertOffset, false);
			m_chunks[chunk].material = mat;
			m_chunks[chunk].hasUvs = blendData->uvs != nullptr;
			vertexMap = m_vertex_maps[m_chunks[chunk].vertOffset];
		}

		if(vertexMap->size() > 65530)
		{
			// next chunk
			nextChunk(chunk, vtype, new_offset, m_new_pos_count / 3, false);
			m_chunks[chunk].material = mat;
			m_chunks[chunk].hasUvs = blendData->uvs != nullptr;
			vertexMap = m_vertex_maps[m_chunks[chunk].vertOffset];
		}

		verts = 3;
		for(vert = 0; vert < verts; ++vert)
		{
			index.pos = blendData->faces[pos_offset];
			pos_offset++;

			/* TODO: norm */
			index.norm = 0;
			index.uv = index.pos;

			if(vertexMap->count(index))
			{
				new_index = (*vertexMap)[index];
			}
			else
			{
				new_index = vertexMap->size();
				vertexMap->insert(index, new_index);

				m_new_pos_count += 3;

				m_new_norm_count += 3;
				m_new_empty_norm_count += 3;

				m_new_uv_count += 2;
			}

			new_faces[new_offset] = (uint16_t)new_index;
			++new_offset;
		}
	}

	m_chunks[chunk].vertCount = (m_new_pos_count - STRIDE * m_chunks[chunk].vertOffset) / 3;

	logger.debug("reindex type Blender took: %lldms", PlatformAdapter::durationMillis(start));
}

void BlendLoader::nextChunk(uint32_t &chunk, BlendLoader::VertexType vtype, uint32_t new_offset,
							uint32_t vertOffset, bool firstOfType)
{
	logger.debug("new_offset: %d", new_offset);
	logger.debug("vertOffset: %d", vertOffset);
	if(m_chunks.size() != 0)
	{
		++chunk;
	}
	if(chunk >= m_chunks.size())
	{
		while(chunk >= m_chunks.size()) m_chunks.push_back(MeshChunk());
	}
	else
	{
		m_chunks[chunk] = MeshChunk();
	}
	m_chunks[chunk] = MeshChunk();
	MeshChunk& newChunk = m_chunks[chunk];
	newChunk.indexCount = m_new_index_count[vtype];
	newChunk.f3Offset = new_offset;

	newChunk.validNormals = vtype == VT_POS_NORM || vtype == VT_POS_UV_NORM;
	newChunk.hasUvs = vtype == VT_POS_UV || vtype == VT_POS_UV_NORM;

	newChunk.vertOffset = vertOffset;

	if(firstOfType)
	{
		newChunk.indexCount = m_new_index_count[vtype];
		newChunk.f3Offset = m_new_f3_start[vtype];
		//newChunk.f4Offset = m_new_f4_start[vtype];
	}
	else
	{
		MeshChunk& oldChunk = m_chunks[chunk - 1];
		newChunk.indexCount = oldChunk.indexCount - (new_offset - oldChunk.f3Offset);
		oldChunk.indexCount = new_offset - oldChunk.f3Offset;
		oldChunk.vertCount = (m_new_pos_count - oldChunk.vertOffset * 3) / 3;
	}

	if(m_vertex_maps.count(newChunk.vertOffset) == 0)
	{
		m_vertex_maps[newChunk.vertOffset] = new P3dMap<VertexIndex, uint32_t>(8192);
	}
}



void BlendLoader::copyVertData(uint32_t vertOffset, P3dMap<VertexIndex, uint32_t>* vertexMap, const BlendData& data,
							   GLfloat* new_norm, GLfloat* new_uv, GLfloat* new_pos)
{
	uint32_t new_offset = 0;
	uint32_t vert_offset = vertOffset;
	float x;
	float y;
	float z;
	unsigned int vertCount = 0;
	logger.debug("vert offset: %u", vertOffset);
	for(auto item: *vertexMap)
	{
		++vertCount;
		const VertexIndex& index = item.first;
		uint32_t new_index = item.second;
		//logger.debug("%u: %u/%u/%u > %u", vertCount, index.pos, index.uv, index.norm, new_index);

		// pos
		vert_offset = index.pos * STRIDE;
		new_offset = (new_index + vertOffset) * STRIDE;

		x = data.verts[vert_offset];
		++vert_offset;
		y = data.verts[vert_offset];
		++vert_offset;
		z = data.verts[vert_offset];


		//logger.debug("%u @ %u: (%f,%f,%f)", vertCount, vert_offset, x, y, z);

		if(x > m_maxX) m_maxX = x;
		if(x < m_minX) m_minX = x;
		if(y > m_maxY) m_maxY = y;
		if(y < m_minY) m_minY = y;
		if(z > m_maxZ) m_maxZ = z;
		if(z < m_minZ) m_minZ = z;
		new_pos[new_offset++] = x;
		new_pos[new_offset++] = y;
		new_pos[new_offset] = z;

		// norm
		vert_offset = index.pos * STRIDE;
		new_offset = (new_index + vertOffset) * STRIDE;
		// store empty normal
		// TODO: actual normal storage if present in BlendData
		new_norm[new_offset] = .5f;
		new_norm[new_offset] = .5f;
		new_norm[new_offset] = .5f;

		// uv
		if(data.uvs)
		{
			vert_offset = index.pos * UVSTRIDE;
			new_offset = (new_index + vertOffset) * UVSTRIDE;
			new_uv[new_offset++] = data.uvs[vert_offset++];
			new_uv[new_offset++] = data.uvs[vert_offset++];
		}

	}

	logger.debug("BB: %f:%f %f:%f %f:%f", m_minX, m_maxX, m_minY, m_maxY, m_minZ, m_maxZ);
}
