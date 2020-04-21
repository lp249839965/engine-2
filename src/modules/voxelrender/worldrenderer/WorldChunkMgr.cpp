/**
 * @file
 */

#include "WorldChunkMgr.h"
#include "core/Trace.h"
#include "voxel/Constants.h"

namespace voxelrender {

WorldChunkMgr::WorldChunkMgr(core::ThreadPool& threadPool) :
		_octree({}, 30), _threadPool(threadPool) {
}

void WorldChunkMgr::updateViewDistance(float viewDistance) {
	const glm::vec3 cullingThreshold(_meshExtractor.meshSize());
	const int maxCullingThreshold = core_max(cullingThreshold.x, cullingThreshold.z) * 4;
	_maxAllowedDistance = glm::pow(viewDistance + maxCullingThreshold, 2);
}

bool WorldChunkMgr::init(voxel::PagedVolume* volume) {
	if (!_meshExtractor.init(volume)) {
		Log::error("Failed to initialize the mesh extractor");
		return false;
	}
	return true;
}

void WorldChunkMgr::shutdown() {
	_meshExtractor.shutdown();
}

void WorldChunkMgr::reset() {
	for (ChunkBuffer& chunkBuffer : _chunkBuffers) {
		chunkBuffer.inuse = false;
	}
	_meshExtractor.reset();
	_octree.clear();
	_activeChunkBuffers = 0;
}

void WorldChunkMgr::handleMeshQueue() {
	voxel::Mesh mesh;
	if (!_meshExtractor.pop(mesh)) {
		return;
	}
	// Now add the mesh to the list of meshes to render.
	core_trace_scoped(WorldRendererHandleMeshQueue);

	ChunkBuffer* freeChunkBuffer = nullptr;
	for (ChunkBuffer& chunkBuffer : _chunkBuffers) {
		if (freeChunkBuffer == nullptr && !chunkBuffer.inuse) {
			freeChunkBuffer = &chunkBuffer;
		}
		// check whether we update an existing one
		if (chunkBuffer.translation() == mesh.getOffset()) {
			freeChunkBuffer = &chunkBuffer;
			break;
		}
	}

	if (freeChunkBuffer == nullptr) {
		Log::warn("Could not find free chunk buffer slot");
		return;
	}

	freeChunkBuffer->mesh = std::move(mesh);
	freeChunkBuffer->_aabb = {freeChunkBuffer->mesh.mins(), freeChunkBuffer->mesh.maxs()};
	if (!_octree.insert(freeChunkBuffer)) {
		Log::warn("Failed to insert into octree");
	}
	if (!freeChunkBuffer->inuse) {
		freeChunkBuffer->inuse = true;
		++_activeChunkBuffers;
	}
}

static inline size_t transform(size_t indexOffset, const voxel::Mesh& mesh, voxel::VertexArray& verts, voxel::IndexArray& idxs) {
	const voxel::IndexArray& indices = mesh.getIndexVector();
	const size_t start = idxs.size();
	idxs.insert(idxs.end(), indices.begin(), indices.end());
	const size_t end = idxs.size();
	for (size_t i = start; i < end; ++i) {
		idxs[i] += indexOffset;
	}
	const voxel::VertexArray& vertices = mesh.getVertexVector();
	verts.insert(verts.end(), vertices.begin(), vertices.end());
	return vertices.size();
}

void WorldChunkMgr::update(const video::Camera &camera, const glm::vec3& focusPos) {
	handleMeshQueue();
	cull(camera);

	_meshExtractor.updateExtractionOrder(focusPos);
	for (ChunkBuffer& chunkBuffer : _chunkBuffers) {
		if (!chunkBuffer.inuse) {
			continue;
		}
		const int distance = distance2(chunkBuffer.translation(), focusPos);
		if (distance < _maxAllowedDistance) {
			continue;
		}
		core_assert_always(_meshExtractor.allowReExtraction(chunkBuffer.translation()));
		chunkBuffer.inuse = false;
		--_activeChunkBuffers;
		_octree.remove(&chunkBuffer);
		Log::trace("Remove mesh from %i:%i", chunkBuffer.translation().x, chunkBuffer.translation().z);
	}
}

void WorldChunkMgr::extractScheduledMesh() {
	_meshExtractor.extractScheduledMesh();
}

// TODO: put into background task with two states - computing and
// next - then the indices and vertices are just swapped
void WorldChunkMgr::cull(const video::Camera& camera) {
	if (_cullResult.valid()) {
		if (_cullResult.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) {
			BufferIndices indices = _cullResult.get();
			_nextBuffer = indices.next;
			_currentBuffer = indices.current;
			_cullResult = std::future<BufferIndices>();
		} else {
			// already culling - but not yet ready
			return;
		}
	}
	core_trace_scoped(WorldRendererCull);

	math::AABB<float> aabb = camera.frustum().aabb();
	// don't cull objects that might cast shadows
	aabb.shift(camera.forward() * -10.0f);

	voxel::IndexArray& indices = _indices[_nextBuffer];
	voxel::VertexArray& vertices = _vertices[_nextBuffer];
	_cullResult = _threadPool.enqueue([aabb, &indices, &vertices, this] () {
		indices.clear();
		vertices.clear();
		size_t indexOffset = 0;
		Tree::Contents contents;
		_octree.query(math::AABB<int>(aabb.mins(), aabb.maxs()), contents);

		for (ChunkBuffer* chunkBuffer : contents) {
			core_trace_scoped(WorldRendererCullChunk);
			indexOffset += transform(indexOffset, chunkBuffer->mesh, vertices, indices);
		}

		return BufferIndices{_nextBuffer, (_nextBuffer + 1) % BufferCount};
	});
}

int WorldChunkMgr::distance2(const glm::ivec3& pos, const glm::ivec3& pos2) const {
	// we are only taking the x and z axis into account here
	const glm::ivec2 dist(pos.x - pos2.x, pos.z - pos2.z);
	const int distance = dist.x * dist.x + dist.y * dist.y;
	return distance;
}

void WorldChunkMgr::extractMeshes(const video::Camera& camera) {
	core_trace_scoped(WorldRendererExtractMeshes);

	const float farplane = camera.farPlane();

	glm::vec3 mins = camera.position();
	mins.x -= farplane;
	mins.y = 0;
	mins.z -= farplane;

	glm::vec3 maxs = camera.position();
	maxs.x += farplane;
	maxs.y = voxel::MAX_HEIGHT;
	maxs.z += farplane;

	_octree.visit(mins, maxs, [&] (const glm::ivec3& mins, const glm::ivec3& maxs) {
		return !_meshExtractor.scheduleMeshExtraction(mins);
	}, glm::vec3(_meshExtractor.meshSize()));
}

void WorldChunkMgr::extractMesh(const glm::ivec3& pos) {
	_meshExtractor.scheduleMeshExtraction(pos);
}

}