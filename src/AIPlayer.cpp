#include "AIPlayer.h"
#include <iostream>
#include <limits>
#include <sstream>
#include <cassert>

AIPlayer::AIPlayer(float alpha, float gamma, float epsilon)
    : _alpha(alpha), _gamma(gamma), _epsilon(epsilon)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

const std::vector<AIPlayer::AIAction>& AIPlayer::allActions()
{
    static const std::vector<AIAction> actions = {
        AIAction::HIT_CARD,
        AIAction::USE_DIE_1,
        AIAction::USE_DIE_2,
        AIAction::USE_DIE_3,
        AIAction::USE_DIE_4,
        AIAction::USE_DIE_5,
        AIAction::USE_DIE_6,
        AIAction::STAND
    };
    return actions;
}

float AIPlayer::getQValue(int state, AIAction action) const
{
    auto it = _qTable.find({ state, action });
    if (it != _qTable.end())
        return it->second;
    return 0.0f;  // unseen (state, action) pairs default to 0
}

float AIPlayer::maxQ(int state) const
{
    float best = -std::numeric_limits<float>::infinity();
    for (const auto& a : allActions())
    {
        float q = getQValue(state, a);
        if (q > best) best = q;
    }
    return best;
}
AIPlayer::AIAction AIPlayer::chooseAction(
    int currentScore,
    const std::vector<AIAction>& availableActions)
{
    const std::vector<AIAction>& pool =
        availableActions.empty() ? allActions() : availableActions;

    AIAction chosen;

    // Explore: busca las distintas opciones legales y elige una al azar
    float roll = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    if (roll < _epsilon)
    {
        int idx = std::rand() % static_cast<int>(pool.size());
        chosen = pool[idx];
    }
    else
    {
        // Exploit: pick the action with the highest Q-value
        float bestVal = -std::numeric_limits<float>::infinity();
        chosen = pool[0];

        for (const auto& a : pool)
        {
            float q = getQValue(currentScore, a);
            if (q > bestVal)
            {
                bestVal = q;
                chosen = a;
            }
        }
    }

    // Record the (state, action) so we can back-propagate the reward later
    _history.push_back({ currentScore, chosen });
    return chosen;
}
void AIPlayer::applyReward(float reward)
{
    if (_history.empty()) return;

    // Walk the history in reverse.
    // For the LAST action the "next-state Q" is 0 (terminal).
    // For earlier actions we use the recorded next-state to compute γ·maxQ.
    //
    //   Q(s,a) ← Q(s,a) + α · [ r + γ · max_a' Q(s',a') − Q(s,a) ]
    //
    // Because intermediate rewards are 0, `r` is only non-zero at the last
    // step.  For previous steps we still run the update so the value
    // propagates backwards through the trajectory.

    for (int i = static_cast<int>(_history.size()) - 1; i >= 0; --i)
    {
        int      s = _history[i].first;
        AIAction a = _history[i].second;

        float currentQ = getQValue(s, a);

        float r = (i == static_cast<int>(_history.size()) - 1) ? reward : 0.0f;

        float nextMax = 0.0f;
        if (i + 1 < static_cast<int>(_history.size()))
        {
            int nextState = _history[i + 1].first;
            nextMax = maxQ(nextState);
        }

        float newQ = currentQ + _alpha * (r + _gamma * nextMax - currentQ);
        _qTable[{ s, a }] = newQ;
    }

    _history.clear();
}

void AIPlayer::rewardBust()
{
    applyReward(-1.0f);
}

void AIPlayer::rewardRoundEnd(bool won)
{
    applyReward(won ? 1.0f : -1.0f);
}

void AIPlayer::rewardGameEnd(bool won)
{
    applyReward(won ? 10.0f : -10.0f);
}

void AIPlayer::rewardDraw()
{
    applyReward(0.0f);
}

void AIPlayer::clearHistory()
{
    _history.clear();
}

void AIPlayer::saveQTable(const std::string& filepath) const
{
    std::ofstream out(filepath);
    if (!out.is_open())
    {
        std::cerr << "[AIPlayer] Could not open file for writing: "
                  << filepath << "\n";
        return;
    }

    for (const auto& entry : _qTable)
    {
        int   state  = entry.first.first;
        int   action = static_cast<int>(entry.first.second);
        float qval   = entry.second;
        out << state << "," << action << "," << qval << "\n";
    }
    out.close();
}

void AIPlayer::loadQTable(const std::string& filepath)
{
    std::ifstream in(filepath);
    if (!in.is_open())
    {
        std::cerr << "[AIPlayer] Could not open file for reading: "
                  << filepath << " (starting fresh)\n";
        return;
    }

    _qTable.clear();

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string tok;

        std::getline(ss, tok, ','); int state     = std::stoi(tok);
        std::getline(ss, tok, ','); int actionInt  = std::stoi(tok);
        std::getline(ss, tok, ','); float qval     = std::stof(tok);

        _qTable[{ state, static_cast<AIAction>(actionInt) }] = qval;
    }
    //std::cerr << "[AIPlayer] Loaded " << _qTable.size() << " Q-table entries from " << filepath << "\n";
    in.close();
}