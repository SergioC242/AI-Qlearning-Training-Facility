#pragma once
#include <map>
#include <vector>
#include <utility>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <algorithm>

/// @brief AI agent that learns to play combat rounds via Q-learning.
///
/// State  = player's current score (int, 0–21+).
/// Action = one of the values in AIAction.
/// The Q-table is stored as a std::map<std::pair<int, AIAction>, float>.
///
/// Because rewards are delayed (given at the end of a round / game),
/// the agent records every (state, action) pair it chose during the
/// round, then propagates the final reward back through the history.
class AIPlayer
{
public:

    // ── Actions the AI can take ──────────────────────────────────────────────
    enum class AIAction
    {
        HIT_CARD,      // Draw a card from the deck
        USE_DIE_1,     
        USE_DIE_2,     
        USE_DIE_3,     
        USE_DIE_4,     
        USE_DIE_5,     
        USE_DIE_6,     
        STAND,         // End the turn

        COUNT          
    };

    // ── Constructor / Destructor ─────────────────────────────────────────────
    /// @param alpha   Learning rate  (default 0.1)
    /// @param gamma   Discount factor (default 0.9)
    /// @param epsilon Exploration rate (default 0.2)
    AIPlayer(float alpha = 0.1f, float gamma = 0.9f, float epsilon = 0.2f);
    ~AIPlayer() = default;

    // ── Core API ─────────────────────────────────────────────────────────────

    /// Choose the next action for the given score using ε-greedy policy.
    /// The (state, action) pair is automatically recorded in the history.
    /// @param currentScore  The player's current blackjack-style score.
    /// @param availableActions  List of actions currently legal (e.g. dice
    ///                          that haven't been used yet).  If empty the
    ///                          agent picks from ALL actions.
    AIAction chooseAction(int currentScore,
                          const std::vector<AIAction>& availableActions = {});

    // ── Reward propagation ───────────────────────────────────────────────────

    /// Call when the player busts (score > 21).  Applies a –1 reward to
    /// every (state, action) in the current history and clears it.
    void rewardBust();

    /// Call at the end of a round with the round result.
    /// @param won  true = player won the round, false = player lost.
    /// Applies +1 (win) or –1 (loss) and clears the round history.
    void rewardRoundEnd(bool won);

    /// Call at the end of the full enemy encounter.
    /// @param won  true = player defeated the enemy, false = player lost.
    /// Applies +10 (win) or –10 (loss) and clears the round history.
    void rewardGameEnd(bool won);

    /// Call when the round is a draw.
    /// Applies 0 reward (still runs the Q-update with γ·maxQ) and clears
    /// the round history.
    void rewardDraw();

    /// Manually clear the action history without applying any reward.
    void clearHistory();

    // ── Persistence ──────────────────────────────────────────────────────────

    /// Save the Q-table to a text file so learning persists across runs.
    void saveQTable(const std::string& filepath) const;

    /// Load a previously saved Q-table.
    void loadQTable(const std::string& filepath);

    // ── Accessors ────────────────────────────────────────────────────────────

    float getQValue(int state, AIAction action) const;
    void  setEpsilon(float e) { _epsilon = e; }
    float getEpsilon() const  { return _epsilon; }
    const std::map<std::pair<int, AIAction>, float>& getQTable() const { return _qTable; }

private:

    // ── Q-learning hyper-parameters ──────────────────────────────────────────
    float _alpha;    // learning rate
    float _gamma;    // discount factor
    float _epsilon;  // exploration rate (ε-greedy)

    // ── Q-table  (state, action) → value ─────────────────────────────────────
    // Using std::map (ordered) so the table is easy to inspect / serialise.
    std::map<std::pair<int, AIAction>, float> _qTable;

    // ── Episode history ──────────────────────────────────────────────────────
    // Stores every (state, action) the agent chose this round so that the
    // delayed reward can be back-propagated once the outcome is known.
    std::vector<std::pair<int, AIAction>> _history;

    // ── Internal helpers ─────────────────────────────────────────────────────

    /// Return the maximum Q-value over all actions for a given state.
    float maxQ(int state) const;

    /// Apply a single reward to the full history and clear it.
    void applyReward(float reward);

    /// All possible actions (cached for convenience).
    static const std::vector<AIAction>& allActions();
};
