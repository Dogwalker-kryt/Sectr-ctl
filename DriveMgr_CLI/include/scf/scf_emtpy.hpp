#pragma once

namespace scf {
    struct ok_t {
        explicit constexpr ok_t() = default;
    };

    inline constexpr ok_t unit{};


    struct nothing_t {
        explicit constexpr nothing_t() = default;
    };

    inline constexpr nothing_t nothing{};
} 
