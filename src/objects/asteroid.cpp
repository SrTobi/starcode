#include "asteroid.hpp"



Asteroid::Asteroid(Resource resource, unsigned int amount)
    : GameObject("Asteroid{" + resource->name() + ":" + std::to_string(amount) + "}")
    , _resource(resource)
    , _amount(amount)
{
}

ScanResult Asteroid::interact_scan()
{
    return {
        _resource,
        _amount
    };
}

boost::optional<MineResult> Asteroid::interact_mine(unsigned int power)
{
    auto amount = std::min(_amount, power);
    _amount -= amount;
    return MineResult { _resource, amount};
}