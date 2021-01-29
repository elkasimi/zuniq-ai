#include <floatfann.h>

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
  Coaching(const string &filename) : curr(filename), best(curr) {}

  // TODO: verify that the games are differents!!
  double pit() {
    constexpr int gamesPerColor = 10;
    int a = 0, b = 0;
    future<bool> results[gamesPerColor];
    for (int i = 0; i < gamesPerColor; ++i) {
      results[i] = async(runGame, best, curr);
    }

    for (int i = 0; i < gamesPerColor; ++i) {
      cerr << "game " << i << " as black";
      bool whiteWins = results[i].get();
      if (whiteWins) {
        cerr << "=> best so far wins" << endl;
        ++b;
      } else {
        cerr << "=> candidate wins" << endl;
        ++a;
      }
    }

    for (int i = 0; i < gamesPerColor; ++i) {
      results[i] = async(runGame, curr, best);
    }

    for (int i = 0; i < gamesPerColor; ++i) {
      cerr << "game " << i << " as white";
      bool whiteWins = results[i].get();
      if (whiteWins) {
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
    createTrainData();
    curr.train("data.train");
    auto winRate = pit();
    cerr << "winRate=" << 100 * winRate << "%" << endl;
    if (winRate >= 0.6) {
      cerr << "A better agent found" << endl;
      best = curr;
      ostringstream stream;
      stream << "bests/" << iteration << ".ann";
      best.save(stream.str());
    }
  }

  void createTrainData() {
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
    fann_save_train(train_data, "data.train");
    fann_destroy_train(train_data);
  }

  NNAgent curr;
  NNAgent best;
  list<Example> examples;
};

int main(int argc, char *argv[]) {
  auto coaching = argc > 1 ? Coaching(argv[1]) : Coaching();

  for (int i = 0; i < 1000; ++i) {
    cout << "iteration=" << i << endl;
    coaching.train(i);
  }

  return 0;
}
