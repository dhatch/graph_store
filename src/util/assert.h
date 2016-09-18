// Useful assert utilities.

#include <cstdlib>

#define DIE() std::exit(EXIT_FAILURE);

#define invariant(cond) if (!(cond)) { DIE(); }
