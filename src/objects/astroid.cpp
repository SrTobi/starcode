#include "astroid.hpp"



Astroid::Astroid(Resource resource, unsigned int amount)
    : _resource(resource)
    , _amount(amount)
{
}

ScanResult Astroid::interact_scan()
{
    return {
        _resource,
        _amount
    };
}

boost::optional<MineResult> Astroid::interact_mine(unsigned int power)
{
    auto amount = std::min(_amount, power);
    _amount -= amount;
    return MineResult { _resource, amount};
}