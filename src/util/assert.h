// Useful assert utilities.

#include <cstdlib>
#include <iostream>

#define STRD(x) #x
#define STR(x) STRD(x)

#define DIE() std::exit(EXIT_FAILURE);

#define invariant(cond) if (!(cond)) { std::cerr << "Invariant failed. " STR(__LINE__) " " STR(__FILE__) << std::endl; DIE(); }
