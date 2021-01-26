#include "Position.h"

#include "RNG.h"

Position::Position()
    : walls{0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
            30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
            45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59},
      placed(emptyBitmask),
      state(emptyBitmask),
      possibleWalls(allWallsBitmask),
      possibleSizes(allSizesBitmask),
      turns(0),
      wallsLength(60) {}

void Position::doMove(const Move &move) {
  add(placed, move.wall);
  add(state, move.wall);
  remove(possibleWalls, move.wall);

  if (move.zone) {
    possibleWalls &= ~move.zone.walls;
    state &= ~move.zone.walls;
    state |= move.zone.border;
    remove(possibleSizes, move.zone.size);
  }
  turns += 1;
  int len = 0;
  for (int i = 0; i < wallsLength; ++i) {
    int wall = walls[i];
    if (isPossibleWall(wall)) {
      walls[len++] = wall;
    }
  }
  wallsLength = len;
}

Move Position::getMove(int wall) const {
  Move move;
  move.wall = wall;
  move.zone = findZone(wall);
  return move;
}

void Position::doMove(int wall) {
  Move move = {wall, findZone(wall)};
  doMove(move);
}

void Position::doMove(const string &s) {
  int wall = parseWall(s);
  doMove(wall);
}

bool Position::isPossibleWall(int wall) const {
  return contains(possibleWalls, wall);
}

bool Position::isPossibleSize(int size) const {
  return contains(possibleSizes, size);
}

void Position::tryClose(int wall, int square, Zone &zone, bool &border) const {
  if (border) return;

  zone.size++;
  add(zone.squares, square);
  zone.walls |= wallsBitmaskOfSquare[square];
  zone.border ^= wallsBitmaskOfSquare[square];

  for (const auto &neighbor : neighborsOf[square]) {
    const auto &[w, nextSquare] = neighbor;

    if (contains(placed, w) || w == wall) {
      continue;
    }

    if (nextSquare == -1) {
      border = true;
      zone = {};
      return;
    }

    if (contains(zone.squares, nextSquare)) {
      continue;
    }

    tryClose(wall, nextSquare, zone, border);
    if (border) {
      return;
    }
  }
}

Zone Position::findZone(Wall wall) const {
  auto [x, y] = neighborsBitmasksOfWall[wall];
  if (!intersect(placed, x) || !intersect(placed, y)) return {};

  for (const auto &square : splitBy[wall]) {
    Zone zone{};
    bool border = false;
    tryClose(wall, square, zone, border);
    if (!border) return zone;
  }

  return {};
}

// split into get Random move for black and white
// pEdgeBlack = 1.0 - pEdgeWhite
// pEdgeWhite = turns / 41
// adjust possible moves to return edge or not edge first
bool Position::getRandomMove(RNG &gen, Move &move) const {
  int len = wallsLength;
  while (len) {
    int r = gen.lessThan(len);
    int wall = walls[r];
    auto zone = findZone(wall);
    if (isPossibleSize(zone.size)) {
      move = {wall, zone};
      return true;
    } else {
      --len;
      swap(walls[r], walls[len]);
    }
  }
  return false;
}

int Position::getImpact(const Move &move) const {
  return __builtin_popcountll(possibleWalls & move.zone.walls);
}

State Position::getStateAfterPlaying(const Move &move) const {
  auto result = state;
  add(result, move.wall);
  if (move.zone) {
    result &= ~move.zone.walls;
    result |= move.zone.border;
  }
  return result;
}
