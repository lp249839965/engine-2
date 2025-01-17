/**
 * @file
 */

#include "CSMFormat.h"
#include "core/Color.h"
#include "core/FourCC.h"
#include "core/Log.h"
#include "glm/common.hpp"
#include "voxel/MaterialColor.h"
#include "voxel/Voxel.h"

namespace voxel {

#define wrap(read) \
	if ((read) != 0) { \
		Log::error("Could not load csm file: Not enough data in stream " CORE_STRINGIFY(read)); \
		return false; \
	}

#define wrapBool(read) \
	if (!(read)) { \
		Log::debug("Error: " CORE_STRINGIFY(read) " at " SDL_FILE ":%i", SDL_LINE); \
		return false; \
	}

static bool readString(io::SeekableReadStream& stream, core::String& str, bool readStringAsInt) {
	if (readStringAsInt) {
		uint32_t length;
		wrap(stream.readUInt32(length))
		if (length > 4096) {
			// sanity check
			return false;
		}
		char name[4096];
		wrapBool(stream.readString(length, name))
		name[length] = '\0';
		str = name;
	} else {
		uint8_t length;
		wrap(stream.readUInt8(length))
		char name[256];
		wrapBool(stream.readString(length, name))
		name[length] = '\0';
		str = name;
	}
	return true;
}

bool CSMFormat::loadGroups(const core::String &filename, io::SeekableReadStream &stream, VoxelVolumes &volumes) {
	const MaterialColorArray& materialColors = getMaterialColors();
	uint32_t magic, version, blank, matrixCount;
	wrap(stream.readUInt32(magic))
	const bool isNVM = magic == FourCC('.','N','V','M');
	wrap(stream.readUInt32(version))
	wrap(stream.readUInt32(blank))
	wrap(stream.readUInt32(matrixCount))

	if (isNVM && version > 2) {
		Log::warn("nvm is only supported up to version 2");
	}
	if (!isNVM && version > 4) {
		Log::warn("csm is only supported up to version 4");
	}

	const bool readStringAsInt = isNVM || version >= 4;

	for (uint16_t i = 0u; (uint16_t)i < matrixCount; ++i) {
		core::String name;
		core::String parent;
		wrapBool(readString(stream, name, readStringAsInt))
		if (version > 1) {
			wrapBool(readString(stream, parent, readStringAsInt))
		}
		uint16_t posx, posy, posz;
		wrap(stream.readUInt16(posx))
		wrap(stream.readUInt16(posy))
		wrap(stream.readUInt16(posz))

		uint16_t sizex, sizey, sizez;
		wrap(stream.readUInt16(sizex))
		wrap(stream.readUInt16(sizey))
		wrap(stream.readUInt16(sizez))

		if (sizex > MaxRegionSize || sizey > MaxRegionSize || sizez > MaxRegionSize) {
			Log::error("Volume exceeds the max allowed size: %i:%i:%i", sizex, sizey, sizez);
			return false;
		}

		const voxel::Region region(0, 0, 0, sizex - 1, sizey - 1, sizez - 1);
		if (!region.isValid()) {
			Log::error("Invalid region: %i:%i:%i", sizex, sizey, sizez);
			return false;
		}

		uint32_t voxels = sizex * sizey * sizez;
		uint32_t matrixIndex = 0u;

		RawVolume *volume = new RawVolume(region);
		volumes.push_back(VoxelVolume{volume, name, true});

		while (matrixIndex < voxels) {
			uint8_t count;
			wrap(stream.readUInt8(count))
			uint8_t r;
			wrap(stream.readUInt8(r))
			uint8_t g;
			wrap(stream.readUInt8(g))
			uint8_t b;
			wrap(stream.readUInt8(b))
			uint8_t interactiontype;
			wrap(stream.readUInt8(interactiontype))
			if (interactiontype == 0u) {
				matrixIndex += count;
				continue;
			}
			const glm::vec4& color = core::Color::fromRGBA(r, g, b, 255);
			const int index = core::Color::getClosestMatch(color, materialColors);
			const voxel::Voxel& voxel = voxel::createVoxel(voxel::VoxelType::Generic, index);

			for (uint32_t v = matrixIndex; v < matrixIndex + count; ++v) {
				const int x = glm::mod((float)glm::floor((float)v / (float)(sizez * sizey)), (float)sizex);
				const int y = glm::mod((float)glm::floor((float)v / (float)(sizez)), (float)sizey);
				const int z = glm::mod((float)v, (float)sizez);
				volume->setVoxel(x, y, z, voxel);
			}

			matrixIndex += count;
		}
	}
	return true;
}

#undef wrap
#undef wrapBool

bool CSMFormat::saveGroups(const VoxelVolumes &volumes, const core::String &filename, io::SeekableWriteStream& stream) {
	return false;
}

}
