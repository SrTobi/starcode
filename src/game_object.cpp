#include "game_object.hpp"

#include <cassert>
#include "game.hpp"

obj_id::obj_id(id_value_type id)
    : proto_id(id)
{
}

obj_ptr obj_id::resolve() const
{
    return Game::Current().resolve_object(*this);
}



Target::Target(const vec2& target)
    : _position(target)
{
}

Target::Target(const obj_ptr& target, const vec2& offset)
    : _target(target)
    , _offset(offset)
    , _position(target->position() + offset)
{
    assert(target);
}

vec2 Target::position() const
{
    auto target = _target.lock();

    if(target)
    {
        _position = target->position() + _offset;
    }

    return _position;
}


GameObject::GameObject()
    : _id(Game::Current().next_obj_id())
{
}

const obj_id& GameObject::id() const
{
    return _id;
}

// controll
/*bool GameObject::activate(bool active)
{
    assert
}*/
const vec2& GameObject::position(const vec2& pos)
{
    return _position = pos;
}

const vec2& GameObject::velocity(const vec2& vel)
{
    return _velocity = vel;
}

void GameObject::set_target(const Target& target)
{
    _target = target;
}

// current status
const vec2& GameObject::position() const
{
    return _position;
}

bool GameObject::active() const
{
    return _active;
}

const vec2& GameObject::velocity()
{
    return _velocity;
}

bool GameObject::has_target() const
{
    return bool{_target};
}
boost::optional<Target> GameObject::target() const
{
    return _target;
}

// physic properties
float GameObject::sight() const
{
    return 0.0f; // no sight
}
float GameObject::acceleration() const
{
    return 2.0f; // little acceleration to make all objects stop eventually
}

float GameObject::max_speed() const // in meter per second
{
    return 2.0f;
}

// events
void GameObject::on_update(double dt) {}
void GameObject::on_collision(const vec2& where, const obj_ptr& with) {}
void GameObject::on_vision(const obj_ptr& who) {}
void GameObject::on_vision_lost(const obj_ptr& who) {}

// possible interactions
boost::optional<MineResult> GameObject::interact_mine(unsigned int power)
{
    return boost::none;
}



void GameObject::_update(float dt)
{
    if(has_target())
    {
        auto to = target()->position();
        auto path = (to - _position);
        auto dir = glm::normalize(path);
        _velocity = std::min(glm::dot(_velocity, dir), 0.f) * dir;

        auto speed = glm::length(_velocity);
        if(speed > max_speed())
            _velocity *= max_speed() / speed;
    } else {
        auto speed = glm::length(_velocity);
        auto dtAcc = dt * acceleration();
        if(dtAcc > speed)
        {
            _velocity *= 1.f - (dtAcc / speed);
        }else{
            _velocity = vec2();
        }
    }
    _position = dt * _velocity;
}


