#pragma once
#include "Card.h"
#include "Die.h"
#include <vector>
#include <array>

// ─────────────────────────────────────────────────────────────────────────────
//  CombatState
//  Everything the AI (and the player) can observe at decision time.
//  Encodes into a single int for the Q-table key.
// ─────────────────────────────────────────────────────────────────────────────
struct CombatState
{
    // ── Observable ───────────────────────────────────────────────────────────
    int  playerScore  = 0;          // current hand value (0-21+)
    int  playerHp     = 0;          // remaining HP
    int  enemyHp      = 0;          // remaining enemy HP
    int  hiLoCount    = 0;          // deck Hi-Lo running count (card counting)
    float bustProb    = 0.0f;       // P(next card busts the player)
    std::array<bool,6> diceAvail = {true,true,true,true,true,true};
    std::array<int ,6> diceValues= {1,1,1,1,1,1};

    // ── State encoding for Q-table ───────────────────────────────────────────
    //
    //  We pack the most strategically relevant variables into a compact int:
    //
    //   bits  0-4  : playerScore clamped to [0,21]       (5 bits, 22 values)
    //   bits  5-9  : hiLoCount shifted & clamped [-10,10] (5 bits, 21 values)
    //   bits 10-11 : bustProb bucket [0-25%, 25-50%, 50-75%, 75-100%] (2 bits)
    //   bits 12-17 : dice availability mask (6 bits)
    //
    //  This gives a state space of ~22 × 21 × 4 × 64 ≈ 118 k states,
    //  manageable for a std::map Q-table.
    int encode() const
    {
        int score    = std::min(playerScore, 21);
        int hiLo     = std::clamp(hiLoCount + 10, 0, 20);   // shift to [0,20]
        int bpBucket = static_cast<int>(bustProb * 4.0f);
        if (bpBucket > 3) bpBucket = 3;

        int diceMask = 0;
        for (int i = 0; i < 6; ++i)
            if (diceAvail[i]) diceMask |= (1 << i);

        return score
             | (hiLo     << 5)
             | (bpBucket << 10)
             | (diceMask << 12);
    }
};
