#pragma once
#include <cstdlib>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
//  DieType
//  Add new die types here. Each value maps to a roll range defined in
//  Die::roll(). The order must match the Combat._dice[] array initialiser.
// ─────────────────────────────────────────────────────────────────────────────
enum class DieType
{
	D1_10,    // uniform [1, 10]
	D10_20,   // uniform [10, 20]
	D1_6,     // uniform [1, 6]
	D1_4,     // uniform [1, 4]
	D15_20,   // uniform [15, 20]
	D1_OR_11, // 50/50: either 1 or 11

	COUNT     // keep last — used to size arrays
};

inline std::string dieTypeName(DieType t)
{
	switch (t)
	{
	case DieType::D1_10:    return "d[1-10]";
	case DieType::D10_20:   return "d[10-20]";
	case DieType::D1_6:     return "d[1-6]";
	case DieType::D1_4:     return "d[1-4]";
	case DieType::D15_20:   return "d[15-20]";
	case DieType::D1_OR_11: return "d[1|11]";
	default:                return "d[?]";
	}
}

// ─────────────────────────────────────────────────────────────────────────────
//  Die
//  Rolls according to its DieType. Marked "used" after rolling; restored
//  on a draw (see Combat).
//
//  To add a new die:
//    1. Add a value to DieType above.
//    2. Add its roll logic in Die::roll() below.
//    3. Add it to the Combat._dice initialiser in Combat.h.
//    4. Add a USE_DIE_N action in AIPlayer.h and AIPlayer::allActions().
// ─────────────────────────────────────────────────────────────────────────────
class Die
{
public:
	explicit Die(DieType type = DieType::D1_6)
		: _type(type), _value(1), _used(false) {
	}

	/// Roll the die according to its type, mark it used, return the value.
	int roll()
	{
		switch (_type)
		{
		case DieType::D1_10:
			_value = (std::rand() % 10) + 1;          // [1, 10]
			break;
		case DieType::D10_20:
			_value = (std::rand() % 11) + 10;         // [10, 20]
			break;
		case DieType::D1_6:
			_value = (std::rand() % 6) + 1;           // [1, 6]
			break;
		case DieType::D1_4:
			_value = (std::rand() % 4) + 1;           // [1, 4]
			break;
		case DieType::D15_20:
			_value = (std::rand() % 6) + 15;          // [15, 20]
			break;
		case DieType::D1_OR_11:
			_value = (std::rand() % 2 == 0) ? 1 : 11; // 1 or 11
			break;
		default:
			_value = 1;
			break;
		}
		_used = true;
		return _value;
	}

	DieType     type()    const { return _type; }
	int         value()   const { return _value; }
	bool        isUsed()  const { return _used; }
	std::string name()    const { return dieTypeName(_type); }

	void restore() { _used = false; }

private:
	DieType _type;
	int     _value;
	bool    _used;
};

// Convenience: total number of dice = DieType::COUNT
static constexpr int NUM_DICE = static_cast<int>(DieType::COUNT);
