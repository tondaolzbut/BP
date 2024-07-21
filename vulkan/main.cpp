#include "aplikace.h"

#include<cstdlib>
#include<iostream>
#include<stdexcept>

int main() {
	ep::RenderApp aplikace{};
	aplikace.run(); //melo by se hodit do try bloku

	return EXIT_SUCCESS;
}