#include "component.h"

namespace ecs
{
component_tag component::mNextAvailableTag = 0;

void component::clone_private_data(ecs::component_ptr c) const
{
	c->mRevision = mRevision;
}
} // namespace ecs
