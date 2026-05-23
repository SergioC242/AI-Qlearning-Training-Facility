#pragma once
#include "Die.h"
#include "Card.h"
#include <vector>
#include <array>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
//  CombatState
//  Everything the AI can observe at decision time.
//  Encodes into a single int for the Q-table key.
// ─────────────────────────────────────────────────────────────────────────────
struct CombatState
{
    int  playerScore = 0;
    int  playerHp    = 0;
    int  enemyHp     = 0;
    int  hiLoCount   = 0;
    float bustProb   = 0.0f;

    std::array<bool, NUM_DICE> diceAvail  = {};
    std::array<int,  NUM_DICE> diceValues = {};

    // ── State encoding ────────────────────────────────────────────────────────
    //
    //   bits  0-4  : playerScore clamped to [0,21]          (5 bits)
    //   bits  5-9  : hiLoCount shifted & clamped [-10,10]   (5 bits)
    //   bits 10-11 : bustProb bucket [0-25,25-50,50-75,75+] (2 bits)
    //   bits 12+   : dice availability mask (NUM_DICE bits)
    //
    int encode() const
    {
        int score    = std::min(playerScore, 21);
        int hiLo     = std::clamp(hiLoCount + 10, 0, 20);
        int bpBucket = static_cast<int>(bustProb * 4.0f);
        if (bpBucket > 3) bpBucket = 3;

        int diceMask = 0;
        for (int i = 0; i < NUM_DICE; ++i)
            if (diceAvail[i]) diceMask |= (1 << i);

        return score
             | (hiLo     << 5)
             | (bpBucket << 10)
             | (diceMask << 12);
    }
};
