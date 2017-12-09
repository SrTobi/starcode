#include "game_object.hpp"

#include <cassert>
#include <iostream>
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


GameObject::GameObject(std::string name)
    : _id(Game::Current().next_obj_id())
    , _name(name + "[" + std::to_string(_id.value()) + "]")
{
}

const obj_id& GameObject::id() const
{
    return _id;
}

const std::string& GameObject::name() const
{
    return _name;
}

// controll
/*bool GameObject::activate(bool active)
{
    assert
}*/
const vec2& GameObject::set_position(const vec2& pos)
{
    return _position = pos;
}

const vec2& GameObject::set_velocity(const vec2& vel)
{
    return _velocity = vel;
}

void GameObject::set_target(const Target& target)
{
    _target = target;
}

// current status
bool GameObject::activate(bool active)
{
    return _active = active;
}

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

bool GameObject::interact_send_code(const std::string& path, const std::string& code)
{
    return false;
}

bool GameObject::interact_reboot()
{
    return false;
}


void GameObject::_update(float dt)
{
    if(has_target())
    {
        auto to = target()->position();
        auto path = to - _position;
        if (glm::length(path) <= 0.1) {
            if (glm::length(_velocity) <= 0.1) {
                _position = to;
                _velocity = vec2();
            } else {
                _velocity -= glm::normalize(_velocity) * std::min(glm::length(_velocity), acceleration()) * dt;
            }
        } else {
            auto dir = glm::normalize(path);
            auto optProj = glm::dot(dir, _velocity) * dir;
            auto pathToOpt = optProj - _velocity;
            auto pathLenToOpt = glm::length(pathToOpt) * dt;
            auto curAcc = acceleration() * dt;
            if (pathLenToOpt < curAcc) {
                auto rest = curAcc - pathLenToOpt;
                _velocity = optProj;
                auto speed = glm::length(_velocity);
                auto neededSecToTarget = speed > 0? glm::length(path) / speed : std::numeric_limits<float>::infinity();
                auto secsToStop = speed / acceleration();
                _velocity += dir * rest * (secsToStop < neededSecToTarget? 1.0f : -1.0f);

            } else {
                _velocity += glm::normalize(pathToOpt) * curAcc;
            }
        }
        auto speed = glm::length(_velocity);
        if(speed > max_speed())
            _velocity *= max_speed() / speed;
    } else {
        auto speed = glm::length(_velocity);
        auto dtAcc = dt * acceleration();
        if(dtAcc < speed)
        {
            _velocity *= 1.f - (dtAcc / speed);
        }else{
            _velocity = vec2();
        }
    }
    _position += dt * _velocity;
    if (_velocity != vec2() && rand() % 30 == 0)
        std::cout << name() << " at " << _position << std::endl;
}


