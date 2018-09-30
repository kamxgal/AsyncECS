#pragma once

#include <memory>

namespace async_ecs
{
struct component;
using component_tag = unsigned int;
using component_const_ptr = std::shared_ptr<const component>;
using component_ptr = std::shared_ptr<component>;

struct component
{
    virtual component_ptr clone() const = 0;

protected:
    void clone_private_data(component_ptr c) const;
private:
    size_t mRevision = 0;

    friend struct entity;
};

template<class T>
component_tag Tag();
}  // namespace async_ecs
