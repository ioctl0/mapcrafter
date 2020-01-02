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

#include "pos.h"

#include "chunk.h"
#include "../util.h"

#include <cmath>
#include <cstdio>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace mc {

RegionPos RegionPos::byFilename(const std::string& filename) {
	std::string name = BOOST_FS_FILENAME(fs::path(filename));

	int x, z;
	if (sscanf(name.c_str(), "r.%d.%d.mca", &x, &z) != 2)
		throw std::runtime_error("Invalid filename " + name + "!");
	return RegionPos(x, z);
}

void RegionPos::rotate(int count) {
	for (int i = 0; i < count; i++) {
		int nx = -z, nz = x;
		x = nx;
		z = nz;
	}
}

ChunkPos::ChunkPos(const BlockPos& block)
	: x(util::floordiv(block.x, 16)),
	  z(util::floordiv(block.z, 16)) {}

int ChunkPos::getLocalX() const {
	return x % 32 < 0 ? x % 32 + 32 : x % 32;
}
int ChunkPos::getLocalZ() const {
	return z % 32 < 0 ? z % 32 + 32 : z % 32;
}

RegionPos ChunkPos::getRegion() const {
	return RegionPos(util::floordiv(x, 32), util::floordiv(z, 32));
}

int ChunkPos::getRow() const {
	return z - x;
}

int ChunkPos::getCol() const {
	return x + z;
}

ChunkPos ChunkPos::byRowCol(int row, int col) {
	return ChunkPos((col - row) / 2, (col + row) / 2);
}

void ChunkPos::rotate(int count) {
	for (int i = 0; i < count; i++) {
		int nx = 31 - z;
		z = x;
		x = nx;
	}
}

int BlockPos::getRow() const {
	return z - x + (CHUNK_HEIGHT*16 - y) * 4;
}


extern const mc::BlockPos DIR_NORTH(0, -1, 0);
extern const mc::BlockPos DIR_SOUTH(0, 1, 0);
extern const mc::BlockPos DIR_EAST(1, 0, 0);
extern const mc::BlockPos DIR_WEST(-1, 0, 0);
extern const mc::BlockPos DIR_TOP(0, 0, 1);
extern const mc::BlockPos DIR_BOTTOM(0, 0, -1);

LocalBlockPos::LocalBlockPos(int x, int z, int y)
	: x(x), z(z), y(y) {
}

LocalBlockPos::LocalBlockPos(const BlockPos& pos)
		: x(pos.x % 16), z(pos.z % 16), y(pos.y) {
	if (x < 0)
		x += 16;
	if (z < 0)
		z += 16;
}

int LocalBlockPos::getRow() const {
	return z - x + (CHUNK_HEIGHT*16 - y) * 4;
}

int LocalBlockPos::getCol() const {
	return x + z;
}

BlockPos LocalBlockPos::toGlobalPos(const ChunkPos& chunk) const {
	return BlockPos(x + chunk.x * 16, z + chunk.z * 16, y);
}

bool LocalBlockPos::operator<(const LocalBlockPos& other) const {
	if (y == other.y) {
		if (x == other.x)
			return z < other.z;
		return x > other.x;
	}
	return y < other.y;
}

std::ostream& operator<<(std::ostream& stream, const RegionPos& region) {
	stream << region.x << ":" << region.z;
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const ChunkPos& chunk) {
	stream << chunk.x << ":" << chunk.z;
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const BlockPos& block) {
	stream << block.x << ":" << block.z << ":" << block.y;
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const LocalBlockPos& block) {
	stream << block.x << ":" << block.z << ":" << block.y;
	return stream;
}

}
}
