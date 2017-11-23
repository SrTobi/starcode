#pragma once

#include <unordered_set>
#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>
#include "id.hpp"
#include "defs.hpp"
#include "resource_type.hpp"

namespace detail {
    struct object_id_tag
    {
    };
}

class GameObject;
class Universe;

using obj_ptr = std::shared_ptr<GameObject>;

struct obj_id : proto_id<detail::object_id_tag>
{
    explicit obj_id(id_value_type id);
    obj_ptr resolve() const;
};

MAKE_ID_HASH(obj_id);


class Target
{
public:
    Target(const vec2& target);
    Target(const obj_ptr& target, const vec2& offset = vec2());

    vec2 position() const;

private:
    std::weak_ptr<GameObject> _target;
    vec2 _offset;
    mutable vec2 _position;
};

class GameObjectInfo: boost::noncopyable
{
public:
    virtual const std::string& name() const = 0;
};

class GameObjectType
{
    GameObjectType(GameObjectInfo* info);

    const std::string& name() const;
private:
    GameObjectInfo* _info;
};

struct ScanResult
{
//    boost::optional<GameObjectType> obj_type;
    boost::optional<Resource> resource;
    boost::optional<unsigned int> resource_amount;
};

struct MineResult
{
    Resource resource;
    unsigned int amount;
};


class GameObject: boost::noncopyable
{
    friend class Universe;
public:
    GameObject();
    virtual ~GameObject() = default;

    const obj_id& id() const;

    // controll
    bool activate(bool active = true);
    const vec2& position(const vec2& pos);
    const vec2& velocity(const vec2& vel);
    void set_target(const Target& target);

    // current status
    const vec2& position() const;
    virtual bool active() const;
    const vec2& velocity();
    bool has_target() const;
    boost::optional<Target> target() const;

    // physic properties
    virtual float sight() const;        // in meter
    virtual float acceleration() const; // in meter per second^2
    virtual float max_speed() const; // in meter per second

    // events
    virtual void on_update(double dt);
    virtual void on_collision(const vec2& where, const obj_ptr& with);
    virtual void on_vision(const obj_ptr& who);
    virtual void on_vision_lost(const obj_ptr& who);

    // possible interactions
    virtual ScanResult interact_scan() = 0;
    virtual boost::optional<MineResult> interact_mine(unsigned int power);
private:
    void _update(float dt);

protected:
    const obj_id _id;
    vec2 _position{};       // in meter
    vec2 _velocity{};       // in meter per second
    boost::optional<Target> _target{};
    std::unordered_set<obj_id> _objInSight{};
    bool _active = false;
};
