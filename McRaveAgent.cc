#include "McRaveAgent.h"

RNG StateInfo::rng;

// 100 millisconds for maximum reading/writing overhead
McRaveAgent::McRaveAgent() { totalTime = 0.1; }

void McRaveAgent::simulate(const Position &pos) {
  auto tmpPos = pos;
  IterationResult result;
  result.firstStateBlack = pos.turns & 1;
  int r = simulateTree(tmpPos, result);
  if (r == 0) {
    result.value = tmpPos.turns & 1 ? OO : -OO;
  } else if (r <= 5) {
    int winningAction = getWinningAction(tmpPos);
    if (winningAction == -1) {
      result.value = tmpPos.turns & 1 ? OO : -OO;
    } else {
      result.add(tmpPos.state, winningAction);
      result.value = tmpPos.turns & 1 ? -OO : OO;
    }
  } else {
    simulateDefault(tmpPos, result);
  }
  backup(result);
}

int McRaveAgent::getWinningAction(const Position &pos) {
  for (const auto &move : pos) {
    auto tmpPos = pos;
    tmpPos.doMove(move);
    if (getWinningAction(tmpPos) == -1) {
      return move.wall;
    }
  }
  return -1;
}

void McRaveAgent::simulateDefault(const Position &pos,
                                  IterationResult &result) {
  result.value = 0.0f;
  for (int s = 0; s < samples; ++s) {
    int len = 0;
    pair<bool, int> actions[60];
    auto tmpPos = pos;
    for (Move move; tmpPos.getRandomMove(gen, move);) {
      actions[len++] = {tmpPos.turns & 1, move.wall};
      tmpPos.doMove(move);
    }
    auto value = tmpPos.turns & 1 ? 1.0f : -1.0f;
    result.value += value;
    for (int i = 0; i < len; ++i) {
      auto [black, action] = actions[i];
      auto &amafStats = result.amafStats[action];
      if (black) {
        amafStats.black.update(value);
      } else {
        amafStats.white.update(value);
      }
      amafStats.any.update(value);
    }
  }

  result.value /= samples;
}

int McRaveAgent::getDepth(const Position &pos) {
  int depth = 0;
  auto tmpPos = pos;
  while (!tmpPos.isEndGame()) {
    auto state = tmpPos.state;
    if (!contains(state)) break;
    const auto &stateInfo = m[state];
    if (stateInfo.isLosing() || stateInfo.isWinning()) return 1000;
    int next = stateInfo.selectMostVisited();
    if (stateInfo.actionInfo[next].isLosing()) {
      next = stateInfo.select();
    }
    auto move = tmpPos.getMove(next);
    ++depth;
    tmpPos.doMove(move);
  }
  return depth;
}

int McRaveAgent::simulateTree(Position &pos, IterationResult &result,
                              StateInfo *lastState, ActionInfo *lastAction) {
  if (pos.isEndGame()) return 0;

  auto state = pos.state;
  if (!contains(state)) {
    if (m.size() < 120000) {
      auto &newState = newNode(pos);
      Move move;
      pos.getRandomMove(gen, move);
      auto action = move.wall;
      pos.doMove(move);
      result.add(state, action);
      return newState.actionsCount;
    }
    return lastState ? lastState->actionsCount : 60;
  }

  auto &stateInfo = m[state];
  if (lastAction != nullptr) {
    lastAction->impact = lastState->actionsCount - stateInfo.actionsCount;
  }
  if (stateInfo.isLosing()) return 0;
  auto move = select(pos);
  auto &actionInfo = stateInfo.actionInfo[move.wall];
  auto action = move.wall;
  pos.doMove(move);
  result.add(state, action);
  return simulateTree(pos, result, &stateInfo, &actionInfo);
}

float McRaveAgent::eval(const Position &pos, const Move &move) {
  return m[pos.state].eval(move.wall);
}

Move McRaveAgent::select(const Position &pos) {
  const auto &stateInfo = m[pos.state];
  if ((pos.turns & 1) != me) {
    int a =
        gen.lessThan(10) == 0 ? stateInfo.selectRandom() : stateInfo.select();
    return pos.getMove(a);
  }

  int a = stateInfo.select();
  return pos.getMove(a);
}

Move McRaveAgent::selectMostVisited(const Position &pos) {
  int mostVisited = m[pos.state].selectMostVisited();
  return pos.getMove(mostVisited);
}

void McRaveAgent::backup(const IterationResult &result) {
  const int T = result.countTransitions;
  float value = result.value;
  int relevantActions[60], relevantActionsCount = 0;
  for (int a = 0; a < 60; ++a) {
    const auto &stats = result.amafStats[a];
    if (stats) relevantActions[relevantActionsCount++] = a;
  }
  for (int t = T - 1; t >= 0; --t) {
    auto [st, at] = result.transitions[t];
    auto &qs = m[st];
    bool black = t & 1 ? !result.firstStateBlack : result.firstStateBlack;
    float v = black ? -value : value;
    if (isExactWin(v)) {
      qs.markWinning(at);
      continue;
    }
    if (isExactLoss(v)) {
      qs.markLosing(at);
      bool loss = all_of(qs.actionInfo, qs.actionInfo + 60,
                         [](const ActionInfo &info) -> bool {
                           return !info || info.isLosing();
                         });
      if (loss) {
        qs.markLosing();
        continue;
      } else {
        value /= OO;
        v = black ? -value : value;
      }
    }

    qs.updateQ1(at, v, samples);

    bool samePlayer = true;
    for (int u = t; u < T; ++u) {
      int au = result.transitions[u].second;
      qs.updateQ3(au, v, samples);
      if (samePlayer) qs.updateQ2(au, v, samples);
      samePlayer = !samePlayer;
    }

    for (int i = 0; i < relevantActionsCount; ++i) {
      int a = relevantActions[i];
      const auto &amafStats = result.amafStats[a];
      auto &actionInfo = qs.actionInfo[a];
      if (black) {
        actionInfo.q2 -= amafStats.black;
        actionInfo.q3 -= amafStats.any;
      } else {
        actionInfo.q2 += amafStats.white;
        actionInfo.q3 += amafStats.any;
      }
    }
  }
}

StateInfo &McRaveAgent::newNode(const Position &pos) {
  auto &info = m[pos.state];
  info.invalid = (~pos.placed) & (~pos.possibleWalls);
  for (const Move &move : pos) {
    info.actionsCount++;

    int w = move.wall;
    if (pos.turns >= 20 || me != (pos.turns & 1) ||
        goodOpeningMove[pos.turns & 1][w]) {
      info.actionInfo[w].status = UNKNOWN;
      info.actionInfo[w].impact = pos.getImpact(move);
    }
  }
  return info;
}

void McRaveAgent::log(const Position &pos, const Move &move) {
  const auto &info = m[pos.state].actionInfo[move.wall];

  if (info.isWinning()) {
    cerr << "Winning move!" << endl;
    return;
  }

  if (info.isLosing()) {
    cerr << "Losing move!" << endl;
    return;
  }

  cerr << "w=" << 50.0f * (1.0f + info.q1.value) << "%" << endl;
  cerr << "e=" << 50.0f * (1.0f + m[pos.state].eval(move.wall)) << "%" << endl;
}

pair<bool, Move> McRaveAgent::getBestMove(const Position &pos,
                                          bool useTimeConstraint) {
  cerr << fixed << setprecision(2);
  const auto start = getTimePoint();
  me = pos.turns & 1;

  auto transformed = transformState(pos.placed);
  if (auto it = openingBook.find(transformed); it != openingBook.end()) {
    cerr << "From opening book" << endl;
    const int *inv = transformations[inverse[transformationIndex]];
    return {false, {inv[it->second], {}}};
  }

  auto maxTime = defaultMaxTime;
  if (pos.turns >= 18) {
    const double maxMoveTime = 2.75;
    maxTime = min(maxMoveTime, (maxTotalTime - totalTime) / 2);
  }
  cerr << "max-time=" << maxTime << endl;

  clean(pos);
  int i = 0;
  for (; i < maxIterations; ++i) {
    simulate(pos);
    const auto &stateInfo = m[pos.state];
    if (stateInfo.isWinning()) {
      auto dt = getDeltaTimeSince(start);
      totalTime += dt;
      cerr << "i=" << i << " dt=" << dt << " tt=" << totalTime << endl;
      cerr << "=>Win found! turn=" << pos.turns + 1 << endl;
      const auto winningMove = pos.getMove(stateInfo.winningAction);
      bool claimWin = canClaimWin;
      canClaimWin = false;
      return {claimWin, winningMove};
    }
    if (stateInfo.isLosing()) {
      // in this case just choose the most visited
      auto dt = getDeltaTimeSince(start);
      totalTime += dt;
      cerr << "i=" << i << " dt=" << dt << " tt=" << totalTime << endl;
      cerr << "Game lost! Playing most visited anyway.." << endl;
      return {false, selectMostVisited(pos)};
    }

    if (useTimeConstraint &&
        getDeltaTimeSince(start) >= maxTime + maxCheckTime) {
      break;
    }

    if (useTimeConstraint && getDeltaTimeSince(start) >= maxTime) {
      if (select(pos) == selectMostVisited(pos)) break;
    }
  }

  auto bestMove = selectMostVisited(pos);
  auto info = m[pos.state].actionInfo[bestMove.wall];
  if (info.isLosing()) {
    bestMove = select(pos);
    info = m[pos.state].actionInfo[bestMove.wall];
    cerr << "Most visited is losing. switching to best selection.." << endl;
  }

  log(pos, bestMove);
  auto dt = getDeltaTimeSince(start);
  auto depth = getDepth(pos);
  totalTime += dt;
  auto speed = 0.001 * static_cast<double>(i) / dt;
  cerr << "i=" << (i + 500) / 1000 << "k d=" << depth << " dt=" << dt
       << " tt=" << totalTime << " " << speed << "k it/s" << endl;
  cerr << "impact=" << info.impact << endl;
  const float value = info.q1.value;
  bool claimWin = canClaimWin && pos.turns >= 18 && value >= 0.34f;
  if (claimWin) canClaimWin = false;

  return {claimWin, bestMove};
}

void McRaveAgent::launchDebugSession(const Position &pos) {
  cerr << fixed << setprecision(3);
  int maxDebugIterations;
  cout << "Enter max debug iterations" << endl;
  cin >> maxDebugIterations;
  int i = 0;
  for (; i < maxDebugIterations; ++i) {
    simulate(pos);
    const auto &stateInfo = m[pos.state];
    if (stateInfo.isLosing() || stateInfo.isWinning()) break;
  }
  cout << "i=" << i << endl;
  cout << ">>use (c) for listing current state children" << endl;
  cout << ">>use children wall to select a child(i.e A1h)" << endl;
  cout << ">>";
  auto t = pos;
  for (string s; cin >> s && s != "q";) {
    if (s == "c") {
      auto mostVisited = selectMostVisited(t);
      auto best = select(t);
      for (const Move &move : t) {
        log(t, move);
        const auto &info = m[t.state].actionInfo[move.wall];
        cout << move << " " << info.q1 << " " << info.q2 << " " << info.q3;
        if (mostVisited == move) cout << "(most visited one)";
        if (best == move) cout << "(best one)";
        cout << endl;
      }
    } else {
      t.doMove(s);
    }
    cout << "\n>>";
  }
  cout << ">> Debugging session finished" << endl;
}
