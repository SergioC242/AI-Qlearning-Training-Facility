#pragma once
#include "Card.h"
#include "Deck.h"
#include <vector>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
//  Enemy
//  Fixed-threshold opponent: draws until score > 15, then stands.
//  Does NOT use dice. Can bust past 21 (bust score = 0 for damage).
// ─────────────────────────────────────────────────────────────────────────────
class Enemy
{
public:
	explicit Enemy(int maxHp, const std::string& name = "Enemy")
		: _maxHp(maxHp), _hp(maxHp), _name(name) {
	}

	void clearHand() { _hand.clear(); }

	const std::vector<Card>& hand()  const { return _hand; }
	int  score()                     const { return handScore(_hand); }
	bool busted()                    const { return score() > 21; }

	int  hp()          const { return _hp; }
	int  maxHp()       const { return _maxHp; }
	bool isDefeated()  const { return _hp <= 0; }

	void takeDamage(int dmg)
	{
		_hp -= dmg;
		if (_hp < 0) _hp = 0;
	}

	const std::string& name() const { return _name; }

	/// Draws cards until score > 15, then stands.
	void playTurn(Deck& deck, bool verbose = false)
	{
		if (verbose)
			std::cout << "\n[" << _name << "] draws until > 15\n";

		while (score() <= 15 && !deck.empty())
		{
			Card c = deck.draw();
			_hand.push_back(c);
			if (verbose)
				std::cout << "  [" << _name << "] draws " << c.name()
				<< "  -> " << score() << "\n";
		}

		if (verbose)
		{
			if (busted())
				std::cout << "  [" << _name << "] BUSTS (" << score() << ")\n";
			else
				std::cout << "  [" << _name << "] STANDS at " << score() << "\n";
		}
	}

private:
	int               _maxHp;
	int               _hp;
	std::string       _name;
	std::vector<Card> _hand;
};
