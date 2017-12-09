#include "spaceship.hpp"

#include "game.hpp"
#include "scripting/processor.hpp"
#include "scripting/binding.hpp"

#include "component/filesystem.hpp"

#include <chrono>
#include <iostream>

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
        _proc = game.processor_pool()->newProcessor(std::chrono::milliseconds(10), std::bind(&ShipAi::init_ctx, this, _1));
        _proc->post(std::bind(&ShipAi::bootup, this, _1, _2));
    }

    ~ShipAi()
    {
    }

private:
    LCtx init_ctx(Isolate* iso)
    {
        using namespace v8;
        using namespace bd;

        auto global = ObjectTemplate::New(iso);
        global->Set(str("position"), FWrap(&Spaceship::position)::NewTemplate(iso, &_ship));
        global->Set(str("flyTo"), FWrap(&ShipAi::set_target)::NewTemplate(iso, this));
        return Context::New(iso, nullptr, global);
    }

    void set_target(vec2 target)
    {
        _ship.set_target(target);
    }
    
    void bootup(Isolate* iso, LCtx ctx)
    {
        try {
            auto code = _ship._fs->read("", "/boot/boot-^");
            Local<v8::String> source = bd::str(code);
            v8::MaybeLocal<v8::Script> script = v8::Script::Compile(ctx, source);
            if (script.IsEmpty()) {
                std::cerr << "Failed to compile script!" << std::endl;
                return;
            }
            auto result = script.ToLocalChecked()->Run(ctx);
        } catch (std::runtime_error& e)
        {
            std::cerr << "failed to find boot script!" << std::endl;
        } catch (std::exception& e)
        {
            std::cerr << "failed to execute script: " << e.what() << std::endl;
        } catch (...)
        {
            std::cerr << "unknown error while executing script!" << std::endl;
        }
    }


private:
    Spaceship& _ship;
    std::shared_ptr<Processor> _proc;
};



Spaceship::Spaceship(player_id player)
    : GameObject("Spaceship(" + Game::Current().resolve_player(player).name() + ")")
    , _player(player)
{
    _fs = std::make_shared<FileSystem>();
    activate();
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
    return 10.f;
}

ScanResult Spaceship::interact_scan()
{
    return {};
}


bool Spaceship::interact_send_code(const std::string& path, const std::string& code)
{
    return _fs->write("", path, code) != nullptr;
}

bool Spaceship::interact_reboot()
{
    _ai = std::make_shared<ShipAi>(this);
    return true;
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
