#pragma once
#include <cstdlib>
#include <ctime>

// ─────────────────────────────────────────────────────────────────────────────
//  Die
//  A single six-sided die.  Rolling it marks it as "used" until restored.
//  On a draw, all used dice are restored (see Combat).
// ─────────────────────────────────────────────────────────────────────────────
class Die
{
public:
    Die() : _value(1), _used(false) {}

    /// Roll the die and mark it as used.  Returns the rolled value (1-6).
    int roll()
    {
        _value = (std::rand() % 6) + 1;
        _used  = true;
        return _value;
    }

    int  value()    const { return _value; }
    bool isUsed()   const { return _used;  }

    void restore()        { _used = false; }

    /// Pre-set value without rolling (useful for testing / replays).
    void setValue(int v)  { _value = v; }

private:
    int  _value;
    bool _used;
};
