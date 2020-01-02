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

#ifndef POS_H_
#define POS_H_

#include <iostream>

/**
 * Here are some Minecraft position things. In Minecraft, x/z are the horizontal axes
 * and y is the vertical axis.
 *
 * Here are the Minecraft directions (also available as constants):
 * north = -z
 * south = +z
 * east = +x
 * west = -x
 */

namespace mapcrafter {
namespace mc {

class RegionPos {
public:
	int x=0, z=0;

	RegionPos() = default;
	RegionPos(int x, int z);

	bool operator==(const RegionPos& other) const;
	bool operator!=(const RegionPos& other) const;
	bool operator<(const RegionPos& other) const;

	static RegionPos byFilename(const std::string& filename);

	void rotate(int count);
};

class BlockPos;

class ChunkPos {
public:
	int x=0, z=0;

	ChunkPos() = default;
	ChunkPos(int x, int z);
	ChunkPos(const BlockPos& block);

	int getLocalX() const;
	int getLocalZ() const;

	RegionPos getRegion() const;

	bool operator==(const ChunkPos& other) const;
	bool operator!=(const ChunkPos& other) const;
	bool operator<(const ChunkPos& other) const;

	int getRow() const;
	int getCol() const;
	static ChunkPos byRowCol(int row, int col);

	void rotate(int count);
};

class LocalBlockPos;

class BlockPos {
public:
	int x=0, z=0, y=0;

	BlockPos() = default;
	BlockPos(int x, int z, int y);

	int getRow() const;
	int getCol() const;

	bool operator==(const BlockPos& other) const;
	bool operator!=(const BlockPos& other) const;
	bool operator<(const BlockPos& other) const;

	BlockPos& operator+=(const BlockPos& p);
	BlockPos& operator-=(const BlockPos& p);
	BlockPos operator+(const BlockPos& p2) const;
	BlockPos operator-(const BlockPos& p2) const;
};

extern const mc::BlockPos DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST, DIR_TOP, DIR_BOTTOM;

class LocalBlockPos {
public:
	int x=0, z=0, y=0;

	LocalBlockPos() = default;
	LocalBlockPos(int x, int z, int y);
	LocalBlockPos(const BlockPos& pos);

	int getRow() const;
	int getCol() const;

	BlockPos toGlobalPos(const ChunkPos& chunk) const;

	bool operator<(const LocalBlockPos& other) const;
};

std::ostream& operator<<(std::ostream& stream, const RegionPos& region);
std::ostream& operator<<(std::ostream& stream, const ChunkPos& chunk);
std::ostream& operator<<(std::ostream& stream, const BlockPos& block);
std::ostream& operator<<(std::ostream& stream, const LocalBlockPos& block);

//////////////////////////////////////////
// Implementations.
//////////////////////////////////////////

inline RegionPos::RegionPos(int x, int z)
	: x(x), z(z) {}

inline bool RegionPos::operator==(const RegionPos& other) const {
	return x == other.x && z == other.z;
}

inline bool RegionPos::operator!=(const RegionPos& other) const {
	return !operator==(other);
}

inline bool RegionPos::operator<(const RegionPos& other) const {
	if (x == other.x)
		return z < other.z;
	return x < other.x;
}


inline ChunkPos::ChunkPos(int x, int z)
	: x(x), z(z) {}

inline bool ChunkPos::operator==(const ChunkPos& other) const {
	return x == other.x && z == other.z;
}

inline bool ChunkPos::operator!=(const ChunkPos& other) const {
	return !operator ==(other);
}

inline bool ChunkPos::operator<(const ChunkPos& other) const {
	if (x == other.x)
		return z < other.z;
	return x < other.x;
}

inline int BlockPos::getCol() const {
	return x + z;
}

inline BlockPos::BlockPos(int x, int z, int y)
	: x(x), z(z), y(y) {
}

inline BlockPos& BlockPos::operator+=(const BlockPos& p) {
	x += p.x;
	z += p.z;
	y += p.y;
	return *this;
}

inline BlockPos& BlockPos::operator-=(const BlockPos& p) {
	x -= p.x;
	z -= p.z;
	y -= p.y;
	return *this;
}

inline BlockPos BlockPos::operator+(const BlockPos& p2) const {
	BlockPos p = *this;
	return p += p2;
}

inline BlockPos BlockPos::operator-(const BlockPos& p2) const {
	BlockPos p = *this;
	return p -= p2;
}

inline bool BlockPos::operator==(const BlockPos& other) const {
	return x == other.x && z == other.z && y == other.y;
}

inline bool BlockPos::operator!=(const BlockPos& other) const {
	return !operator==(other);
}

inline bool BlockPos::operator<(const BlockPos& other) const {
	if (y == other.y) {
		if (x == other.x)
			return z < other.z;
		return x > other.x;
	}
	return y < other.y;
}

}
}

#endif
