#pragma once

#include "game_object.hpp"
#include "player.hpp"


class ShipAi;
class FileSystem;


class Spaceship : public GameObject
{
    friend class ShipAi;
public:
    Spaceship(player_id player);

    virtual void boot_script(std::string code) = 0;

    player_id player() const;


    virtual float sight() const override;        // in meter
    virtual float acceleration() const override; // in meter per second^2
    virtual float max_speed() const override; // in meter per second
    virtual ScanResult interact_scan() override;

public:
    static std::shared_ptr<Spaceship> Create(player_id player);
protected:
    player_id _player;
    std::unique_ptr<ShipAi> _ai;
    std::unique_ptr<FileSystem> _fs;
};