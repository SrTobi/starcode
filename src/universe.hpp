#pragma once

#include <boost/noncopyable.hpp>
#include <list>
#include "game_object.hpp"

class Universe: boost::noncopyable
{
public:
    void add_object(std::shared_ptr<GameObject> obj);

    void update(float dt);

private:
    void _check_sight(const obj_ptr& from, const obj_ptr& to, bool insight);

private:
    std::list<obj_ptr> _dynObjs{};
    std::list<obj_ptr> _staticObjs{};
};
