#include "game.hpp"

#include "scripting/processor.hpp"
#include "objects/astroid.hpp"


#include <cassert>
#include <tuple>

namespace {
    Game* _CurrentGame = nullptr;
}


Game::Game(const GameConfig& config)
{
    assert(!_CurrentGame);
    _CurrentGame = this;

    _ppool = V8ProcessorPool::Create(1);

    auto make_player = [this](id_value_type id, const std::string& name) -> Player*
    {
        auto fid = fraction_id{id};
        auto& f = _fractions.emplace(std::piecewise_construct, std::make_tuple(fid), std::make_tuple(fid, name + "-fraction")).first->second;
        auto pid = player_id{id};
        auto& p = _players.emplace(std::piecewise_construct, std::make_tuple(pid), std::make_tuple(pid, name, std::ref(f))).first->second;
        f.add_player(p);
        return &p;
    };

    // make gaia
    make_player(0, "gaia");
    _nextId = 1;

    // make players
    for(const auto& name : config.players)
    {
        make_player(_next_id(), name);
    }

    // make map
    make_object<Astroid>(&_ore_type, 10000);
}

Game::~Game()
{
    assert(_CurrentGame == this);
    _CurrentGame = nullptr;
}



Player& Game::resolve_player(const player_id& id)
{
    assert(_players.count(id));
    return _players.at(id);
}

Fraction& Game::resolve_fraction(const fraction_id& id)
{
    assert(_fractions.count(id));
    return _fractions.at(id);
}

obj_ptr Game::resolve_object(const obj_id& id)
{
    assert(_objects.count(id));
    return _objects.at(id);
}

Universe& Game::universe()
{
    return _universe;
}


const std::shared_ptr<V8ProcessorPool>& Game::processor_pool() const
{
    return _ppool;
}

obj_id Game::next_obj_id()
{
    return obj_id{_next_id()};
}

void Game::register_object(const obj_ptr& obj)
{
    _objects.emplace(obj->id(), obj);
    _universe.add_object(obj);
}


id_value_type Game::_next_id()
{
    return _nextId++;
}



Game& Game::InitializeGame(const GameConfig& config)
{
    _CurrentGame = new Game(config);
    return Current();
}

Game& Game::Current()
{
    assert(_CurrentGame);
    return *_CurrentGame;
}

void Game::Shutdown()
{
    assert(_CurrentGame);
    delete _CurrentGame;
}
