/**
 * @file
 */

#pragma once

#include "Format.h"

namespace voxel {

/**
 * @brief Qubicle Binary (qb) format.
 *
 * https://getqubicle.com/qubicle/documentation/docs/file/qb/
 */
class QBFormat : public Format {
private:
	enum class ColorFormat : uint32_t {
		RGBA = 0,
		BGRA = 1
	};
	enum class ZAxisOrientation : uint32_t {
		Left = 0,
		Right = 1
	};
	enum class Compression : uint32_t {
		None = 0,
		RLE = 1
	};

	// If set to 0 the A value of RGBA or BGRA is either 0 (invisble voxel) or 255 (visible voxel).
	// If set to 1 the visibility mask of each voxel is encoded into the A value telling your software
	// which sides of the voxel are visible. You can save a lot of render time using this option.
	enum class VisibilityMask : uint32_t {
		AlphaChannelVisibleByValue,
		AlphaChannelVisibleSidesEncoded
	};
	struct State {
		uint32_t _version;
		ColorFormat _colorFormat;
		ZAxisOrientation _zAxisOrientation;
		Compression _compressed;
		VisibilityMask _visibilityMaskEncoded;
	};
	// left shift values for the vis mask for the single faces
	enum class VisMaskSides : uint8_t {
		Invisble,
		Left,
		Right,
		Top,
		Bottom,
		Front,
		Back
	};

	bool setVoxel(State& state, voxel::RawVolume* volume, uint32_t x, uint32_t y, uint32_t z, const glm::ivec3& offset, const voxel::Voxel& voxel);
	voxel::Voxel getVoxel(State& state, io::SeekableReadStream& stream);
	bool loadMatrix(State& state, io::SeekableReadStream& stream, VoxelVolumes& volumes);
	bool loadFromStream(io::SeekableReadStream& stream, VoxelVolumes& volumes);

	bool saveMatrix(io::SeekableWriteStream& stream, const VoxelVolume& volume) const;
public:
	bool loadGroups(const core::String &filename, io::SeekableReadStream& stream, VoxelVolumes& volumes) override;
	bool saveGroups(const VoxelVolumes& volumes, const core::String &filename, io::SeekableWriteStream& stream) override;
};

}
