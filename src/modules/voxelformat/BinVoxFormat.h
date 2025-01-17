/**
 * @file
 */

#pragma once

#include "Format.h"

namespace voxel {

/**
 * @brief BinVox (binvox) format.
 *
 * https://www.patrickmin.com/binvox/binvox.html
 */
class BinVoxFormat : public Format {
private:
	struct State {
		uint32_t _version = 0u;
		uint32_t _w = 0u;
		uint32_t _h = 0u;
		uint32_t _d = 0u;
		uint32_t _size = 0u;
		float _tx = 0.0f;
		float _ty = 0.0f;
		float _tz = 0.0f;
		float _scale = 0.0f;
	};

	bool readData(State& state, const core::String& filename, io::SeekableReadStream& stream, VoxelVolumes& volumes);
public:
	bool loadGroups(const core::String& filename, io::SeekableReadStream& stream, VoxelVolumes& volumes) override;
	bool saveGroups(const VoxelVolumes& volumes, const core::String &filename, io::SeekableWriteStream& stream) override;
};

}
