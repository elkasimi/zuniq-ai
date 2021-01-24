#pragma once

#include "Common.h"
#include "Position.h"
#include "RNG.h"
#include "robin_hood.h"

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
struct ActionInfo {
  ActionInfo() : status(INVALID) {}

  Stats q1;
  Stats q2;
  Stats q3;
  unsigned int status : 2;
  unsigned int impact : 6;

  operator bool() const { return status != INVALID; }
  bool isWinning() const { return status == WIN; }
  bool isLosing() const { return status == LOSS; }
  void markWinning() { status = WIN; }
  void markLosing() { status = LOSS; }
};

constexpr float OO = 1000000.0f;
inline bool isExactWin(float v) { return v > 10000.0f; };
inline bool isExactLoss(float v) { return v < -10000.0f; };

struct StateInfo {
  StateInfo() : status(UNKNOWN), actionsCount(0), visits(0) {}

  // This can be optimised in term of memory usage by using pointers instead of objects as
  // at least half of them are not used. but as CodeCup competition offered 256MB this year
  // I did not force myself to optimise it
  ActionInfo actionInfo[60];
  Bitmask invalid;
  unsigned int status : 2;
  unsigned int winningAction : 6;
  unsigned int actionsCount : 6;
  unsigned int visits : 18;

  static RNG rng;

  bool isWinning() const { return status == WIN; }
  bool isLosing() const { return status == LOSS; }
  void markWinning(int a) {
    status = WIN;
    winningAction = a;
    actionInfo[a].markWinning();
  }
  void markLosing(int a) { actionInfo[a].markLosing(); }
  void markLosing() { status = LOSS; }

  float eval(int a) const {
    const auto &[q1, q2, q3, status, impact] = actionInfo[a];

    if (status == WIN) return OO;

    if (status == LOSS) return -OO;

    float bias = static_cast<float>(impact);

    if (q3.visits == 0) {
      return 1000.0f * bias + rng.fromRange(0, 60);
    }

    auto [v1, v2, v3] = make_tuple(q1.value, q2.value, q3.value);
    auto [n1, n2, n3] = make_tuple(q1.visits, q2.visits, q3.visits);
    v2 = max(v2, v1);
    v3 = max(v3, v1);
    int n = n1 + n2 + n3;
    float value = n1 * v1 + n2 * v2 + n3 * v3;
    value /= n;
    return value + bias * sqrtf(visits) / n;
  }

  int selectRandom() const {
    int actions[60];
    int len = 0;
    for (int a = 0; a < 60; ++a) {
      if (actionInfo[a]) actions[len++] = a;
    }
    return actions[rng.lessThan(len)];
  }

  int select() const {
    if (isWinning()) {
      return winningAction;
    }

    int best = -1;
    float bestValue = numeric_limits<float>::lowest();
    for (int a = 0; a < 60; ++a) {
      if (!actionInfo[a]) continue;
      auto value = eval(a);
      if (bestValue < value) {
        best = a;
        bestValue = value;
      }
    }
    return best;
  }

  int selectMostVisited() const {
    int mostVisited = -1;
    float maxVisits = numeric_limits<int>::lowest();
    for (int a = 0; a < 60; ++a) {
      if (!actionInfo[a]) continue;
      int visits = actionInfo[a].q1.visits;
      if (maxVisits < visits) {
        maxVisits = visits;
        mostVisited = a;
      }
    }
    return mostVisited;
  }

  void updateQ1(int a, float v, int c) {
    if (visits < 200000) ++visits;
    actionInfo[a].q1.update(v, c);
  }

  void updateQ2(int a, float v, int c) {
    auto &info = actionInfo[a];
    if (!info) return;
    info.q2.update(v, c);
  }

  void updateQ3(int a, float v, int c) {
    auto &info = actionInfo[a];
    if (!info) return;
    info.q3.update(v, c);
  }
};

constexpr int samples = 10;

struct AMAFStats {
  Stats white;
  Stats black;
  Stats any;

  operator bool() const { return white || black || any; }
};

struct IterationResult {
  pair<State, Action> transitions[60];
  float value;
  bool firstStateBlack;
  int countTransitions = 0;
  AMAFStats amafStats[60];

  void add(State s, Action a) {
    assert(countTransitions < 60);
    transitions[countTransitions++] = {s, a};
  }
};

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

struct McRaveAgent {
  McRaveAgent();

  void simulate(const Position &pos);
  static int getWinningAction(const Position &pos);
  void simulateDefault(const Position &pos, IterationResult &result);
  int simulateTree(Position &pos, IterationResult &result,
                   StateInfo *lastState = nullptr,
                   ActionInfo *lastAction = nullptr);
  float eval(const Position &pos, const Move &move);
  Move select(const Position &pos);
  Move selectMostVisited(const Position &pos);
  void backup(const IterationResult &result);
  StateInfo &newNode(const Position &pos);
  pair<bool, Move> getBestMove(const Position &pos,
                               bool useTimeConstraint = true);
  void log(const Position &pos, const Move &move);
  int getDepth(const Position &pos);
  void launchDebugSession(const Position &pos);

  inline void pickTransformation() {
    transformationIndex = gen.lessThan(8);
    cerr << "Using transformation=" << transformationIndex << endl;
  }

  inline State transformState(State state) {
    auto transformedState = emptyBitmask;
    for (int wall = 0; wall < 60; ++wall) {
      if (::contains(state, wall)) {
        add(transformedState, transformations[transformationIndex][wall]);
      }
    }
    return transformedState;
  }

  void clean(const Position &pos) {
    cerr << "ri=" << m.size() << " ";
    for (auto it = m.begin(); it != m.end();) {
      const auto &state = it->first;
      const auto &invalid = it->second.invalid;
      auto diff = pos.state & (~state);
      if ((diff & invalid) != diff) {
        it = m.erase(it);
      } else {
        ++it;
      }
    }
    cerr << "rf=" << m.size() << endl;
    m.reserve(120000);
  }

  bool contains(State s) { return m.find(s) != m.end(); }

  robin_hood::unordered_map<State, StateInfo> m;

  RNG gen;
  double totalTime;
  int transformationIndex = 0;
  bool canClaimWin = true;
  int me;
  static constexpr double r = 1.0;
  static constexpr double defaultMaxTime = r * 2.0;
  static constexpr double maxCheckTime = r * 0.25;
  static constexpr double maxTotalTime = r * 30.0;
  static constexpr double timeTroubleThreshold = r * 25.0;
  static constexpr int maxIterations = 200000;
};

extern const robin_hood::unordered_map<State, int> openingBook;
