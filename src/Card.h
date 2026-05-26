#pragma once
#include <string>
#include <array>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  Card
//  Standard 52-card deck.  Ace = 11 (soft), can be reduced to 1 if bust.
// ─────────────────────────────────────────────────────────────────────────────
enum class Suit { CLUBS, DIAMONDS, HEARTS, SPADES };
enum class Rank {
	TWO = 2, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN,
	JACK, QUEEN, KING, ACE
};

struct Card
{
	Suit suit;
	Rank rank;

	/// Blackjack value (face cards = 10, Ace = 11).
	int value() const
	{
		int r = static_cast<int>(rank);
		if (r >= 10 && r <= 13) return 10;   // J, Q, K
		if (rank == Rank::ACE)  return 11;
		return r;
	}

	std::string name() const
	{
		static const std::array<std::string, 13> rankNames = {
			"2","3","4","5","6","7","8","9","10","J","Q","K","A"
		};
		static const std::array<std::string, 4> suitNames = {
			"♣","♦","♥","♠"
		};
		int ri = static_cast<int>(rank) - 2;
		int si = static_cast<int>(suit);
		return rankNames[ri] + suitNames[si];
	}
};

/// Compute the best blackjack score for a hand (aces reduced to 1 if needed).
inline int handScore(const std::vector<Card>& hand)
{
	int total = 0;
	int aces = 0;
	for (const auto& c : hand)
	{
		total += c.value();
		if (c.rank == Rank::ACE) ++aces;
	}
	while (total > 21 && aces > 0)
	{
		total -= 10;
		--aces;
	}
	return total;
}
