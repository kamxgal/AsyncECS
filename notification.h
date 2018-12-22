#pragma once

namespace async_ecs
{

using entity_id = size_t;

enum class operation_t
{
	inserted,
	updated,
	removed
};

template <class T>
struct Notification
{
	using data_type = T;

	operation_t operation;
	entity_id entityId;
	std::shared_ptr<const T> component; // nullptr for "removed" operation
};
} // namespace async_ecs