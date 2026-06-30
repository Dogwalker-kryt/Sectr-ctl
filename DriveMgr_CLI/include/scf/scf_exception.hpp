/* 
 * SCF exception - Stack C++ Framework exceptions
 * Copyright (C) 2026 Dogwalker-kryt
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

namespace scf {

class exception {
protected:
    const char* msg_;

public:
    explicit exception(const char* msg) : msg_(msg) {}
    virtual ~exception() = default;
    const char* what() const noexcept { return msg_; }
};

class length_error : public exception {
public:
    explicit length_error(const char* msg) : exception(msg) {}
};

class out_of_range : public exception {
public:
    explicit out_of_range(const char* msg) : exception(msg) {}
};

class invalid_argument : public exception {
public:
    explicit invalid_argument(const char* msg) : exception(msg) {}
};

class runtime_error : public exception {
public:
    explicit runtime_error(const char* msg) : exception(msg) {}
};


} // namespace scf