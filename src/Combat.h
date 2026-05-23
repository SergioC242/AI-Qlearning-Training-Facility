#pragma once
#include "Card.h"
#include "Deck.h"
#include "Die.h"
#include "Enemy.h"
#include "CombatState.h"
#include "AIPlayer.h"

#include <vector>
#include <array>
#include <iostream>
#include <string>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
//  RoundResult
// ─────────────────────────────────────────────────────────────────────────────
enum class RoundResult { PLAYER_WIN, ENEMY_WIN, DRAW };

// ─────────────────────────────────────────────────────────────────────────────
//  Combat
//
//  AI-only combat encounter. No human input.
//
//  Damage rules:
//    PLAYER WIN  → damage to enemy  = playerScore - enemyScore
//    PLAYER LOSE → damage to player = enemyScore - playerScore
//    BLACKJACK   → critical: enemy HP set to 0 instantly
//    DRAW        → no damage; dice restored
//
//  Enemy rule: draws until score > 15, then stands.
// ─────────────────────────────────────────────────────────────────────────────
class Combat
{
public:
    Combat(int playerMaxHp,
           int enemyMaxHp,
           const std::string& enemyName = "Enemy")
        : _playerMaxHp(playerMaxHp)
        , _playerHp(playerMaxHp)
        , _enemy(enemyMaxHp, enemyName)
    {}

    // ── Accessors ─────────────────────────────────────────────────────────────

    int  playerHp()       const { return _playerHp; }
    int  playerMaxHp()    const { return _playerMaxHp; }
    const Enemy& enemy()  const { return _enemy; }
    const Deck&  deck()   const { return _deck; }
    int  roundNumber()    const { return _round; }

    bool playerDefeated() const { return _playerHp <= 0; }
    bool enemyDefeated()  const { return _enemy.isDefeated(); }
    bool isOver()         const { return playerDefeated() || enemyDefeated(); }

    // ── Main round ────────────────────────────────────────────────────────────

    RoundResult runRound(AIPlayer& ai, bool verbose = false)
    {
        ++_round;

        if (verbose)
        {
            std::cout << "\n══════════════════════════════\n";
            std::cout << "  ROUND " << _round << "\n";
            std::cout << "  AI HP : " << _playerHp << " / " << _playerMaxHp << "\n";
            std::cout << "  " << _enemy.name() << " HP: "
                      << _enemy.hp() << " / " << _enemy.maxHp() << "\n";
            std::cout << "══════════════════════════════\n";
        }

        // Reshuffle if deck is running low
        if (_deck.remaining() < 10)
        {
            if (verbose) std::cout << "[Deck reshuffled]\n";
            _deck.reset();
        }

        // Clear both hands
        _playerHand.clear();
        _enemy.clearHand();

        // Deal two cards to each side
        _playerHand.push_back(_deck.draw());
        _playerHand.push_back(_deck.draw());

        std::vector<Card> enemyStart = { _deck.draw(), _deck.draw() };
        injectEnemyCards(enemyStart);

        int ps = playerScore();

        if (verbose)
        {
            std::cout << "  AI hand: ";
            for (const auto& c : _playerHand) std::cout << c.name() << " ";
            std::cout << "-> " << ps << "\n";
        }

        // ── AI player turn ────────────────────────────────────────────────────
        bool playerBusted = false;
        bool blackjack    = (ps == 21);

        if (blackjack)
        {
            if (verbose) std::cout << "  ★ BLACKJACK!\n";
        }
        else
        {
            aiTurn(ai, playerBusted, verbose);
            ps = playerScore();
        }

        // ── Enemy turn (skipped if AI busted) ─────────────────────────────────
        RoundResult result;

        if (playerBusted)
        {
            if (verbose) std::cout << "  AI BUSTS with " << ps << ".\n";
            result = RoundResult::ENEMY_WIN;
        }
        else
        {
            _enemy.playTurn(_deck, verbose);
            result = resolveRound(ps, blackjack, verbose);
        }

        // ── Damage ────────────────────────────────────────────────────────────
        applyDamage(result, ps, blackjack, verbose);

        // ── Rewards ───────────────────────────────────────────────────────────
        if (playerBusted)
            ai.rewardBust();
        else
        {
            switch (result)
            {
            case RoundResult::PLAYER_WIN: ai.rewardRoundEnd(true);  break;
            case RoundResult::ENEMY_WIN:  ai.rewardRoundEnd(false); break;
            case RoundResult::DRAW:       ai.rewardDraw();          break;
            }
        }

        if (isOver())
            ai.rewardGameEnd(!playerDefeated());

        // Restore dice on draw
        if (result == RoundResult::DRAW)
        {
            for (auto& d : _dice) d.restore();
            if (verbose) std::cout << "  [Draw] Dice restored.\n";
        }

        return result;
    }

    // ── Full encounter ────────────────────────────────────────────────────────

    void runEncounter(AIPlayer& ai, bool verbose = false)
    {
        while (!isOver())
            runRound(ai, verbose);

        if (verbose)
        {
            std::cout << "\n══════════════════════════════\n";
            std::cout << (playerDefeated() ? "  DEFEAT\n" : "  VICTORY\n");
            std::cout << "══════════════════════════════\n";
        }
    }

private:
    int   _playerMaxHp;
    int   _playerHp;
    Enemy _enemy;
    int   _round = 0;

    Deck              _deck;
    std::array<Die,6> _dice;
    std::vector<Card> _playerHand;

    // ── Helpers ───────────────────────────────────────────────────────────────

    int playerScore() const { return handScore(_playerHand); }

    void injectEnemyCards(const std::vector<Card>& cards)
    {
        for (const auto& c : cards)
            const_cast<std::vector<Card>&>(_enemy.hand()).push_back(c);
    }

    // ── AI turn ───────────────────────────────────────────────────────────────

    void aiTurn(AIPlayer& ai, bool& busted, bool verbose)
    {
        while (true)
        {
            int ps = playerScore();
            if (ps > 21) { busted = true; return; }

            // Available actions: HIT, unused dice, STAND
            std::vector<AIPlayer::AIAction> available;
            available.push_back(AIPlayer::AIAction::HIT_CARD);
            for (int i = 0; i < 6; ++i)
                if (!_dice[i].isUsed())
                    available.push_back(static_cast<AIPlayer::AIAction>(
                        static_cast<int>(AIPlayer::AIAction::USE_DIE_1) + i));
            available.push_back(AIPlayer::AIAction::STAND);

            // Encode state
            CombatState cs;
            cs.playerScore = ps;
            cs.playerHp    = _playerHp;
            cs.enemyHp     = _enemy.hp();
            cs.hiLoCount   = _deck.hiLoCount();
            cs.bustProb    = _deck.bustProbability(ps);
            for (int i = 0; i < 6; ++i)
            {
                cs.diceAvail[i]  = !_dice[i].isUsed();
                cs.diceValues[i] = _dice[i].value();
            }

            AIPlayer::AIAction action = ai.chooseAction(cs.encode(), available);

            if (action == AIPlayer::AIAction::STAND)
            {
                if (verbose) std::cout << "  AI STANDS at " << ps << "\n";
                break;
            }

            if (action == AIPlayer::AIAction::HIT_CARD)
            {
                if (_deck.empty()) break;
                Card c = _deck.draw();
                _playerHand.push_back(c);
                if (verbose)
                    std::cout << "  AI draws " << c.name()
                              << "  -> " << playerScore() << "\n";
            }
            else
            {
                int dieIdx = static_cast<int>(action)
                           - static_cast<int>(AIPlayer::AIAction::USE_DIE_1);
                if (dieIdx < 0 || dieIdx >= 6 || _dice[dieIdx].isUsed())
                    continue;

                int rolled = _dice[dieIdx].roll();
                Card fakeCard{ Suit::CLUBS, static_cast<Rank>(rolled + 1) };
                _playerHand.push_back(fakeCard);
                if (verbose)
                    std::cout << "  AI uses die " << (dieIdx+1)
                              << " (+" << rolled << ")  -> " << playerScore() << "\n";
            }
        }
    }

    // ── Resolution ────────────────────────────────────────────────────────────

    RoundResult resolveRound(int ps, bool blackjack, bool verbose) const
    {
        int  es      = _enemy.score();
        bool eBusted = _enemy.busted();

        if (verbose)
        {
            std::cout << "  AI: " << ps << "  |  "
                      << _enemy.name() << ": " << es
                      << (eBusted ? " (BUST)" : "") << "\n";
        }

        if (blackjack || eBusted || ps > es)
        {
            if (verbose) std::cout << "  -> AI wins the round\n";
            return RoundResult::PLAYER_WIN;
        }
        if (ps == es)
        {
            if (verbose) std::cout << "  -> Draw\n";
            return RoundResult::DRAW;
        }
        if (verbose) std::cout << "  -> " << _enemy.name() << " wins the round\n";
        return RoundResult::ENEMY_WIN;
    }

    // ── Damage ────────────────────────────────────────────────────────────────

    void applyDamage(RoundResult result, int ps, bool blackjack, bool verbose)
    {
        int es = _enemy.busted() ? 0 : _enemy.score();

        switch (result)
        {
        case RoundResult::PLAYER_WIN:
            if (blackjack)
            {
                _enemy.takeDamage(_enemy.hp());   // instant kill on blackjack
                if (verbose)
                    std::cout << "  Critical! " << _enemy.name() << " HP -> 0\n";
            }
            else
            {
                int dmg = ps - es;                // difference
                _enemy.takeDamage(dmg);
                if (verbose)
                    std::cout << "  Enemy takes " << dmg
                              << " dmg  (HP: " << _enemy.hp() << ")\n";
            }
            break;

        case RoundResult::ENEMY_WIN:
        {
            int loss = es - ps;                   // difference
            _playerHp -= loss;
            if (_playerHp < 0) _playerHp = 0;
            if (verbose)
                std::cout << "  AI loses " << loss
                          << " HP  (HP: " << _playerHp << ")\n";
            break;
        }

        case RoundResult::DRAW:
            break;
        }
    }
};
