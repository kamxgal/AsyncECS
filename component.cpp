#include "component.h"

void async_ecs::component::clone_private_data(async_ecs::component_ptr c) const
{
    c->mRevision = mRevision;
}
