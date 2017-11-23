#include "resource_type.hpp"


ResourceType::ResourceType(const std::string& name)
    : _name(name)
{

}

const std::string& ResourceType::name()
{
    return _name;
}