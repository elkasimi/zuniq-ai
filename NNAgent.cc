#include "NNAgent.h"

#include <floatfann.h>

RNG NNAgent::gen;

NNAgent::NNAgent(const NNAgent &other) { ann = fann_copy(other.ann); }

NNAgent &NNAgent::operator=(const NNAgent &other) {
  fann_destroy(ann);
  ann = fann_copy(other.ann);
  return *this;
}

NNAgent::NNAgent() {
  ann = fann_create_standard(4, 60, 40, 20, 1);
  fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
  fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);
}

NNAgent::NNAgent(const string &filename) {
  ann = fann_create_from_file(filename.c_str());
}

NNAgent::~NNAgent() { fann_destroy(ann); }

void NNAgent::save(const string &filename) { fann_save(ann, filename.c_str()); }

void NNAgent::train(fann_train_data *train_data) {
  fann_train_on_data(ann, train_data, 100, 10, 0.000001f);
  fann_destroy_train(train_data);
}

void NNAgent::selfPlay(list<Example> &examples) {
  vector<pair<int, State>> states;
  Position pos;
  while (!pos.isEndGame()) {
    auto move = getBestMoveForSelfPlay(pos);
    pos.doMove(move);
    states.emplace_back(pos.turns & 1, pos.state);
  }

  int winner = 1 - (pos.turns & 1);
  for (const auto &[player, state] : states) {
    float result = player == winner ? 1.0f : -1.0f;

    Example e{state, result};
    examples.push_back(e);
  }
}

Move NNAgent::getBestMove(const Position &pos) {
  m.clear();
  turn0 = pos.turns;

  constexpr int maxIterations = 400;
  for (int i = 0; i < maxIterations; ++i) {
    simulate(pos);
  }

  auto bestMove = selectMostVisited(pos);
  return bestMove;
}

void NNAgent::simulate(const Position &pos) {
  vector<Transition> transitions;
  auto t = pos;
  simulateTree(t, transitions);
  float result = simulateDefault(t);
  backup(transitions, result);
}

void NNAgent::simulateTree(Position &pos, vector<Transition> &transitions) {
  if (pos.isEndGame()) return;

  auto state = pos.state;
  if (!contains(state)) {
    newNode(pos);
    Move move;
    pos.getRandomMove(gen, move);
    pos.doMove(move);
    transitions.emplace_back(state, move.wall, pos.turns & 1);
    return;
  }

  auto move = select(pos);
  transitions.emplace_back(state, move.wall, pos.turns & 1);
  pos.doMove(move);
}

float NNAgent::estimate(State state) {
  auto value = 0.0f;
  for (auto s : getAllTransformations(state)) {
    vector<float> input(60);
    for (int i = 0; i < 60; ++i) {
      if (::contains(s, i)) {
        input[i] = 1.0f;
      } else {
        input[i] = 0.0f;
      }
    }
    value += fann_run(ann, input.data())[0];
  }
  return 0.125f * value;
}

float NNAgent::simulateDefault(const Position &pos) {
  if (pos.isEndGame()) {
    return pos.turns & 1 ? 1.0f : -1.0f;
  }

  return pos.turns & 1 ? -estimate(pos.state) : estimate(pos.state);
}

Move NNAgent::select(const Position &pos) {
  int best = m[pos.state].select();
  return pos.getMove(best);
}

Move NNAgent::selectMostVisited(const Position &pos) {
  int mostVisited = m[pos.state].selectMostVisited();
  return pos.getMove(mostVisited);
}

void NNAgent::backup(const vector<Transition> &transitions, float result) {
  for (auto &[s, a, player] : transitions) {
    auto &info = m[s];
    if (player == 0)
      info.update(a, result);
    else
      info.update(a, -result);
  }
}

StateInfo &NNAgent::newNode(const Position &pos) {
  auto &info = m[pos.state];
  info.invalid = (~pos.placed) & (~pos.possibleWalls);
  for (const Move &move : pos) {
    info.actionsCount++;
    auto nextState = pos.getStateAfterPlaying(move);
    float p = -estimate(nextState) +
              (pos.turns == turn0 ? 0.01f * gen.lessThan(11) : 0.0f);
    info.actionInfo[move.wall].p = p;
    info.actionInfo[move.wall].valid = true;
  }
  return info;
}

Move NNAgent::getBestMoveForSelfPlay(const Position &pos) {
  m.clear();
  turn0 = pos.turns;

  constexpr int maxIterations = 200;
  for (int i = 0; i < maxIterations; ++i) {
    simulate(pos);
  }

  if (pos.turns >= 30) {
    auto bestMove = selectMostVisited(pos);
    return bestMove;
  }

  auto s = pos.state;
  vector<double> weights;
  auto actionInfo = m[s].actionInfo;
  Move moves[60];
  int c = 0;
  for (const auto &move : pos) {
    weights.push_back(actionInfo[move.wall].q.visits);
    moves[c++] = move;
  }

  auto r = gen.withDiscreteDistribution(weights);
  return moves[r];
}
