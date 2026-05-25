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
//  Cards are drawn by random sampling WITHOUT removal — the deck always
//  contains all 52 cards.  remaining() always returns 52; empty() is
//  always false.
//  Supports card counting: seenCards() returns every card dealt so far.
// ─────────────────────────────────────────────────────────────────────────────
class Deck
{
public:
    Deck()
    {
        reset();
    }

    /// Rebuild the full 52-card deck.  _seen history is cleared.
    /// shuffle() is kept for API compatibility but is a no-op since draw()
    /// already samples randomly on every call.
    void reset()
    {
        _cards.clear();
        _seen.clear();

        for (int s = 0; s < 4; ++s)
            for (int r = 2; r <= 14; ++r)
                _cards.push_back({ static_cast<Suit>(s),
                                   static_cast<Rank>(r) });
    }

    void shuffle()
    {
        // No-op: draw() samples randomly, so a pre-shuffle is unnecessary.
        // Kept so existing call sites continue to compile.
    }

    /// Draw a random card WITHOUT removing it from the deck.
    /// The deck always retains all 52 cards; remaining() never decreases.
    Card draw()
    {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> dist(0, static_cast<int>(_cards.size()) - 1);
        Card c = _cards[dist(rng)];
        _seen.push_back(c);
        return c;
    }

    int  remaining()  const { return static_cast<int>(_cards.size()); }  // always 52
    bool empty()      const { return false; }  // deck never empties

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
    /// Because the full deck is always present, this reflects the true
    /// probability across all 52 cards at every point in time.
    float bustProbability(int currentScore) const
    {
        if (currentScore >= 21) return 1.0f;
        int needed = 21 - currentScore;
        int safe   = 0;
        for (const auto& c : _cards)
            if (c.value() <= needed) ++safe;
        return 1.0f - static_cast<float>(safe) / static_cast<float>(_cards.size());
    }

private:
    std::vector<Card> _cards;   // full 52-card deck — never modified after reset()
    std::vector<Card> _seen;    // dealt cards (card-counting history)
};
