#pragma once

#include "Common.h"
#include "Position.h"
#include "RNG.h"
#include "robin_hood.h"

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

struct StateInfo {
  StateInfo() : status(UNKNOWN), actionsCount(0), visits(0) {}

  // This can be optimised in term of memory usage by using pointers instead of
  // objects as at least half of them are not used. but as CodeCup competition
  // offered 256MB this year I did not force myself to optimise it
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
