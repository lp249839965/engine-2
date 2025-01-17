/**
 * @file
 */

#include "VoxOldFormat.h"
#include "core/Color.h"
#include "core/Log.h"
#include "voxel/MaterialColor.h"

#define wrap(read) \
	if ((read) != 0) { \
		Log::debug("Error: " CORE_STRINGIFY(read) " at " SDL_FILE ":%i", SDL_LINE); \
		return false; \
	}

#define wrapBool(read) \
	if (!(read)) { \
		Log::debug("Error: " CORE_STRINGIFY(read) " at " SDL_FILE ":%i", SDL_LINE); \
		return false; \
	}

namespace voxel {

bool VoxOldFormat::loadGroups(const core::String &filename, io::SeekableReadStream &stream, VoxelVolumes &volumes) {
	uint32_t depth, height, width;
	wrap(stream.readUInt32(depth))
	wrap(stream.readUInt32(height))
	wrap(stream.readUInt32(width))

	if (width > 2048 || height > 2048 || depth > 2048) {
		Log::error("Volume exceeds the max allowed size: %i:%i:%i", width, height, depth);
		return false;
	}

	const voxel::Region region(0, 0, 0, (int)width - 1, (int)height - 1, (int)depth - 1);
	if (!region.isValid()) {
		Log::error("Invalid region: %i:%i:%i", width, height, depth);
		return false;
	}
	RawVolume *volume = new RawVolume(region);
	volumes.push_back(VoxelVolume{volume, filename, true});

	const MaterialColorArray& materialColors = getMaterialColors();

	const int64_t voxelPos = stream.pos();
	stream.skip((int64_t)width * height * depth);

	for (int i = 0; i < 256; i++) {
		uint8_t r, g, b;
		wrap(stream.readUInt8(r))
		wrap(stream.readUInt8(g))
		wrap(stream.readUInt8(b))

		const glm::vec4 color = core::Color::fromRGBA(r, g, b, 255);
		_colors[i] = core::Color::getRGBA(color);
		_palette[i] = core::Color::getClosestMatch(color, materialColors);
	}

	stream.seek(voxelPos);
	for (uint32_t h = 0u; h < height; ++h) {
		for (uint32_t d = 0u; d < depth; ++d) {
			for (uint32_t w = 0u; w < width; ++w) {
				uint8_t palIdx;
				wrap(stream.readUInt8(palIdx))
				if (palIdx == 255) {
					continue;
				}
				const voxel::Voxel& voxel = voxel::createVoxel(voxel::VoxelType::Generic, _palette[palIdx]);
				// we have to flip depth with height for our own coordinate system
				volume->setVoxel((int)w, (int)h, (int)d, voxel);
			}
		}
	}
	return true;
}

#undef wrap
#undef wrapBool

} // namespace voxel
