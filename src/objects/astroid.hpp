#pragma once


#include "game_object.hpp"


class Astroid : public GameObject
{
public:
    Astroid(Resource resource, unsigned int amount);

    virtual ScanResult interact_scan() override;
    virtual boost::optional<MineResult> interact_mine(unsigned int power) override;
private:
    const Resource _resource;
    unsigned int _amount;
};