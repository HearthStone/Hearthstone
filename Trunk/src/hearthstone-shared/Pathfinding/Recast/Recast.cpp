/***
 * Demonstrike Core
 */

#include "../../Common.h"

float rcSqrt(float x)
{
	return sqrtf(x);
}

void rcIntArray::resize(int n)
{
	if (n > m_cap)
	{
		if (!m_cap) m_cap = n;
		while (m_cap < n) m_cap *= 2;
		int* newData = (int*)malloc(m_cap*sizeof(int));
		if (m_size && newData) memcpy(newData, m_data, m_size*sizeof(int));
		free(m_data);
		m_data = newData;
	}
	m_size = n;
}

void rcContext::log(const rcLogCategory category, const char* format, ...)
{
	if (!m_logEnabled)
		return;
	static const int MSG_SIZE = 512;
	char msg[MSG_SIZE];
	va_list ap;
	va_start(ap, format);
	int len = vsnprintf(msg, MSG_SIZE, format, ap);
	if (len >= MSG_SIZE)
	{
		len = MSG_SIZE-1;
		msg[MSG_SIZE-1] = '\0';
	}
	va_end(ap);
	doLog(category, msg, len);
}

rcHeightfield* mallocHeightfield()
{
	rcHeightfield* hf = (rcHeightfield*)malloc(sizeof(rcHeightfield));
	memset(hf, 0, sizeof(rcHeightfield));
	return hf;
}

void freeHeightField(rcHeightfield* hf)
{
	if (!hf) return;
	// Delete span array.
	free(hf->spans);
	// Delete span pools.
	while (hf->pools)
	{
		rcSpanPool* next = hf->pools->next;
		free(hf->pools);
		hf->pools = next;
	}
	free(hf);
}

rcCompactHeightfield* mallocCompactHeightfield()
{
	rcCompactHeightfield* chf = (rcCompactHeightfield*)malloc(sizeof(rcCompactHeightfield));
	memset(chf, 0, sizeof(rcCompactHeightfield));
	return chf;
}

void freeCompactHeightfield(rcCompactHeightfield* chf)
{
	if (!chf) return;
	free(chf->cells);
	free(chf->spans);
	free(chf->dist);
	free(chf->areas);
	free(chf);
}

rcContourSet* mallocContourSet()
{
	rcContourSet* cset = (rcContourSet*)malloc(sizeof(rcContourSet));
	memset(cset, 0, sizeof(rcContourSet));
	return cset;
}

void freeContourSet(rcContourSet* cset)
{
	if (!cset) return;
	for (int i = 0; i < cset->nconts; ++i)
	{
		free(cset->conts[i].verts);
		free(cset->conts[i].rverts);
	}
	free(cset->conts);
	free(cset);
}

rcPolyMesh* mallocPolyMesh()
{
	rcPolyMesh* pmesh = (rcPolyMesh*)malloc(sizeof(rcPolyMesh));
	memset(pmesh, 0, sizeof(rcPolyMesh));
	return pmesh;
}

void freePolyMesh(rcPolyMesh* pmesh)
{
	if (!pmesh) return;
	free(pmesh->verts);
	free(pmesh->polys);
	free(pmesh->regs);
	free(pmesh->flags);
	free(pmesh->areas);
	free(pmesh);
}

rcPolyMeshDetail* mallocPolyMeshDetail()
{
	rcPolyMeshDetail* dmesh = (rcPolyMeshDetail*)malloc(sizeof(rcPolyMeshDetail));
	memset(dmesh, 0, sizeof(rcPolyMeshDetail));
	return dmesh;
}

void freePolyMeshDetail(rcPolyMeshDetail* dmesh)
{
	if (!dmesh) return;
	free(dmesh->meshes);
	free(dmesh->verts);
	free(dmesh->tris);
	free(dmesh);
}


void rcCalcBounds(const float* verts, int nv, float* bmin, float* bmax)
{
	// Calculate bounding box.
	rcVcopy(bmin, verts);
	rcVcopy(bmax, verts);
	for (int i = 1; i < nv; ++i)
	{
		const float* v = &verts[i*3];
		rcVmin(bmin, v);
		rcVmax(bmax, v);
	}
}

void rcCalcGridSize(const float* bmin, const float* bmax, float cs, int* w, int* h)
{
	*w = (int)((bmax[0] - bmin[0])/cs+0.5f);
	*h = (int)((bmax[2] - bmin[2])/cs+0.5f);
}

bool rcCreateHeightfield(rcContext* /*ctx*/, rcHeightfield& hf, int width, int height,
						 const float* bmin, const float* bmax,
						 float cs, float ch)
{
	// TODO: VC complains about unref formal variable, figure out a way to handle this better.
//	ASSERT(ctx);
	
	hf.width = width;
	hf.height = height;
	rcVcopy(hf.bmin, bmin);
	rcVcopy(hf.bmax, bmax);
	hf.cs = cs;
	hf.ch = ch;
	hf.spans = (rcSpan**)malloc(sizeof(rcSpan*)*hf.width*hf.height);
	if (!hf.spans)
		return false;
	memset(hf.spans, 0, sizeof(rcSpan*)*hf.width*hf.height);
	return true;
}

static void calcTriNormal(const float* v0, const float* v1, const float* v2, float* norm)
{
	float e0[3], e1[3];
	rcVsub(e0, v1, v0);
	rcVsub(e1, v2, v0);
	rcVcross(norm, e0, e1);
	rcVnormalize(norm);
}

void rcMarkWalkableTriangles(rcContext* /*ctx*/, const float walkableSlopeAngle,
							 const float* verts, int /*nv*/,
							 const int* tris, int nt,
							 unsigned char* areas)
{
	// TODO: VC complains about unref formal variable, figure out a way to handle this better.
//	ASSERT(ctx);
	
	const float walkableThr = cosf(walkableSlopeAngle/180.0f*RC_PI);

	float norm[3];
	
	for (int i = 0; i < nt; ++i)
	{
		const int* tri = &tris[i*3];
		calcTriNormal(&verts[tri[0]*3], &verts[tri[1]*3], &verts[tri[2]*3], norm);
		// Check if the face is walkable.
		if (norm[1] > walkableThr)
			areas[i] = RC_WALKABLE_AREA;
	}
}

void rcClearUnwalkableTriangles(rcContext* /*ctx*/, const float walkableSlopeAngle,
								const float* verts, int /*nv*/,
								const int* tris, int nt,
								unsigned char* areas)
{
	// TODO: VC complains about unref formal variable, figure out a way to handle this better.
//	ASSERT(ctx);
	
	const float walkableThr = cosf(walkableSlopeAngle/180.0f*RC_PI);
	
	float norm[3];
	
	for (int i = 0; i < nt; ++i)
	{
		const int* tri = &tris[i*3];
		calcTriNormal(&verts[tri[0]*3], &verts[tri[1]*3], &verts[tri[2]*3], norm);
		// Check if the face is walkable.
		if (norm[1] <= walkableThr)
			areas[i] = RC_NULL_AREA;
	}
}

int rcGetHeightFieldSpanCount(rcContext* /*ctx*/, rcHeightfield& hf)
{
	// TODO: VC complains about unref formal variable, figure out a way to handle this better.
//	ASSERT(ctx);
	
	const int w = hf.width;
	const int h = hf.height;
	int spanCount = 0;
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			for (rcSpan* s = hf.spans[x + y*w]; s; s = s->next)
			{
				if (s->area != RC_NULL_AREA)
					spanCount++;
			}
		}
	}
	return spanCount;
}

bool rcBuildCompactHeightfield(rcContext* ctx, const int walkableHeight, const int walkableClimb,
							   rcHeightfield& hf, rcCompactHeightfield& chf)
{
	ASSERT(ctx);
	
	ctx->startTimer(RC_TIMER_BUILD_COMPACTHEIGHTFIELD);
	
	const int w = hf.width;
	const int h = hf.height;
	const int spanCount = rcGetHeightFieldSpanCount(ctx, hf);

	// Fill in header.
	chf.width = w;
	chf.height = h;
	chf.spanCount = spanCount;
	chf.walkableHeight = walkableHeight;
	chf.walkableClimb = walkableClimb;
	chf.maxRegions = 0;
	rcVcopy(chf.bmin, hf.bmin);
	rcVcopy(chf.bmax, hf.bmax);
	chf.bmax[1] += walkableHeight*hf.ch;
	chf.cs = hf.cs;
	chf.ch = hf.ch;
	chf.cells = (rcCompactCell*)malloc(sizeof(rcCompactCell)*w*h);
	if (!chf.cells)
	{
		ctx->log(RC_LOG_ERROR, "rcBuildCompactHeightfield: Out of memory 'chf.cells' (%d)", w*h);
		return false;
	}
	memset(chf.cells, 0, sizeof(rcCompactCell)*w*h);
	chf.spans = (rcCompactSpan*)malloc(sizeof(rcCompactSpan)*spanCount);
	if (!chf.spans)
	{
		ctx->log(RC_LOG_ERROR, "rcBuildCompactHeightfield: Out of memory 'chf.spans' (%d)", spanCount);
		return false;
	}
	memset(chf.spans, 0, sizeof(rcCompactSpan)*spanCount);
	chf.areas = (unsigned char*)malloc(sizeof(unsigned char)*spanCount);
	if (!chf.areas)
	{
		ctx->log(RC_LOG_ERROR, "rcBuildCompactHeightfield: Out of memory 'chf.areas' (%d)", spanCount);
		return false;
	}
	memset(chf.areas, RC_NULL_AREA, sizeof(unsigned char)*spanCount);
	
	const int MAX_HEIGHT = 0xffff;
	
	// Fill in cells and spans.
	int idx = 0;
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			const rcSpan* s = hf.spans[x + y*w];
			// If there are no spans at this cell, just leave the data to index=0, count=0.
			if (!s) continue;
			rcCompactCell& c = chf.cells[x+y*w];
			c.index = idx;
			c.count = 0;
			while (s)
			{
				if (s->area != RC_NULL_AREA)
				{
					const int bot = (int)s->smax;
					const int top = s->next ? (int)s->next->smin : MAX_HEIGHT;
					chf.spans[idx].y = (unsigned short)rcClamp(bot, 0, 0xffff);
					chf.spans[idx].h = (unsigned char)rcClamp(top - bot, 0, 0xff);
					chf.areas[idx] = s->area;
					idx++;
					c.count++;
				}
				s = s->next;
			}
		}
	}

	// Find neighbour connections.
	const int MAX_LAYERS = RC_NOT_CONNECTED-1;
	int tooHighNeighbour = 0;
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			const rcCompactCell& c = chf.cells[x+y*w];
			for (int i = (int)c.index, ni = (int)(c.index+c.count); i < ni; ++i)
			{
				rcCompactSpan& s = chf.spans[i];
				
				for (int dir = 0; dir < 4; ++dir)
				{
					rcSetCon(s, dir, RC_NOT_CONNECTED);
					const int nx = x + rcGetDirOffsetX(dir);
					const int ny = y + rcGetDirOffsetY(dir);
					// First check that the neighbour cell is in bounds.
					if (nx < 0 || ny < 0 || nx >= w || ny >= h)
						continue;
						
					// Iterate over all neighbour spans and check if any of the is
					// accessible from current cell.
					const rcCompactCell& nc = chf.cells[nx+ny*w];
					for (int k = (int)nc.index, nk = (int)(nc.index+nc.count); k < nk; ++k)
					{
						const rcCompactSpan& ns = chf.spans[k];
						const int bot = rcMax(s.y, ns.y);
						const int top = rcMin(s.y+s.h, ns.y+ns.h);

						// Check that the gap between the spans is walkable,
						// and that the climb height between the gaps is not too high.
						if ((top - bot) >= walkableHeight && rcAbs((int)ns.y - (int)s.y) <= walkableClimb)
						{
							// Mark direction as walkable.
							const int idx = k - (int)nc.index;
							if (idx < 0 || idx > MAX_LAYERS)
							{
								tooHighNeighbour = rcMax(tooHighNeighbour, idx);
								continue;
							}
							rcSetCon(s, dir, idx);
							break;
						}
					}
					
				}
			}
		}
	}
	
	if (tooHighNeighbour > MAX_LAYERS)
	{
		ctx->log(RC_LOG_ERROR, "rcBuildCompactHeightfield: Heightfield has too many layers %d (max: %d)",
				 tooHighNeighbour, MAX_LAYERS);
	}
		
	ctx->stopTimer(RC_TIMER_BUILD_COMPACTHEIGHTFIELD);
	
	return true;
}

/*
static int getHeightfieldMemoryUsage(const rcHeightfield& hf)
{
	int size = 0;
	size += sizeof(hf);
	size += hf.width * hf.height * sizeof(rcSpan*);
	
	rcSpanPool* pool = hf.pools;
	while (pool)
	{
		size += (sizeof(rcSpanPool) - sizeof(rcSpan)) + sizeof(rcSpan)*RC_SPANS_PER_POOL;
		pool = pool->next;
	}
	return size;
}

static int getCompactHeightFieldMemoryusage(const rcCompactHeightfield& chf)
{
	int size = 0;
	size += sizeof(rcCompactHeightfield);
	size += sizeof(rcCompactSpan) * chf.spanCount;
	size += sizeof(rcCompactCell) * chf.width * chf.height;
	return size;
}
*/