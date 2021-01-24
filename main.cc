#include "Common.h"
#include "McRaveAgent.h"
#include "Position.h"

struct OpeningEntry {
  Bitmask placed;
  int wall;
  float value;
};

ostream &operator<<(ostream &out, const OpeningEntry &e) {
  out << "{0x";
  out.precision(6);
  out.width(16);
  out.fill('0');
  out << hex << e.placed << ", " << dec << e.wall << "},//" << e.value;
  return out;
}

void generateOpening(const Position &pos, McRaveAgent &agent, int curr,
                     int length) {
  if ((length - curr) % 2 == 0) {
    auto bestMove = agent.getBestMove(pos, false).second;
    if (length == curr) {
      OpeningEntry e{pos.placed, bestMove.wall, agent.eval(pos, bestMove)};
      cout << e << endl;
    } else {
      auto tmpPos = pos;
      tmpPos.doMove(bestMove);
      generateOpening(tmpPos, agent, curr + 1, length);
    }
  } else {
    for (const Move &move : pos) {
      auto tmpPos = pos;
      tmpPos.doMove(move);
      generateOpening(tmpPos, agent, curr + 1, length);
    }
  }
}

void generateOpening(int turn) {
  Position pos;
  McRaveAgent agent;
  generateOpening(pos, agent, 0, turn);
}

int main(int argc, char *argv[]) {
  if (argc >= 2) {
    if (string(argv[1]) == "--opening-0") {
      generateOpening(0);
      return 0;
    }

    if (string(argv[1]) == "--opening-1") {
      generateOpening(1);
      return 0;
    }

    if (string(argv[1]) == "--opening-2") {
      generateOpening(2);
      return 0;
    }

    if (string(argv[1]) == "--opening-3") {
      generateOpening(3);
      return 0;
    }

    if (string(argv[1]) == "--opening-4") {
      generateOpening(4);
      return 0;
    }

    if (string(argv[1]) == "--opening-5") {
      generateOpening(5);
      return 0;
    }

    if (argv[1] == string("--debug")) {
      Position pos;
      for (int i = 2; i < argc; ++i) {
        pos.doMove(argv[i]);
      }
      McRaveAgent agent;
      agent.launchDebugSession(pos);
      return 0;
    }

    if (argv[1] == string("--check-randomness")) {
      RNG gen;
      Position pos;
      int turn;
      cout << "Give start turn" << endl;
      cin >> turn;
      for (int i = 0; i < turn; ++i) {
        Move move;
        pos.getRandomMove(gen, move);
        pos.doMove(move);
      }
      int c[60];
      for (int i = 0; i < 60; ++i) c[i] = 0;
      int n = 1000000;
      for (int i = 0; i < n; ++i) {
        Move move;
        pos.getRandomMove(gen, move);
        c[move.wall]++;
      }
      int count = 0;
      cout.precision(2);
      for (int i = 0; i < 60; ++i) {
        if (!c[i]) continue;

        cout << i << " => " << 100.f * c[i] / n << "%" << endl;
        ++count;
      }
      float p = 100.f / count;
      cout << "\nexpecting probability=" << p << "% for each move" << endl;

      return 0;
    }

    if (argv[1] == string("--benchmark-playout")) {
      auto start = getTimePoint();
      constexpr int count = 100000;
      RNG gen;
      int wins = 0;
      for (int i = 0; i < count; ++i) {
        Position pos;
        for (Move move; pos.getRandomMove(gen, move); pos.doMove(move)) {
        }
        wins += pos.turns & 1;
      }
      auto dt = getDeltaTimeSince(start);
      cout.precision(2);
      cout.setf(ios::fixed);
      cout << "Run " << count << " playouts in " << dt << " seconds" << endl;
      cout << 0.001 * count / dt << "k playout/s" << endl;
      cout << "w=" << 100.0f * wins / count << "%" << endl;
      return 0;
    }

    if (argv[1] == string("--benchmark-simulation")) {
      auto start = getTimePoint();
      constexpr int count = 100000;
      McRaveAgent agent;
      Position pos;
      for (int i = 0; i < count; ++i) {
        agent.simulate(pos);
      }
      auto dt = getDeltaTimeSince(start);
      cout.precision(2);
      cout.setf(ios::fixed);
      cout << "Run " << count << " simulations in " << dt << " seconds" << endl;
      cout << "Speed=" << 0.001 * count / dt << "k it/s" << endl;
      // Thu Jan 21 23:34:04 CET 2021
      // Run 100000 simulations in 7.41 seconds
      // Speed=13.49k it/s
      return 0;
    }
  }

  Position pos;
  McRaveAgent agent;
  agent.pickTransformation();
  for (string s; cin >> s && s != "Quit";) {
    if (s != "Start") {
      cerr << s << endl;
      pos.doMove(s);
    }

    auto [claimWin, bestMove] = agent.getBestMove(pos);
    pos.doMove(bestMove);
    cerr << bestMove;
    cout << bestMove;

    if (claimWin) {
      cerr << "!";
      cout << "!";
    }
    cerr << endl;
    cout << endl;
  }

  return 0;
}
