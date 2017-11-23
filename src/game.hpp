#pragma once

#include <boost/noncopyable.hpp>
#include <unordered_map>
#include "defs.hpp"
#include "player.hpp"
#include "fraction.hpp"
#include "game_object.hpp"
#include "universe.hpp"

struct GameConfig
{
    std::vector<std::string> players;
};

class V8ProcessorPool;

class Game: boost::noncopyable
{
public:
    ~Game();

    static Game& InitializeGame(const GameConfig& config);
    static Game& Current();
    static void Shutdown();

    Player& get_gaia();
    Player& resolve_player(const player_id& id);
    Fraction& resolve_fraction(const fraction_id& id);
    obj_ptr resolve_object(const obj_id& id);

    template<typename T, typename... Args>
    std::shared_ptr<T> make_object(Args&&... args)
    {
        auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
        register_object(ptr);
        return ptr;
    }

    void register_object(const obj_ptr& obj);

    Universe& universe();
    const std::shared_ptr<V8ProcessorPool>& processor_pool() const;
    obj_id next_obj_id();
private:
    Game(const GameConfig& config);
    
    id_value_type _next_id();
private:
    Universe _universe{};
    ResourceType _ore_type {"ore"};
    std::unordered_map<player_id, Player> _players{};
    std::unordered_map<fraction_id, Fraction> _fractions{};
    std::unordered_map<obj_id, obj_ptr> _objects{};
    id_value_type _nextId = 0;
    std::shared_ptr<V8ProcessorPool> _ppool;
};