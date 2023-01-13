/*
 * AsyncECS
 * Copyright (c) 2018 kamxgal Kamil Galant kamil.galant@gmail.com
 *
 * MIT licence
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#pragma once

#include <memory>

namespace ecs
{
struct component;
using component_tag = unsigned int;
using component_const_ptr = std::shared_ptr<const component>;
using component_ptr = std::shared_ptr<component>;

struct component
{
private:
    template<class T>
    struct RegisteredComponents {
        static bool is_registered;
        static component_tag tag;
    };

public:
    virtual ~component() = default;

    template<class T>
    static component_tag tag_t() {
        assert(RegisteredComponents<T>::is_registered);
        return RegisteredComponents<T>::tag;
    }

    template<class T>
    void register_t() {
        if (RegisteredComponents<T>::is_registered) {
            return;
        }
        RegisteredComponents<T>::is_registered = true;
        RegisteredComponents<T>::tag = ++mNextAvailableTag;
        RegisteredComponents<const T>::is_registered = true;
        RegisteredComponents<const T>::tag = mNextAvailableTag;
    }

    virtual component_tag tag() const = 0;

protected:
    void clone_private_data(component_ptr c) const;
private:
    static component_tag mNextAvailableTag;
    size_t mRevision = 0;
    friend struct entity;
};

template<class T> bool component::RegisteredComponents<T>::is_registered = false;
template<class T> component_tag component::RegisteredComponents<T>::tag = 0;

#define ECS_COMPONENT(NAME) \
NAME() { \
    component::register_t<NAME>(); \
} \
~NAME() override = default; \
ecs::component_tag tag() const override { \
    return component::tag_t<NAME>(); \
} \
public: \
NAME clone() const { \
    return *this; \
}

}  // namespace ecs
