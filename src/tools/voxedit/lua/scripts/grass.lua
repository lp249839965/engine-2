local vol = require "modules.volume"

function arguments()
	return {
		{ name = 'height', desc = 'the height of the grass to add', type = 'int', default = '4' },
		{ name = 'density', desc = 'the density of the grass', type = 'int', default = '2' },
		{ name = 'grasscolor', desc = 'the palette index of the color to use as grass', type = 'int', min = '0', max = '255' }
	}
end

function main(volume, region, color, height, density, grasscolor)
	print("create grass with color " .. grasscolor .. " and in the height range 1.." .. height)
	local visitor = function (volume, x, y, z)
		local rndHeight = math.random(1, height)
		for h = 1, rndHeight do
			volume:setVoxel(x, y + h, z, grasscolor)
		end
	end

	local condition = function (volume, x, y, z)
		local voxel = volume:voxel(x, y, z)
		if voxel == color then
			return true
		end
		return false
	end
	vol.conditionYXZDown(volume, region, visitor, condition)
end