#include <floatfann.h>

#include <fstream>
#include <future>
#include <sstream>
#include <thread>

#include "Common.h"
#include "NNAgent.h"

using namespace std;

bool runGame(NNAgent x, NNAgent y) {
  Position pos;
  while (!pos.isEndGame()) {
    Move move;
    if (pos.turns & 1) {
      move = y.getBestMove(pos);
    } else {
      move = x.getBestMove(pos);
    }
    pos.doMove(move);
  }
  return (pos.turns & 1);
}

struct Coaching {
  Coaching() : curr(), best(curr) {}

  void init() {
    best = NNAgent("data/best.ann");
    curr = best;
    ifstream in("data/examples.txt");
    for (Example example; in >> example;) {
      examples.push_back(example);
    }
    cout << "Started from previous data" << endl;
    cout << "With " << examples.size() << " example." << endl;
  }

  void saveData() {
    best.save("data/best.ann");
    ofstream out("data/examples.txt");
    for (const auto &example : examples) {
      out << example << endl;
    }
    out.close();
  }

  // TODO: verify that the games are differents!!
  double pit() {
    constexpr int gamesCount = 20;
    int wins = 0;
    future<bool> results[gamesCount];
    for (int i = 0; i < gamesCount; ++i) {
      results[i] =
          (i & 1) ? async(runGame, best, curr) : async(runGame, curr, best);
    }

    for (int i = 0; i < gamesCount; ++i) {
      cerr << "game " << i;
      if (i & 1)
        cout << " as black=>";
      else
        cout << " as white=>";
      bool whiteWins = results[i].get();
      if (whiteWins) {
        if (i & 1)
          cerr << "candidate lost" << endl;
        else {
          ++wins;
          cerr << "candidate wins" << endl;
        }
      } else {
        if (!(i & 1))
          cerr << "candidate lost" << endl;
        else {
          ++wins;
          cerr << "candidate wins" << endl;
        }
      }
    }

    return static_cast<double>(wins) / static_cast<double>(gamesCount);
  }

  void train(int iteration) {
    constexpr int episodesCount = 10;
    future<list<Example>> results[episodesCount];
    for (int e = 0; e < episodesCount; ++e) {
      results[e] = async([this]() {
        list<Example> episodeExamples;
        auto master = best;
        master.selfPlay(episodeExamples);
        return episodeExamples;
      });
    }
    // TODO: Check that games in self play are differents!!
    for (int e = 0; e < episodesCount; ++e) {
      cout << "episode " << e << " ..";
      cout.flush();
      auto episodeExamples = results[e].get();
      for (const auto &example : episodeExamples) {
        examples.push_back(example);
      }
      cout << "done!" << endl;
    }

    cout << "training with " << examples.size() << " examples .." << endl;
    auto train_data = createTrainData();
    curr.train(train_data);
    auto winRate = pit();
    cerr << "winRate=" << 100 * winRate << "%" << endl;
    if (winRate >= 0.55) {
      cerr << "A better agent found" << endl;
      best = curr;
      ostringstream stream;
      stream << "data/" << iteration << ".ann";
      best.save(stream.str());
    }
  }

  fann_train_data *createTrainData() {
    while (examples.size() > 25000) {
      examples.pop_front();
    }

    vector<Example> sample;
    std::sample(examples.begin(), examples.end(), std::back_inserter(sample),
                2500, std::mt19937{std::random_device{}()});

    auto train_data = fann_create_train(8 * sample.size(), 60, 1);
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
    return train_data;
  }

  NNAgent curr;
  NNAgent best;
  list<Example> examples;
};

int main(int argc, char *argv[]) {
  Coaching coaching;
  if (argc > 1 && string(argv[1]) == "--continue") {
    coaching.init();
  }

  int maxIterations;
  cout << "Please give max iterations:" << endl;
  cin >> maxIterations;
  for (int i = 1; i <= maxIterations; ++i) {
    cout << "iteration=" << i << endl;
    coaching.train(i);
    if (i % 10 == 0) coaching.saveData();
  }

  cout << "A sample game of slef play with best one" << endl;
  cout << "moves=";
  NNAgent agent("data/best.ann");
  Position pos;
  while (!pos.isEndGame()) {
    auto move = agent.getBestMoveForSelfPlay(pos);
    pos.doMove(move);
    cout << "'" << showWall(move.wall) << "', ";
  }
  cout << endl;

  return 0;
}
