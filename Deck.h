#pragma once
#include "Card.h"
#include <vector>
#include <array>
#include <algorithm>
#include <random>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
//  Deck
//  One standard 52-card deck shared for the entire combat encounter.
//  Supports card counting: seenCards() returns every card dealt so far.
// ─────────────────────────────────────────────────────────────────────────────
class Deck
{
public:
    Deck()
    {
        reset();
    }

    /// Rebuild and reshuffle the full 52-card deck.
    void reset()
    {
        _cards.clear();
        _seen.clear();

        for (int s = 0; s < 4; ++s)
            for (int r = 2; r <= 14; ++r)
                _cards.push_back({ static_cast<Suit>(s),
                                   static_cast<Rank>(r) });

        shuffle();
    }

    void shuffle()
    {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::shuffle(_cards.begin(), _cards.end(), rng);
    }

    /// Draw the top card.  Throws if the deck is empty.
    Card draw()
    {
        if (_cards.empty())
            throw std::runtime_error("Deck is empty");
        Card c = _cards.back();
        _cards.pop_back();
        _seen.push_back(c);
        return c;
    }

    int  remaining()  const { return static_cast<int>(_cards.size()); }
    bool empty()      const { return _cards.empty(); }

    // ── Card counting ────────────────────────────────────────────────────────

    /// All cards dealt so far this encounter.
    const std::vector<Card>& seenCards() const { return _seen; }

    /// How many times a specific rank has been seen.
    int countSeen(Rank r) const
    {
        int n = 0;
        for (const auto& c : _seen)
            if (c.rank == r) ++n;
        return n;
    }

    /// Hi-Lo running count: +1 for 2-6, 0 for 7-9, -1 for 10-A.
    /// A high positive count means many low cards are gone → rich in face cards.
    int hiLoCount() const
    {
        int count = 0;
        for (const auto& c : _seen)
        {
            int v = c.value();
            if      (v >= 2 && v <= 6)  ++count;
            else if (v >= 10)           --count;
        }
        return count;
    }

    /// Estimated probability that the next card will push a given score over 21.
    /// Useful hint for the AI state encoding.
    float bustProbability(int currentScore) const
    {
        if (currentScore >= 21) return 1.0f;
        int needed   = 21 - currentScore;   // max card value that avoids bust
        int safe     = 0;
        for (const auto& c : _cards)
            if (c.value() <= needed) ++safe;
        if (_cards.empty()) return 1.0f;
        return 1.0f - static_cast<float>(safe) / static_cast<float>(_cards.size());
    }

private:
    std::vector<Card> _cards;   // remaining cards
    std::vector<Card> _seen;    // dealt cards (card-counting history)
};
