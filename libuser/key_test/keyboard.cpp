#include <iostream>
#include <fstream>

#include "keyboard.hpp"

void g_switchKeyboard::init() {
	layoutKeyboard = new g_layoutKeyboard;

	std::ifstream conf("/system/keyboard/config.cfg");
	if (!conf.good()) {
		//g_logger::log("Error load keyboard configure file \"config.cfg\".");
		std::cout << "Error load keyboard configure file." << std::endl;

	} else {
		//g_logger::log("Keyboard configure file load.");
		std::cout << "Keyboard configure file load." << std::endl;

	}
	layoutKeyboard->layout = "std";
	std::cout << layoutKeyboard->layout;
	setStatus(false);
	conf.close();
};

bool g_switchKeyboard::getStatus() {
	//return layoutKeyboard->switchStatus;
}

void g_switchKeyboard::setStatus(bool logic) {
	//layoutKeyboard->switchStatus = logic;
}

void g_switchKeyboard::switchLayout() {

	//if (g_keyboard::loadLayout(layoutKeyboard.layout)) {
	//	g_logger::log("keyboard layout '" + layoutKeyboard.layout + "' loaded");
	// } else {
		// g_logger::log("unable to load keyboard layout '" + layoutKeyboard.layout + "'");
	// }

}