#include "universe.hpp"

void Universe::add_object(std::shared_ptr<GameObject> obj)
{
    if(obj->active())
    {
        _dynObjs.push_back(std::move(obj));
    }else{
        _staticObjs.push_back(std::move(obj));
    }
}

void Universe::update(float dt)
{
    for(auto& obj : _dynObjs)
    {
        obj->_update(dt);
    }

    // check sights
    for(auto it = _dynObjs.begin(); it != _dynObjs.end(); ++it)
    {
        auto& obj = *it;
        auto sight = obj->sight();

        if (sight > 0.0f)
        {
            // check with static object
            for(auto& sobj : _staticObjs)
            {
                auto dist = glm::distance(sobj->position(), obj->position());
                _check_sight(obj, sobj, dist < sight);
            }
        }

        // check with other dynamic objects
        for(auto it2 = it; it2 != _dynObjs.end(); ++it2)
        {
            auto& obj2 = *it;
            auto sight2 = obj2->sight();
            auto dist = glm::distance(obj->position(), obj2->position());
            _check_sight(obj, obj2, dist < sight);

            if(sight2 > 0.0f) {
                _check_sight(obj2, obj, dist < sight2);
            }
        }
    }
}


void Universe::_check_sight(const obj_ptr& subj, const obj_ptr& to, bool insight)
{
    auto& sightSet = subj->_objInSight;
    auto id = to->id();
    auto it = sightSet.find(id);

    if(it == sightSet.end())
    {
        // was not in sight before
        if(insight)
        {
            // but is now
            sightSet.insert(id);
            subj->on_vision(to);
        }
        
    } else {
        // was in sight
        if(!insight)
        {
            // but not anymore
            sightSet.erase(it);
            subj->on_vision_lost(to);
        }
    }
}
