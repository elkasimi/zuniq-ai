#pragma once

#include "Common.h"
#include "Position.h"
#include "RNG.h"
#include "robin_hood.h"

struct Example {
  State state;
  float value;
};

struct ActionInfo {
  ActionInfo() : status(INVALID) {}

  Stats q;
  unsigned int status : 2;

  operator bool() const { return status != INVALID; }
  bool isWinning() const { return status == WIN; }
  bool isLosing() const { return status == LOSS; }
  void markWinning() { status = WIN; }
  void markLosing() { status = LOSS; }
};

struct StateInfo {
  StateInfo() : status(UNKNOWN), actionsCount(0), visits(0) {}

  ActionInfo actionInfo[60];
  Bitmask invalid;
  unsigned int status : 2;
  unsigned int winningAction : 6;
  unsigned int actionsCount : 6;
  unsigned int visits : 18;

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
    // TODO
    return 0.0f;
  }

  int selectRandom() const {
    // TODO
    return 0;
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
      int visits = actionInfo[a].q.visits;
      if (maxVisits < visits) {
        maxVisits = visits;
        mostVisited = a;
      }
    }
    return mostVisited;
  }

  void update(int a, float value) {
    ++visits;
    actionInfo[a].q.update(value);
  }
};

struct FANN;
struct NNAgent {
  NNAgent();
  NNAgent(const string &filename);

  void simulate(const Position &pos);
  float simulateDefault(const Position &pos);
  void simulateTree(Position &pos);
  float eval(const Position &pos, const Move &move);
  Move select(const Position &pos);
  Move selectMostVisited(const Position &pos);
  void backup(float value);
  StateInfo &newNode(const Position &pos);
  Move getBestMove(const Position &pos);

  void train(const string &filename);
  void selfPlay(list<Example> &examples);
  void save(const string &filename);

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
    m.reserve(maxIterations);
  }

  bool contains(State s) { return m.find(s) != m.end(); }

  FANN *ann;
  robin_hood::unordered_map<State, StateInfo> m;

  static constexpr int maxIterations = 1000;
  static RNG gen;
};
