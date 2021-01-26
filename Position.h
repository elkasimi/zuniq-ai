#pragma once

#include "Common.h"

struct RNG;
struct Position {
  Position();

  void doMove(int wall);
  void doMove(const Move& move);
  void doMove(const string& move);

  void tryClose(int wall, int square, Zone& zone, bool& border) const;
  Zone findZone(int wall) const;

  bool isPossibleWall(int wall) const;
  bool isPossibleSize(int size) const;

  bool getRandomMove(RNG& gen, Move& move) const;

  Move getMove(int Wall) const;

  struct MoveIterator {
    Move move;
    const Position* pos;
    int index;

    MoveIterator(const Position* p, int i = 0) : pos(p), index(i) { advance(); }

    void advance() {
      while (index < pos->wallsLength) {
        int wall = pos->walls[index];
        auto zone = pos->findZone(wall);
        if (pos->isPossibleSize(zone.size)) {
          move = {wall, zone};
          break;
        }
        ++index;
      }
    }

    void operator++() {
      ++index;
      advance();
    }

    const Move& operator*() { return move; }

    bool operator==(const MoveIterator& it) { return index == it.index; }

    bool operator!=(const MoveIterator& it) { return index != it.index; }
  };

  MoveIterator begin() const { return MoveIterator(this); }

  MoveIterator end() const { return MoveIterator(this, wallsLength); }

  bool isEndGame() const { return begin() == end(); }

  int getImpact(const Move& move) const;

  State getStateAfterPlaying(const Move& move) const;

  mutable int walls[60];
  Bitmask placed;
  Bitmask state;
  Bitmask possibleWalls;
  Bitmask possibleSizes;
  int turns;
  int wallsLength;
};
