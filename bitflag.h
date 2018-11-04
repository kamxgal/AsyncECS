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
    bool operator&(const bitflag& rhs) const;
	bitflag operator!() const;

	std::string str() const {
		std::stringstream ss;
		ss << *this;
		return ss.str();
	}

    friend std::ostream& operator<<(std::ostream& o, const bitflag& rhs ) {
        for (int j=0; j<rhs.size(); ++j) {
            o << rhs.at(j);
        }
        return o;
    }

private:
    std::unique_ptr<bitflag_private> data;
};
