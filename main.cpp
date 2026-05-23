#include "Combat.h"
#include "AIPlayer.h"
#include <iostream>
#include <string>
#include <iomanip>

// ─────────────────────────────────────────────────────────────────────────────
//  Config
// ─────────────────────────────────────────────────────────────────────────────
static constexpr int  PLAYER_HP  = 100;
static constexpr int  ENEMY_HP   = 80;
static constexpr int  MIN_BET    = 10;
static const std::string QTABLE  = "qtable.txt";
static const std::string ENEMY_NAME = "Goblin";

// ─────────────────────────────────────────────────────────────────────────────
//  Train
//  Epsilon decays from 1.0 (pure exploration) to 0.05 (mostly exploit).
// ─────────────────────────────────────────────────────────────────────────────
static void train(int episodes)
{
    AIPlayer ai(0.1f, 0.9f, 1.0f);
    ai.loadQTable(QTABLE);

    int wins = 0, losses = 0;
    const int logEvery = std::max(1, episodes / 10);

    for (int ep = 0; ep < episodes; ++ep)
    {
        float eps = 1.0f - 0.95f * (static_cast<float>(ep) / static_cast<float>(episodes));
        if (eps < 0.05f) eps = 0.05f;
        ai.setEpsilon(eps);

        Combat combat(PLAYER_HP, ENEMY_HP, MIN_BET, ENEMY_NAME);

        // Silence output during training
        std::streambuf* old = std::cout.rdbuf(nullptr);
        combat.runEncounter(ai, false);
        std::cout.rdbuf(old);

        if (combat.playerDefeated()) ++losses; else ++wins;

        if ((ep + 1) % logEvery == 0)
        {
            float winRate = 100.f * static_cast<float>(wins) / static_cast<float>(wins + losses);
            std::cout << "  ep " << std::setw(7) << (ep+1)
                      << "  eps=" << std::fixed << std::setprecision(3) << eps
                      << "  win% " << std::setprecision(1) << winRate
                      << "  Q-entries " << ai.getQTable().size() << "\n";
            wins = losses = 0;
        }
    }

    ai.saveQTable(QTABLE);
    std::cout << "\nDone. Q-table saved to " << QTABLE << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
//  Watch — one encounter with full logging
// ─────────────────────────────────────────────────────────────────────────────
static void watch()
{
    AIPlayer ai(0.1f, 0.9f, 0.0f);   // epsilon=0 → pure exploit
    ai.loadQTable(QTABLE);

    Combat combat(PLAYER_HP, ENEMY_HP, MIN_BET, ENEMY_NAME);
    combat.runEncounter(ai, true);

    std::cout << "\nFinal AI HP  : " << combat.playerHp() << "\n";
    std::cout << "Final enemy HP: " << combat.enemy().hp() << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "=== BlackJack Combat — AI only ===\n\n";
    std::cout << "  [1] Train AI\n";
    std::cout << "  [2] Watch AI play\n";
    std::cout << "> ";

    int choice = 1;
    std::cin >> choice;

    if (choice == 1)
    {
        std::cout << "Episodes (e.g. 50000): ";
        int ep = 50000;
        std::cin >> ep;
        std::cout << "\nTraining " << ep << " episodes...\n\n";
        train(ep);
    }
    else
    {
        watch();
    }

    return 0;
}
