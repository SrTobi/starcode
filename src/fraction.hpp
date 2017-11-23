#pragma once

#include <boost/noncopyable.hpp>
#include <vector>
#include "player.hpp"

namespace detail {
    struct fraction_id_tag
    {
    };
}

class Fraction;

struct fraction_id : proto_id<detail::fraction_id_tag>
{
    explicit fraction_id(id_value_type id);
    Fraction& resolve() const;
};
MAKE_ID_HASH(fraction_id)

class Fraction: boost::noncopyable
{
public:
    Fraction(const fraction_id& id, const std::string& name);

    void add_player(Player& player);

    const fraction_id& id() const;
    const std::string& name() const;
    const std::vector<Player*> players() const;

private:
    const std::string _name;
    const fraction_id _id;

    std::vector<Player*> _members;
};