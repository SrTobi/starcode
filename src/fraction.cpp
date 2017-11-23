#include "fraction.hpp"

#include <cassert>
#include <algorithm>
#include "game.hpp"

fraction_id::fraction_id(id_value_type id)
    : proto_id(id)
{
}

Fraction& fraction_id::resolve() const
{
    return Game::Current().resolve_fraction(*this);
}




Fraction::Fraction(const fraction_id& id, const std::string& name)
    : _id(id)
    , _name(name)
{
}

void Fraction::add_player(Player& player)
{
    assert(!std::count(_members.begin(), _members.end(), &player));
    _members.push_back(&player);
}

const fraction_id& Fraction::id() const
{
    return _id;
}

const std::string& Fraction::name() const
{
    return _name;
}

const std::vector<Player*> Fraction::players() const
{
    return _members;
}
