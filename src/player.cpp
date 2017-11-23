#include "player.hpp"

#include <cassert>
#include <algorithm>
#include "fraction.hpp"
#include "game.hpp"

player_id::player_id(id_value_type id)
    : proto_id(id)
{
}

Player& player_id::resolve() const
{
    return Game::Current().resolve_player(*this);
}




Player::Player(const player_id& id, const std::string& name, Fraction& fraction)
    : _id(id)
    , _name(name)
    , _fraction(fraction)
{
}


const player_id& Player::id() const
{
    return _id;
}

const std::string& Player::name() const
{
    return _name;
}

Fraction& Player::fraction()
{
    return _fraction;
}
