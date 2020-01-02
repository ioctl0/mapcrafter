/*
 * Copyright 2012-2016 Moritz Hilscher
 *
 * This file is part of Mapcrafter.
 *
 * Mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WORLDCACHE_H_
#define WORLDCACHE_H_

#include "chunk.h"
#include "pos.h"
#include "region.h"
#include "world.h"

#include <optional>
#include <set>

namespace mapcrafter {
namespace mc {

class BlockStateRegistry;

/**
 * A block with id/data/biome/lighting data.
 */
struct Block {
	Block();
	Block(const mc::BlockPos& pos, uint16_t id);

	// which block does this data belong to (set by getBlock-method)
	mc::BlockPos pos;
	uint16_t id;
	uint8_t biome;
	uint8_t block_light, sky_light;
	// which of the fields above are set? (set by getBlock-method)
	int fields_set;
};

const int GET_ID = 1;
// obsolete
//const int GET_DATA = 2; 
const int GET_BIOME = 4;
const int GET_BLOCK_LIGHT = 8;
const int GET_SKY_LIGHT = 16;
const int GET_LIGHT = GET_BLOCK_LIGHT | GET_SKY_LIGHT;

/**
 * Some cache statistics for debugging. Not used at the moment.
 *
 * Maybe add a set of corrupt chunks/regions to dump them at the end of the rendering.
 */
struct CacheStats {
	CacheStats() {}

	void print(const std::string& name) const {
		std::cout << name << ":" << std::endl;
		std::cout << "  hits: " << hits << std::endl
				  << "  misses: " << misses << std::endl
				  << "  region_not_found: " << region_not_found << std::endl
				  << "  not_found: " << not_found << std::endl
				  << "  unavailable: " << unavailable << std::endl
				  << "  invalid: " << invalid << std::endl;
	}

	int hits = 0;
	int misses = 0;

	int region_not_found = 0;
	int not_found = 0;
	int invalid = 0;
	int unavailable = 0;
};

template <typename Key, typename Value, int kBits>
class PositionCache;

/**
 * This is a world cache with regions and chunks.
 *
 * Every region and chunk has a fixed position in the cache. The position in the cache is
 * calculated by using the first few bits of the region/chunk coordinates. Then the
 * regions/chunks are stored with the "smaller" coordinates in a 2D-like array.
 *
 * For the regions the first 2 bits are used, this are 4x4 regions (4 = 1 << 2) in the
 * cache. The regions store only the raw region file data and are used to read the chunks
 * when necessary.
 *
 * For the chunks the first 5 bits are used, this are 16x16 chunk (also the size of a
 * region) in the cache.
 *
 * When someone is trying to access the cache, the cache calculates the position of a
 * region/chunk coordinate in the cache. Then the cache checks if there is already
 * something stored on this position and if the real coordinate of this cache entry is
 * the coordinate of the requested region/chunk. If yes, the cache returns the objects.
 * If not, the cache tries to load the chunk/region and puts it in this cache entry
 * (overwrites an already loaded region/chunk at this cache position).
 */
class WorldCache {
private:
	mc::BlockStateRegistry& block_registry;
	World world;

	using RegionCache = PositionCache<RegionPos, RegionFile, 2>;
	using ChunkCache = PositionCache<ChunkPos, Chunk, 5>;
	std::unique_ptr<RegionCache> regioncache;
	std::unique_ptr<ChunkCache> chunkcache;

	CacheStats regionstats;
	CacheStats chunkstats;

public:
	WorldCache(mc::BlockStateRegistry& block_registry, const World& world);
	~WorldCache();

	const World& getWorld() const;

	RegionFile* getRegion(const RegionPos& pos);
	Chunk* getChunk(const ChunkPos& pos);

	Block getBlock(const mc::BlockPos& pos, const mc::Chunk* chunk, int get = GET_ID);

	const CacheStats& getRegionCacheStats() const;
	const CacheStats& getChunkCacheStats() const;
};

}
}

#endif /* WORLDCACHE_H_ */
