
#include "NNAgent.h"
#include "Position.h"

const string bestPath = "./best.ann";

int main() {
  Position pos;
  NNAgent agent(bestPath);
  for (string s; cin >> s && s != "Quit";) {
    if (s != "Start") {
      pos.doMove(s);
    }

    auto bestMove = agent.getBestMove(pos);
    pos.doMove(bestMove);
    cout << bestMove << endl;
  }

  return 0;
}
