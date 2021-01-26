#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <optional>
#include <random>
#include <stack>
#include <string>
#include <vector>

using namespace std;

using Square = int;

using Wall = int;

using Bitmask = uint64_t;

constexpr Bitmask emptyBitmask = 0ull;

struct Zone {
  Zone()
      : squares(emptyBitmask),
        walls(emptyBitmask),
        border(emptyBitmask),
        size(0) {}

  Zone(int _size, Bitmask _squares, Bitmask _walls, Bitmask _border)
      : squares(_squares), walls(_walls), border(_border), size(_size) {}

  Bitmask squares;
  Bitmask walls;
  Bitmask border;
  int size;

  operator bool() const { return size > 0; }
};

struct Move {
  Wall wall;
  Zone zone;
};

inline bool operator==(const Move &lhs, const Move &rhs) {
  return lhs.wall == rhs.wall;
}

inline bool operator!=(const Move &lhs, const Move &rhs) {
  return lhs.wall != rhs.wall;
}

constexpr Bitmask getFlag(int pos) { return 1ull << pos; }

template <typename... Args>
constexpr auto mkBitmask(Args... args) {
  return (getFlag(args) | ... | emptyBitmask);
}

inline bool contains(Bitmask b, int i) { return (b & getFlag(i)); }

inline bool intersect(Bitmask b1, Bitmask b2) { return (b1 & b2); }

inline void add(Bitmask &b, int i) { b |= getFlag(i); }

inline void remove(Bitmask &b, int i) { b &= ~getFlag(i); }

inline Wall parseWall(const string &s) {
  char r = s[0];
  char c = s[1];
  return s[2] == 'h' ? 5 * (r - 'A') + c - '1' : 6 * (r - 'A') + c - '1' + 30;
}

inline string showWall(Wall wall) {
  if (wall < 30) {
    char r = static_cast<char>('A' + wall / 5);
    char c = static_cast<char>('1' + wall % 5);
    return {r, c, 'h'};
  } else {
    wall -= 30;
    char r = static_cast<char>('A' + wall / 6);
    char c = static_cast<char>('1' + wall % 6);
    return {r, c, 'v'};
  }
}

inline ostream &operator<<(ostream &out, const Move &move) {
  out << showWall(move.wall);
  return out;
}

constexpr Bitmask allWallsBitmask =
    mkBitmask(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
              19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
              35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
              51, 52, 53, 54, 55, 56, 57, 58, 59);

constexpr Bitmask allSizesBitmask =
    mkBitmask(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
              19, 20, 21, 22, 23, 24, 25);

const vector<vector<Square>> splitBy{
    {0},      {1},      {2},      {3},      {4},      {5, 0},   {6, 1},
    {7, 2},   {8, 3},   {9, 4},   {10, 5},  {11, 6},  {12, 7},  {13, 8},
    {14, 9},  {15, 10}, {16, 11}, {17, 12}, {18, 13}, {19, 14}, {20, 15},
    {21, 16}, {22, 17}, {23, 18}, {24, 19}, {20},     {21},     {22},
    {23},     {24},     {0},      {1, 0},   {2, 1},   {3, 2},   {4, 3},
    {4},      {5},      {6, 5},   {7, 6},   {8, 7},   {9, 8},   {9},
    {10},     {11, 10}, {12, 11}, {13, 12}, {14, 13}, {14},     {15},
    {16, 15}, {17, 16}, {18, 17}, {19, 18}, {19},     {20},     {21, 20},
    {22, 21}, {23, 22}, {24, 23}, {24},
};

const vector<vector<Wall>> wallsOfSquare{
    {0, 31, 5, 30},   {1, 32, 6, 31},   {2, 33, 7, 32},   {3, 34, 8, 33},
    {4, 35, 9, 34},   {5, 37, 10, 36},  {6, 38, 11, 37},  {7, 39, 12, 38},
    {8, 40, 13, 39},  {9, 41, 14, 40},  {10, 43, 15, 42}, {11, 44, 16, 43},
    {12, 45, 17, 44}, {13, 46, 18, 45}, {14, 47, 19, 46}, {15, 49, 20, 48},
    {16, 50, 21, 49}, {17, 51, 22, 50}, {18, 52, 23, 51}, {19, 53, 24, 52},
    {20, 55, 25, 54}, {21, 56, 26, 55}, {22, 57, 27, 56}, {23, 58, 28, 57},
    {24, 59, 29, 58}};

const vector<Bitmask> wallsBitmaskOfSquare{
    mkBitmask(0, 31, 5, 30),   mkBitmask(1, 32, 6, 31),
    mkBitmask(2, 33, 7, 32),   mkBitmask(3, 34, 8, 33),
    mkBitmask(4, 35, 9, 34),   mkBitmask(5, 37, 10, 36),
    mkBitmask(6, 38, 11, 37),  mkBitmask(7, 39, 12, 38),
    mkBitmask(8, 40, 13, 39),  mkBitmask(9, 41, 14, 40),
    mkBitmask(10, 43, 15, 42), mkBitmask(11, 44, 16, 43),
    mkBitmask(12, 45, 17, 44), mkBitmask(13, 46, 18, 45),
    mkBitmask(14, 47, 19, 46), mkBitmask(15, 49, 20, 48),
    mkBitmask(16, 50, 21, 49), mkBitmask(17, 51, 22, 50),
    mkBitmask(18, 52, 23, 51), mkBitmask(19, 53, 24, 52),
    mkBitmask(20, 55, 25, 54), mkBitmask(21, 56, 26, 55),
    mkBitmask(22, 57, 27, 56), mkBitmask(23, 58, 28, 57),
    mkBitmask(24, 59, 29, 58)};

const vector<vector<pair<Wall, Square>>> neighborsOf{
    {{31, 1}, {30, -1}, {5, 5}, {0, -1}},
    {{32, 2}, {31, 0}, {6, 6}, {1, -1}},
    {{33, 3}, {32, 1}, {7, 7}, {2, -1}},
    {{34, 4}, {33, 2}, {8, 8}, {3, -1}},
    {{35, -1}, {34, 3}, {9, 9}, {4, -1}},
    {{37, 6}, {36, -1}, {10, 10}, {5, 0}},
    {{38, 7}, {37, 5}, {11, 11}, {6, 1}},
    {{39, 8}, {38, 6}, {12, 12}, {7, 2}},
    {{40, 9}, {39, 7}, {13, 13}, {8, 3}},
    {{41, -1}, {40, 8}, {14, 14}, {9, 4}},
    {{43, 11}, {42, -1}, {15, 15}, {10, 5}},
    {{44, 12}, {43, 10}, {16, 16}, {11, 6}},
    {{45, 13}, {44, 11}, {17, 17}, {12, 7}},
    {{46, 14}, {45, 12}, {18, 18}, {13, 8}},
    {{47, -1}, {46, 13}, {19, 19}, {14, 9}},
    {{49, 16}, {48, -1}, {20, 20}, {15, 10}},
    {{50, 17}, {49, 15}, {21, 21}, {16, 11}},
    {{51, 18}, {50, 16}, {22, 22}, {17, 12}},
    {{52, 19}, {51, 17}, {23, 23}, {18, 13}},
    {{53, -1}, {52, 18}, {24, 24}, {19, 14}},
    {{55, 21}, {54, -1}, {25, -1}, {20, 15}},
    {{56, 22}, {55, 20}, {26, -1}, {21, 16}},
    {{57, 23}, {56, 21}, {27, -1}, {22, 17}},
    {{58, 24}, {57, 22}, {28, -1}, {23, 18}},
    {{59, -1}, {58, 23}, {29, -1}, {24, 19}}};

const vector<pair<Bitmask, Bitmask>> neighborsBitmasksOfWall{
    {0x0000000080000002u, 0x0000000040000000u},
    {0x0000000100000004u, 0x0000000080000001u},
    {0x0000000200000008u, 0x0000000100000002u},
    {0x0000000400000010u, 0x0000000200000004u},
    {0x0000000800000000u, 0x0000000400000008u},
    {0x0000002080000040u, 0x0000001040000000u},
    {0x0000004100000080u, 0x0000002080000020u},
    {0x0000008200000100u, 0x0000004100000040u},
    {0x0000010400000200u, 0x0000008200000080u},
    {0x0000020800000000u, 0x0000010400000100u},
    {0x0000082000000800u, 0x0000041000000000u},
    {0x0000104000001000u, 0x0000082000000400u},
    {0x0000208000002000u, 0x0000104000000800u},
    {0x0000410000004000u, 0x0000208000001000u},
    {0x0000820000000000u, 0x0000410000002000u},
    {0x0002080000010000u, 0x0001040000000000u},
    {0x0004100000020000u, 0x0002080000008000u},
    {0x0008200000040000u, 0x0004100000010000u},
    {0x0010400000080000u, 0x0008200000020000u},
    {0x0020800000000000u, 0x0010400000040000u},
    {0x0082000000200000u, 0x0041000000000000u},
    {0x0104000000400000u, 0x0082000000100000u},
    {0x0208000000800000u, 0x0104000000200000u},
    {0x0410000001000000u, 0x0208000000400000u},
    {0x0820000000000000u, 0x0410000000800000u},
    {0x0080000004000000u, 0x0040000000000000u},
    {0x0100000008000000u, 0x0080000002000000u},
    {0x0200000010000000u, 0x0100000004000000u},
    {0x0400000020000000u, 0x0200000008000000u},
    {0x0800000000000000u, 0x0400000010000000u},
    {0x0000000000000001u, 0x0000001000000020u},
    {0x0000000000000003u, 0x0000002000000060u},
    {0x0000000000000006u, 0x00000040000000c0u},
    {0x000000000000000cu, 0x0000008000000180u},
    {0x0000000000000018u, 0x0000010000000300u},
    {0x0000000000000010u, 0x0000020000000200u},
    {0x0000000040000020u, 0x0000040000000400u},
    {0x0000000080000060u, 0x0000080000000c00u},
    {0x00000001000000c0u, 0x0000100000001800u},
    {0x0000000200000180u, 0x0000200000003000u},
    {0x0000000400000300u, 0x0000400000006000u},
    {0x0000000800000200u, 0x0000800000004000u},
    {0x0000001000000400u, 0x0001000000008000u},
    {0x0000002000000c00u, 0x0002000000018000u},
    {0x0000004000001800u, 0x0004000000030000u},
    {0x0000008000003000u, 0x0008000000060000u},
    {0x0000010000006000u, 0x00100000000c0000u},
    {0x0000020000004000u, 0x0020000000080000u},
    {0x0000040000008000u, 0x0040000000100000u},
    {0x0000080000018000u, 0x0080000000300000u},
    {0x0000100000030000u, 0x0100000000600000u},
    {0x0000200000060000u, 0x0200000000c00000u},
    {0x00004000000c0000u, 0x0400000001800000u},
    {0x0000800000080000u, 0x0800000001000000u},
    {0x0001000000100000u, 0x0000000002000000u},
    {0x0002000000300000u, 0x0000000006000000u},
    {0x0004000000600000u, 0x000000000c000000u},
    {0x0008000000c00000u, 0x0000000018000000u},
    {0x0010000001800000u, 0x0000000030000000u},
    {0x0020000001000000u, 0x0000000020000000u}};

using TimePoint = std::chrono::system_clock::time_point;

inline TimePoint getTimePoint() { return std::chrono::system_clock::now(); }

inline double getDeltaTimeSince(const TimePoint &start) {
  auto curr = getTimePoint();
  auto delta =
      std::chrono::duration_cast<std::chrono::microseconds>(curr - start)
          .count();
  return static_cast<double>(delta) / 1e6;
}

// 28 white moves =
// ['B1h','B2h','B3h','B4h','B5h','C1h','C5h','D1h','D5h','E1h','E2h','E3h','E4h','E5h','A2v','A3v','A4v','A5v','B2v','B5v','C2v','C5v','D2v','D5v','E2v','E3v','E4v','E5v']
// 36 black moves =
// ['A2h','A3h','A4h','B1h','B3h','B5h','C2h','C3h','C4h','D2h','D3h','D4h','E1h','E3h','E5h','F2h','F3h','F4h','A2v','A5v','B1v','B3v','B4v','B6v','C1v','C2v','C3v','C4v','C5v','C6v','D1v','D3v','D4v','D6v','E2v','E5v']
constexpr bool goodOpeningMove[2][60] = {
    {
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1,
        1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0,
        1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0,
    },
    {
        0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0,
    },
};

struct Stats {
  float value = 0.0f;
  int visits = 0;

  inline void update(float v, int c = 1) {
    value = value * visits + v * c;
    value /= visits + c;
    visits += c;
  }

  operator bool() const { return visits > 0; }

  inline void operator+=(const Stats &s) {
    if (s.visits == 0) return;
    update(s.value, s.visits);
  }

  inline void operator-=(const Stats &s) {
    if (s.visits == 0) return;
    update(-s.value, s.visits);
  }
};

inline ostream &operator<<(ostream &out, const Stats &stats) {
  out << "(" << stats.value << ", ";
  if (stats.visits < 1000)
    out << stats.visits;
  else
    out << (stats.visits / 1000.0f) << "k";
  out << ")";
  return out;
}

constexpr unsigned int INVALID = 0;
constexpr unsigned int UNKNOWN = 1;
constexpr unsigned int WIN = 2;
constexpr unsigned int LOSS = 3;

using State = Bitmask;
using Action = int;

constexpr float OO = 1000000.0f;
inline bool isExactWin(float v) { return v > 10000.0f; };
inline bool isExactLoss(float v) { return v < -10000.0f; };

constexpr int transformations[8][60] = {
    {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
     30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
     45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59},
    {54, 48, 42, 36, 30, 55, 49, 43, 37, 31, 56, 50, 44, 38, 32,
     57, 51, 45, 39, 33, 58, 52, 46, 40, 34, 59, 53, 47, 41, 35,
     25, 20, 15, 10, 5,  0,  26, 21, 16, 11, 6,  1,  27, 22, 17,
     12, 7,  2,  28, 23, 18, 13, 8,  3,  29, 24, 19, 14, 9,  4},
    {29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15,
     14, 13, 12, 11, 10, 9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
     59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45,
     44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30},
    {35, 41, 47, 53, 59, 34, 40, 46, 52, 58, 33, 39, 45, 51, 57,
     32, 38, 44, 50, 56, 31, 37, 43, 49, 55, 30, 36, 42, 48, 54,
     4,  9,  14, 19, 24, 29, 3,  8,  13, 18, 23, 28, 2,  7,  12,
     17, 22, 27, 1,  6,  11, 16, 21, 26, 0,  5,  10, 15, 20, 25},
    {4,  3,  2,  1,  0,  9,  8,  7,  6,  5,  14, 13, 12, 11, 10,
     19, 18, 17, 16, 15, 24, 23, 22, 21, 20, 29, 28, 27, 26, 25,
     35, 34, 33, 32, 31, 30, 41, 40, 39, 38, 37, 36, 47, 46, 45,
     44, 43, 42, 53, 52, 51, 50, 49, 48, 59, 58, 57, 56, 55, 54},
    {25, 26, 27, 28, 29, 20, 21, 22, 23, 24, 15, 16, 17, 18, 19,
     10, 11, 12, 13, 14, 5,  6,  7,  8,  9,  0,  1,  2,  3,  4,
     54, 55, 56, 57, 58, 59, 48, 49, 50, 51, 52, 53, 42, 43, 44,
     45, 46, 47, 36, 37, 38, 39, 40, 41, 30, 31, 32, 33, 34, 35},
    {59, 53, 47, 41, 35, 58, 52, 46, 40, 34, 57, 51, 45, 39, 33,
     56, 50, 44, 38, 32, 55, 49, 43, 37, 31, 54, 48, 42, 36, 30,
     29, 24, 19, 14, 9,  4,  28, 23, 18, 13, 8,  3,  27, 22, 17,
     12, 7,  2,  26, 21, 16, 11, 6,  1,  25, 20, 15, 10, 5,  0},
    {30, 36, 42, 48, 54, 31, 37, 43, 49, 55, 32, 38, 44, 50, 56,
     33, 39, 45, 51, 57, 34, 40, 46, 52, 58, 35, 41, 47, 53, 59,
     0,  5,  10, 15, 20, 25, 1,  6,  11, 16, 21, 26, 2,  7,  12,
     17, 22, 27, 3,  8,  13, 18, 23, 28, 4,  9,  14, 19, 24, 29}};

constexpr int inverse[8] = {0, 3, 2, 1, 4, 5, 6, 7};

inline vector<State> getAllTransformations(State state) {
  vector<State> result(8);
  for (auto i = 0; i < 8; ++i) {
    State s = emptyBitmask;
    for (int w = 0; w < 60; ++w) {
      if (contains(state, w)) {
        add(s, transformations[i][w]);
      }
    }
    result[i] = s;
  }

  return result;
}
