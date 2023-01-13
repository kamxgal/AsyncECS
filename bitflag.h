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

#include <cstdlib>
#include <assert.h>
#include <algorithm>
#include <memory>
#include <sstream>
#include <ostream>

struct bitflag_private;

struct bitflag
{
    bitflag(size_t size = 1);
    bitflag(const bitflag& other);
    bitflag(bitflag&& other);
    ~bitflag();

    bool at(size_t pos) const;
    void set(size_t pos, bool value);
    size_t size() const;
    void resize(size_t size);
    size_t enabled_flags_count() const;
    bool has(const bitflag& rhs) const;
    bitflag operator!() const;

    std::string str() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& o, const bitflag& rhs ) {
        for (size_t j=0; j<rhs.size(); ++j) {
            o << rhs.at(j);
        }
        return o;
    }

private:
    std::unique_ptr<bitflag_private> data;
};
