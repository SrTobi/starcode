#include "spaceship.hpp"

#include "game.hpp"
#include "scripting/processor.hpp"
#include "scripting/binding.hpp"

#include "component/filesystem.hpp"

#include <chrono>

using v8::Isolate;
using v8::Local;
using v8::Value;
using v8::Context;
using LCtx = Local<Context>;

using namespace std::placeholders;




class ShipAi
{
public:
    ShipAi(Spaceship* ship)
        : _ship(*ship)
    {
        Game& game = Game::Current();
        _proc = game.processor_pool()->newProcessor(std::chrono::milliseconds(100), std::bind(&ShipAi::init_ctx, this, _1));
        //_proc->post(std::bind(&ShipAi::bootup, this, _1, _2, std::move(code)));
    }

    ~ShipAi()
    {
    }

private:
    LCtx init_ctx(Isolate* iso)
    {
        //FWrap(&Space::test)::NewTemplate(iso, this);
        return Context::New(iso);
    }
    
    void run_script(Isolate* iso, LCtx ctx, const std::string& code)
    {

    }


private:
    Spaceship& _ship;
    std::shared_ptr<Processor> _proc;
};



Spaceship::Spaceship(player_id player)
    : _player(player)
{
    _fs = std::make_unique<FileSystem>();
    _ai = std::make_unique<ShipAi>(this);
}

float Spaceship::sight() const        // in meter
{
    return 100.f;
}

float Spaceship::acceleration() const // in meter per second^2
{
    return 4.f;
}

float Spaceship::max_speed() const // in meter per second
{
    return 40.f;
}

ScanResult Spaceship::interact_scan()
{
    return {};
}

/*
    virtual void boot_script(std::string code) override
    {
        Game& game = Game::Current();
        _proc = game.processor_pool()->newProcessor(std::chrono::milliseconds(100), std::bind(&SpaceshipImpl::init_ctx, this, _1));
        _proc->post(std::bind(&SpaceshipImpl::run_script, this, _1, _2, std::move(code)));
    }

    std::string test(int i, const std::string& x) const {}

    LCtx init_ctx(Isolate* iso)
    {
        FWrap(&SpaceshipImpl::test)::NewTemplate(iso, this);
        return Context::New(iso);
    }
    
    void run_script(Isolate* iso, LCtx ctx, const std::string& code)
    {

    }
private:
    std::shared_ptr<Processor> _proc;
};


Spaceship::Spaceship(player_id player)
    : _player(player)
{
}

player_id Spaceship::player() const
{
    return _player;
}

std::shared_ptr<Spaceship> Spaceship::Create(player_id player)
{
    return Game::Current().make_object<SpaceshipImpl>(player);
}*/
