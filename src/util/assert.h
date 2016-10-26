// Useful assert utilities.

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>

#define STRD(x) #x
#define STR(x) STRD(x)

#define DIE() std::exit(EXIT_FAILURE);

#define invariant(cond) if (!(cond)) { std::cerr << "Invariant failed. " STR(__LINE__) " " STR(__FILE__) << std::endl; DIE(); }
#define check_errno(err) if ((err) == -1) { std::cerr << "Error check failed." STR(__LINE__) " " STR(__FILE__) << std::endl; std::cerr << errno << " " << std::strerror(errno) << std::endl; DIE(); }
