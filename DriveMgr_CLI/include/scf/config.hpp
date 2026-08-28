#pragma once

#define SCF_ALLOW_STL

// #define SCF_DONT_THROW

#ifdef SCF_ALLOW_STL
    #pragma message("SCF_ALLOW_STL is defined. STL features will be enabled in scf_str.hpp and scf_io.hpp")
#endif