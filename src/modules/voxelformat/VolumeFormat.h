/**
 * @file
 */

#pragma once

#include "io/File.h"
#include "io/Stream.h"
#include "io/FormatDescription.h"
#include "VoxelVolumes.h"
#include "video/Texture.h"
#include "core/collection/Array.h"

namespace voxelformat {

#define VOX_FORMAT_FLAG_SCREENSHOT_EMBEDDED (1 << 0)
#define VOX_FORMAT_FLAG_PALETTE_EMBEDDED (1 << 1)
#define VOX_FORMAT_FLAG_MESH (1 << 2)

extern const io::FormatDescription SUPPORTED_VOXEL_FORMATS_LOAD[];
extern const char *SUPPORTED_VOXEL_FORMATS_LOAD_LIST[];
extern const io::FormatDescription SUPPORTED_VOXEL_FORMATS_SAVE[];

extern size_t loadPalette(const core::String &fileName, io::SeekableReadStream& stream, core::Array<uint32_t, 256> &palette);
extern image::ImagePtr loadScreenshot(const core::String &fileName, io::SeekableReadStream& stream);
extern bool loadFormat(const core::String &fileName, io::SeekableReadStream& stream, voxel::VoxelVolumes& newVolumes);

/**
 * @brief Save both to volume or to mesh - depends on the given file extension
 */
extern bool saveFormat(const io::FilePtr& filePtr, voxel::VoxelVolumes& volumes);

extern bool isMeshFormat(const core::String& filename);

}
