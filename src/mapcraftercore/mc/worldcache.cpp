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

#include "worldcache.h"

#include "blockstate.h"

namespace mapcrafter {
namespace mc {

template <typename Key, typename Value, int kBits>
class PositionCache {
private:
	static constexpr int kWidth = 1 << kBits;
	static constexpr int kNumBuckets = kWidth * kWidth;

	struct Bucket {
		static constexpr int kDefaultValue = std::numeric_limits<int>::max();
		Key key{kDefaultValue, kDefaultValue};
		std::unique_ptr<Value> value;
	};

	std::array<Bucket, kNumBuckets> data;

	static constexpr int IndexForKey(const Key& key) {
		constexpr uint32_t kMask = static_cast<uint32_t>(kWidth) - 1;
	    return ((key.x & kMask) << kBits) | (key.z & kMask);
	}

	std::string DebugString(const Key& key) {
		return std::to_string(key.x) + "," + std::to_string(key.z);
	}

public:
	bool Get(const Key& key, Value** value) {
		Bucket& bucket = data[IndexForKey(key)];
		if (bucket.key == key) {
			*value = bucket.value.get();
			return true;
		} else {
			return false;
		}
	}
	void Put(const Key& key, std::unique_ptr<Value> value) {
		Bucket& bucket = data[IndexForKey(key)];
		bucket.key = key;
		bucket.value = std::move(value);
	}
};


Block::Block()
//	: Block(mc::BlockPos(0, 0, 0), 0, 0) { /* gcc 4.4 being stupid :/ */
	: pos(mc::BlockPos(0, 0, 0)), id(0), biome(0),
	  block_light(0), sky_light(15), fields_set(0) {
}

Block::Block(const mc::BlockPos& pos, uint16_t id)
	: pos(pos), id(id), biome(0),
	  block_light(0), sky_light(15), fields_set(GET_ID) {
}

WorldCache::WorldCache(mc::BlockStateRegistry& block_registry, const World& world)
	: block_registry(block_registry),
	  world(world),
	  regioncache(std::make_unique<RegionCache>()),
	  chunkcache(std::make_unique<ChunkCache>()) {}

WorldCache::~WorldCache() {
	regionstats.print("Region Cache");
	chunkstats.print("Chunk Cache");
}

const World& WorldCache::getWorld() const {
	return world;
}

RegionFile* WorldCache::getRegion(const RegionPos& pos) {
	RegionFile* region_ptr;
	if (regioncache->Get(pos, &region_ptr)) {
		regionstats.hits++;
		return region_ptr;
	}

	auto region = std::make_unique<RegionFile>();
	// region does not exist, region in cache was not modified
	if (!world.getRegion(pos, *region)) {
		regioncache->Put(pos, nullptr);
		regionstats.region_not_found++;
		return nullptr;
	}

	if (!region->read()) {
		regioncache->Put(pos, nullptr);
		regionstats.invalid++;
		return nullptr;
	}

	regionstats.misses++;
	region_ptr = region.get();
	regioncache->Put(pos, std::move(region));
	return region_ptr;
}

Chunk* WorldCache::getChunk(const ChunkPos& pos) {
	Chunk* chunk_ptr;
	if (chunkcache->Get(pos, &chunk_ptr)) {
		chunkstats.hits++;
		return chunk_ptr;
	}

	// if not try to get the region of the chunk from the cache
	RegionFile* region = getRegion(pos.getRegion());
	if (region == nullptr) {
		chunkstats.region_not_found++;
		return nullptr;
	}

	auto chunk = std::make_unique<Chunk>();
	int status = region->loadChunk(pos, block_registry, *chunk);
	// the chunk does not exist, chunk in cache was not modified
	if (status == RegionFile::CHUNK_DOES_NOT_EXIST) {
		chunkcache->Put(pos, nullptr);
		chunkstats.not_found++;
		return nullptr;
	}

	if (status != RegionFile::CHUNK_OK) {
		chunkcache->Put(pos, nullptr);
		// the chunk is not valid, chunk in cache was probably modified
		chunkstats.invalid++;
		return nullptr;
	}

	chunkstats.misses++;
	chunk_ptr = chunk.get();
	chunkcache->Put(pos, std::move(chunk));
	return chunk_ptr;
}

Block WorldCache::getBlock(const mc::BlockPos& pos, const mc::Chunk* chunk, int get) {
	// this can happen when we check for the bottom block shadow edges
	if (pos.y < 0)
		return Block();

	mc::ChunkPos chunk_pos(pos);
	const mc::Chunk* mychunk = chunk;
	if (chunk == nullptr || chunk_pos != chunk->getPos())
		mychunk = getChunk(chunk_pos);
	// chunk may be nullptr
	if (mychunk == nullptr) {
		return Block();
	// otherwise get all required block data
	} else {
		mc::LocalBlockPos local(pos);
		Block block;
		block.pos = pos;
		if (get & GET_ID) {
			block.id = mychunk->getBlockID(local);
			block.fields_set |= GET_ID;
		}
		if (get & GET_BIOME) {
			block.biome = mychunk->getBiomeAt(local);
			block.fields_set |= GET_BIOME;
		}
		if (get & GET_BLOCK_LIGHT) {
			block.block_light = mychunk->getBlockLight(local);
			block.fields_set |= GET_BLOCK_LIGHT;
		}
		if (get & GET_SKY_LIGHT) {
			block.sky_light = mychunk->getSkyLight(local);
			block.fields_set |= GET_SKY_LIGHT;
		}
		return block;
	}
}

const CacheStats& WorldCache::getRegionCacheStats() const {
	return regionstats;
}

const CacheStats& WorldCache::getChunkCacheStats() const {
	return chunkstats;
}

}
}
