#include "component.h"

namespace async_ecs
{
component_tag component::mNextAvailableTag = 0;

void component::clone_private_data(async_ecs::component_ptr c) const
{
	c->mRevision = mRevision;
}
} // namespace async_ecs