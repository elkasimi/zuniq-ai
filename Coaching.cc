#include <floatfann.h>

#include "Common.h"
#include "NNAgent.h"

vector<State> getAllTransformations(State state) {
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

  vector<Bitmask> result(8);
  for (auto i = 0; i < 8; ++i) {
    Bitmask b = emptyBitmask;
    for (int w = 0; w < 60; ++w) {
      if (contains(state, w)) {
        add(b, transformations[i][w]);
      }
    }
    result[i] = b;
  }

  int c = 0;
  for (int i = 0; i < 8; ++i) {
    bool duplicate = false;
    for (int j = 0; j < c; ++j) {
      if (result[i] == result[j]) {
        duplicate = true;
        break;
      }
    }
    if (!duplicate) {
      result[c++] = result[i];
    }
  }

  result.resize(c);

  return result;
}

struct Coaching {
  Coaching() : curr(), best(curr) {}
  Coaching(const string &filename) : curr(filename), best(curr) {}

  double pit() {
    int a = 0, b = 0;
    for (int i = 1; i <= 5; ++i) {
      cerr << "game " << i << " as black";
      Position pos;
      while (!pos.isEndGame()) {
        Move move;
        if (pos.turns & 1) {
          move = curr.getBestMove(pos);
        } else {
          move = best.getBestMove(pos);
        }
        pos.doMove(move);
      }
      if (pos.turns & 1) {
        cerr << "=> best so far wins" << endl;
        ++b;
      } else {
        cerr << "=> candidate wins" << endl;
        ++a;
      }
    }

    for (int i = 1; i <= 5; ++i) {
      cerr << "game " << i << " as white";
      Position pos;
      while (!pos.isEndGame()) {
        Move move;
        if (pos.turns & 1) {
          move = best.getBestMove(pos);
        } else {
          move = curr.getBestMove(pos);
        }
        pos.doMove(move);
      }
      if (pos.turns & 1) {
        cerr << "=> candidate wins" << endl;
        ++a;
      } else {
        cerr << "=> best so far wins" << endl;
        ++b;
      }
    }

    return static_cast<double>(a) / static_cast<double>(a + b);
  }

  void train(int iteration) {
    for (int e = 1; e <= 10; ++e) {
      cout << "episode " << e << " ..";
      cout.flush();
      best.selfPlay(examples);
      cout << "done!" << endl;
    }
    cout << "training with " << examples.size() << " examples .." << endl;
    createTrainData();
    curr.train("data.train");
    auto winRate = pit();
    cerr << "winRate=" << 100 * winRate << "%" << endl;
    if (winRate >= 0.6) {
      cerr << "A better agent found" << endl;
      best = curr;
      best.save("best.ann");
    }
  }

  void createTrainData() {
    while (examples.size() > 25000) {
      examples.pop_front();
    }

    vector<Example> sample;
    std::sample(examples.begin(), examples.end(), std::back_inserter(sample),
                2500, std::mt19937{std::random_device{}()});

    auto train_data = fann_create_train(4 * sample.size(), 60, 1);
    auto i = 0u;
    for (const auto &[state, result] : sample) {
      for (const auto s : getAllTransformations(state)) {
        for (auto w = 0u; w < 60; ++w) {
          if (contains(s, w)) {
            train_data->input[i][w] = 1.0f;
          } else {
            train_data->input[i][w] = 0.0f;
          }
        }
        train_data->output[i][0] = result;
        i += 1;
      }
    }

    fann_shuffle_train_data(train_data);
    fann_save_train(train_data, "data.train");
    fann_destroy_train(train_data);
  }

  NNAgent curr;
  NNAgent best;
  list<Example> examples;
};

int main(int argc, char *argv[]) {
  auto coaching = argc > 1 ? Coaching("best.ann") : Coaching();

  for (int i = 0; i < 1000; ++i) {
    cout << "iteration=" << i << endl;
    coaching.train(i);
  }

  return 0;
}
