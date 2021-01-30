#pragma once

#include "Common.h"
#include "Position.h"
#include "RNG.h"

struct Example {
  State state;
  float value;
};

inline istream &operator>>(istream &in, Example &example) {
  in >> example.state >> example.value;
  return in;
}

inline ostream &operator<<(ostream &out, const Example &example) {
  out << example.state << " " << example.value;
  return out;
}

struct ActionInfo {
  ActionInfo() {}

  Stats q;
  float p;
  bool valid = false;

  operator bool() const { return valid; }
};

struct StateInfo {
  StateInfo() : actionsCount(0), visits(0) {}

  ActionInfo actionInfo[60];
  Bitmask invalid;
  int actionsCount;
  int visits;

  float eval(int a) const {
    const auto &[q, p, valid] = actionInfo[a];
    assert(valid);
    return q.value + p * sqrtf(visits) / (1.0f + q.visits);
  }

  int select() const {
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

using Transition = tuple<State, int, int>;

struct fann;
struct fann_train_data;
struct NNAgent {
  NNAgent();
  NNAgent(const NNAgent &other);
  NNAgent(const string &filename);
  NNAgent &operator=(const NNAgent &other);

  ~NNAgent();

  void simulate(const Position &pos);
  float simulateDefault(const Position &pos);
  void simulateTree(Position &pos, vector<Transition> &transitions);
  float eval(const Position &pos, const Move &move);
  Move select(const Position &pos);
  Move selectMostVisited(const Position &pos);
  void backup(const vector<Transition> &transitions, float value);
  StateInfo &newNode(const Position &pos);
  Move getBestMove(const Position &pos);
  Move getBestMoveForSelfPlay(const Position &pos);

  void train(fann_train_data *train_data);
  void selfPlay(list<Example> &examples);
  void save(const string &filename);
  float estimate(State state);
  bool contains(State s) { return m.find(s) != m.end(); }

  fann *ann;
  unordered_map<State, StateInfo> m;
  static RNG gen;
  int turn0;
};
