#include <iostream>
#include "keyboard.hpp"

int main() {
	g_switchKeyboard::init();
	if (g_switchKeyboard::getStatus()){
		std::cout << "Status True." << std::endl;
	} else {
		std::cout << "Status False." << std::endl;
	}
	return 0;
}