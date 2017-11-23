#pragma once

#include <boost/noncopyable.hpp>
#include <string>

class ResourceType: boost::noncopyable
{
public:
    ResourceType(const std::string& name);

    const std::string& name();

private:
    std::string _name;
};

using Resource = ResourceType*;