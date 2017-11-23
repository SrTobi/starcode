#pragma once

#include "defs.hpp"

using id_value_type = unsigned int;

template<typename Tag>
class proto_id
{
public:
    explicit proto_id(id_value_type value)
        : _value(value)
    {
    }

    id_value_type value() const
    {
        return _value;
    }

    bool operator ==(const proto_id& other) const
    {
        return value() == other.value();
    }

    bool operator !=(const proto_id& other) const
    {
        return value() != other.value();
    }
private:
    const id_value_type _value;
};

#define MAKE_ID_HASH(clazz) \
        namespace std \
        { \
            template<> struct hash<clazz> \
            { \
                std::size_t operator()(clazz const& s) const \
                { \
                    return std::hash<id_value_type>{}(s.value()); \
                } \
            }; \
        } \

