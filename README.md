# AsyncECS
database based on entity-component with thread-safe access for asynchronous systems 

# Basic usage
In order to start using the library a user has to create a top-level database object of type called `registry`.
```
#include <registry.h>
// ...
ecs::registry database;
```
The registry class supports four most common operations named just like in SQL:
* insert
* update
* remove
* select

## Creating component
Each component has to derive from ecs::component and include macro `ECS_COMPONENT` as in example below:
```
struct MyComponent : public ecs::component
{
    ECS_COMPONENT(MyComponent);
    
    std::string mProperty1;
    // ...
};
```
## Using components
Components are added to entities. Entities can be created in the following way:
```
ecs::registry database;
ecs::entity_t entityId = database.createEntity();
```
Now, as we have id of a newly created entity we can add a component into it:
```
MyComponent myComponent;
myComponent.mProperty1 = "Hello, World!";

database.insert(entityId, std::move(myComponent));
```
From now on access to the component is possible only via shared pointer to const object:
```
std::shared_ptr<const MyComponent> myComponent = database.select<MyComponent>(entityId);
```
> Using `auto` is strongly recommended in order to shorten the code:
> ```
> auto myComponent = database.select<MyComponent>(entityId);
> ```
Finding a proper component has logarithmic complexity in the size of entities and constant in the size of components per entity.
How to update components? A regular approach forbids to modify a registered component directly in database. At first we have to get component from the database, then create a copy of it, then modify its content and finally update it:
```
auto myComponent = database.select<MyComponent>(entityId);
MyComponent updated = myComponent->clone();
updated.mProperty1 = "Hello, Updated World!";
database.update(entityId, updated);
```
Components are versioned (they have their revisions defined). If two asynchronous systems will try to write to the same component at the same time then only the first one will succeed. Success or failure of an operation is indicated by the update method itself by returning a boolean value.
Components can be removed from an entity in a similar way as in previous examples:
```
bool result = database.remove<MyComponent>(entityId);
```

## Selecting more than one component at a time
We can select more than one component at a time by creating view:
```
ecs::view<Component1, Component2, Component3> myView = database.select<Component1, Component2, Component3>();
// or more convenient syntax:
auto myView2 = database.select<Component1, Component2, Component3>();
```
Each entity is checked if it has all the components listed in the select method. If so then we will have access to all of them from the view's interface. Creating a view has linear complexity in the size of entities and constant in the size of components.

### Selecting single component from a view
View provides access to information which entities it is related to. Based on that you can select chosen components from the view and access them via shared pointer to const struct. The complexity of such getter is constant.
```
std::vector<entity_id> ids = myView.entities();
for (entity_id id : ids)
{
    auto component1 = myView.select<MyComponent1>(id);
    auto component2 = myView.select<MyComponent2>(id);
    // ...
}
```
### Selecting components from multiple entities
Once view is created user can perform additional selection of components of certain type with lambda expression which would check values of properties of a component. Example:
```
std::map<entity_id, std::shared_ptr<const MyComponent>> = myView.select<MyComponent>([](auto ptr) -> bool {
    return ptr->mProperty1 == "Hello, World!";
});
```
Complexity is linear in the size of entities, constant in the size of components stored by a view.

# Subscribing for changes in registry
User is able to subscribe for changes in registry. Operations that are notified are:
* insert
* update
* remove

In order to subscribe a user has to provide callback for certain notification. Callback is called on a thread of an operation's caller.
```
registry.subscribe<MyComponent>([](const Notification<MyComponent>& notif) {
    // do something
});
```

Notification's properties are:
* operation - type of operation
* entityId - id of parent entity
* component - shared pointer to related component (nullptr for "removed" operation)

It is also possible to provide precondition function to check certain properties of a notification or updated component. For example user is able to subscribe only to update operations of component of MyComponent type:
```
registry.subscribe<MyComponent>([](const Notification<MyComponent>& notif) {
    // do something
}, [](const Notification<MyComponent>& notif) -> bool {
    return notif.operation == operation_t::updated;
});
```
## Unsubscribing
The `registry.subscribe` returns a function to unsubscribe. Call it do deactivate subscription.
```
auto unsubscriber = registry.subscribe<MyComponent>([](const Notification<MyComponent>& notif) {
    // do something
});

// ...
// getting notifications
// ...

unsubscriber();
```
