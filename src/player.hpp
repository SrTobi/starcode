#pragma once

#include <string>
#include <boost/noncopyable.hpp>
#include "defs.hpp"
#include "id.hpp"

namespace detail {
    struct player_id_tag
    {
    };
}

class Player;
class Fraction;

struct player_id : proto_id<detail::player_id_tag>
{
    explicit player_id(id_value_type id);
    Player& resolve() const;
};

MAKE_ID_HASH(player_id);

class Player: boost::noncopyable
{
public:
    Player(const player_id& id, const std::string& name, Fraction& fraction);

    const player_id& id() const;
    const std::string& name() const;
    Fraction& fraction();
private:
    const player_id _id;
    const std::string _name;
    Fraction& _fraction;
};