#include "utils.hpp"

# include <iostream>
# include <cstdlib>

void exit_with_error(const std::string &msg)
{
	std::cerr << "Error: " << msg << std::endl;
	std::exit(1);
}
