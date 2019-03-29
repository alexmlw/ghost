/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *  Ghost, a micro-kernel based operating system for the x86 architecture    *
 *  Copyright (C) 2015, Max Schlüssel <lokoxe@gmail.com>                     *
 *                                                                           *
 *  This program is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <ghost.h>
#include <ghostuser/io/files/file_utils.hpp>
#include <ghostuser/io/keyboard.hpp>
#include <ghostuser/io/ps2.hpp>
#include <ghostuser/io/ps2_driver_constants.hpp>
#include <ghostuser/utils/logger.hpp>
#include <ghostuser/utils/property_file_parser.hpp>
#include <ghostuser/utils/utils.hpp>
#include <string>
#include <map>
#include <fstream>
#include <stdio.h>

static bool statusCtrl = false;
static bool statusShift = false;
static bool statusAlt = false;

// TODO
static std::map<uint32_t, std::string> scancodeLayout;
static std::map<g_key_info, char> conversionLayout;

static std::string currentLayout;

static g_key_info last_unknown_key;
static bool have_last_unknown_key = false;

/**
 *
 */



std::string to_hex(int numb){
	char conv[5] = "0x";
    int i = 1;

    while(i < 3){
        switch(numb & 15){
            case 10: 
                conv[4 - i] = 'A';
                break;
            case 11: 
                conv[4 - i] = 'B';
                break;
            case 12: 
                conv[4 - i] = 'C';
                break;
            case 13: 
                conv[4 - i] = 'D';
                break;
            case 14: 
                conv[4 - i] = 'E';
                break;
            case 15: 
                conv[4 - i] = 'F';
                break;
            default:
                conv[4 - i] = 48 + (numb & 15);
                break;
        }
        i++;
        numb >>= 4;
    }

    conv[4] = '\0';
	
	return conv;
}





void g_keyboard::init() {
	std::ifstream conf("/system/keyboard/config.cfg");
	if (!conf.good()) {
		g_logger::log("Error load keyboard configure file \"config.cfg\".");

	} else {
		g_logger::log("Keyboard configure file load.");

	}



	g_switchKeyboard::setStatus(false);
	ptrKeyLayout->layout = "de-DE";
	conf.close();
	g_logger::log("Keyboard " + ptrKeyLayout->layout + "!");



	//g_switchKeyboard::couter = 0;
};

g_key_info g_keyboard::readKey(bool* break_condition) {

	if (!g_ps2_is_registered) {
		if (!g_ps2::registerSelf()) {
			return g_key_info();
		}
	}

	// wait until there are bytes in the buffer
	if (g_atomic_block_to(&g_ps2_area->keyboard.buffer_empty_lock, 10000) == false) {

		// keyboard pipe is non-blocking
		uint8_t scancode;
		if (g_read(g_ps2_keyboard_pipe, &scancode, 1) > 0) {

			// decrease buffer counter
			g_atomic_lock(&g_ps2_area->keyboard.buffer_amount_lock);
			g_ps2_area->keyboard.buffer_amount--;
			if (g_ps2_area->keyboard.buffer_amount == 0) {
				g_ps2_area->keyboard.buffer_empty_lock = true;
			}
			g_ps2_area->keyboard.buffer_amount_lock = false;

			// read and convert data
			g_key_info info;
			if (keyForScancode(scancode, &info)) {
				return info;
			}
		}
	}
	return g_key_info();
}

/**
 *
 */
bool g_keyboard::keyForScancode(uint8_t scancode, g_key_info* out) {

	// Get "pressed" info from scancode
	out->pressed = !(scancode & (1 << 7));
	out->scancode = scancode & ~(1 << 7); // remove 7th bit

	// out->pressed = scancode == 0xE0 ? 0 : 1;
	
	g_logger::log(to_hex(scancode) + " - scancode");
	
	// if (!out->pressed) {
	// 	return false;
	// }

	// out->scancode = scancode;
	
	// Get key from layout map
	bool found_compound = false;
	if (have_last_unknown_key) {
		int compoundScancode = last_unknown_key.scancode << 8 | out->scancode;

		// Try to find a compound key
		auto pos = scancodeLayout.find(compoundScancode);
		if (pos != scancodeLayout.end()) {

			out->key = pos->second;
			out->scancode = compoundScancode;
			found_compound = true;
			have_last_unknown_key = false;
		}
	}

	// When it is no compound
	if (!found_compound) {

		// Try to find the normal key
		auto pos = scancodeLayout.find(out->scancode);
		if (pos == scancodeLayout.end()) {

			// If it's not found, this might be the start of a compound
			have_last_unknown_key = true;
			last_unknown_key = *out;
			return false;

		} else {
			out->key = pos->second;
		}
	}

	// Handle special keys
	if (out->key == "KEY_CTRL_L" || out->key == "KEY_CTRL_R") {
		statusCtrl = out->pressed;

	} else if (out->key == "KEY_SHIFT_L" || out->key == "KEY_SHIFT_R") {
		statusShift = out->pressed;

	} else if (out->key == "KEY_ALT_L" || out->key == "KEY_ALT_R") {
		statusAlt = out->pressed;

	}

	// Set control key info
	out->ctrl = statusCtrl;
	out->shift = statusShift;
	out->alt = statusAlt;

	if(out->shift && out->alt){
		g_switchKeyboard::switchLayout();
	}
	
	return true;
}

/**
 *
 */
g_key_info g_keyboard::fullKeyInfo(g_key_info_basic basic) {

	// Get key from layout map
	g_key_info info;
	info.alt = basic.alt;
	info.ctrl = basic.ctrl;
	info.pressed = basic.pressed;
	info.scancode = basic.scancode;
	info.shift = basic.shift;

	auto pos = scancodeLayout.find(basic.scancode);
	if (pos != scancodeLayout.end()) {
		info.key = pos->second;
	}
	return info;
}

/**
 *
 */
char g_keyboard::charForKey(g_key_info info) {

	auto pos = conversionLayout.find(info);
	if (pos != conversionLayout.end()) {
		return pos->second;
	}

	return -1;
}

/**
 *
 */
bool g_keyboard::loadLayout(std::string iso) {
	if (loadScancodeLayout(iso) && loadConversionLayout(iso)) {
		return true;
	}
	return false;
}

/**
 *
 */
std::string g_keyboard::getCurrentLayout() {
	return ptrKeyLayout->layout;
}

/**
 *
 */
bool g_keyboard::loadScancodeLayout(std::string iso) {

	// Clear layout and parse file
	std::ifstream in("/system/keyboard/layout/" + iso + ".layout");
	if (!in.good()) {
		g_logger::log("layout false");
		return false;
	}
	g_property_file_parser props(in);

	scancodeLayout.clear();
	std::map<std::string, std::string> properties = props.getProperties();

	for (auto entry : properties) {

		uint32_t scancode = 0;

		auto spacepos = entry.first.find(" ");
		if (spacepos != std::string::npos) {
			std::string part1 = entry.first.substr(0, spacepos);
			std::string part2 = entry.first.substr(spacepos + 1);

			uint32_t part1val;
			std::stringstream conv1;
			conv1 << std::hex << part1;
			conv1 >> part1val;

			uint32_t part2val;
			std::stringstream conv2;
			conv2 << std::hex << part2;
			conv2 >> part2val;

			scancode = (part1val << 8) | part2val;

		} else {
			std::stringstream conv;
			conv << std::hex << entry.first;
			conv >> scancode;
		}

		if (entry.second.empty()) {
			std::stringstream msg;
			msg << "could not map scancode ";
			msg << (uint32_t) scancode;
			msg << ", key name '";
			msg << entry.second;
			msg << "' is not known";
			g_logger::log(msg.str());
		} else {
			scancodeLayout[scancode] = entry.second;
		}
	}

	return true;
}

/**
 *
 */
bool g_keyboard::loadConversionLayout(std::string iso) {

	// Read layout file
	std::stringstream conversionLayoutStream;

	// Clear layout and parse file
	std::ifstream in("/system/keyboard/conversion/" + iso + ".conversion");
	if (!in.good()) {
		g_logger::log("conversion false");
		return false;
	}
	g_property_file_parser props(in);

	// empty existing conversion layout
	conversionLayout.clear();

	std::map<std::string, std::string> properties = props.getProperties();
	for (auto entry : properties) {

		// create key info value
		g_key_info info;

		// they shall be triggered on press
		info.pressed = true;

		// take the key and split if necessary
		std::string keyName = entry.first;
		int spacePos = keyName.find(' ');

		if (spacePos != -1) {
			std::string flags = keyName.substr(spacePos + 1);
			keyName = keyName.substr(0, spacePos);

			// Handle the flags
			for (int i = 0; i < flags.length(); i++) {

				if (flags[i] == 's') {
					info.shift = true;

				} else if (flags[i] == 'c') {
					info.ctrl = true;

				} else if (flags[i] == 'a') {
					info.alt = true;

				} else {
					std::stringstream msg;
					msg << "unknown flag in conversion mapping: ";
					msg << flags[i];
					g_logger::log(msg.str());
				}
			}
		}

		// Set key
		info.key = keyName;

		// Push the mapping
		char c = -1;
		std::string value = entry.second;
		if (value.length() > 0) {
			c = value[0];

			// Escaped numeric values
			if (c == '\\') {
				if (value.length() > 1) {
					std::stringstream conv;
					conv << value.substr(1);
					uint32_t num;
					conv >> num;
					c = num;

				} else {
					g_logger::log("skipping value '" + value + "' in key " + keyName + ", illegal format");
					continue;
				}
			}
		}
		conversionLayout[info] = c;
	}

	return true;
}

bool g_switchKeyboard::getStatus() {
	return ptrKeyLayout->switchStatus;
}

void g_switchKeyboard::setStatus(bool logic) {
	ptrKeyLayout->switchStatus = logic;
}

void g_switchKeyboard::switchLayout() {

	if (g_switchKeyboard::getStatus()) {
		ptrKeyLayout->layout = "de-DE";
	} else {
		ptrKeyLayout->layout = "en-US";
	}

	if (g_switchKeyboard::getStatus()) {
		g_switchKeyboard::setStatus(false);
	} else {
		g_switchKeyboard::setStatus(true);
	}

	if (g_keyboard::loadLayout(ptrKeyLayout->layout)) {
		g_logger::log("keyboard layout '" + ptrKeyLayout->layout + "' loaded");
	} else {
		g_logger::log("unable to load keyboard layout '" + ptrKeyLayout->layout + "'");
	}
}