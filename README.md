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
We can select more than one component at a time:
```
ecs::view<Component1, Component2, Component3> myView = database.select<Component1, Component2, Component3>();
// or more convenient syntax:
auto myView2 = database.select<Component1, Component2, Component3>();
```
Each entity is checked if it has all the components listed in the select method. If so then we will have access to all of them from the view's interface. Creating a view has linear complexity in the size of entities and constant in the size of components.
