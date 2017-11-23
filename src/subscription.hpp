#pragma once

#include <forward_list>
#include <memory>

namespace detail {

    struct subscription_base
    {
        virtual ~subscription_base();
    };

}

using subscription = std::shared_ptr<subscription_base>;

template<typename... Args>
class subscriptable
{
public:
    using subscription_func = std::function<void(const Args&...)>;
private:
    struct subscription_inst : subscription_base
    {
        subscription_inst(subsription_func&& func)
            : func(std::move(func))
        {
        }
        subscription_func func;
    };

public:    
    subscription subscribe(subscription_func func)
    {
        return std::make_shared<subscription_inst>(std::move(func));
    }

    void notify(const Args&... args)
    {
        auto last_it = _subscriptions.before_begin();
        auto it = _subscription.begin();

        while(it != _subscription.end())
        {
            if(it->unique())
            {
                ++it;
                _subscription.erase(last_it);
            } else {
                (*it)->func(args...);
                last_it = it;
                ++it;
            }
        }
    }

private:
    std::forward_list<std::shared_ptr<subscription_inst>> _subscriptions{};
};