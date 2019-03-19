#ifndef KEYBOARD
#define KEYBOARD

#include <stdint.h>
#include <string>
#include <sstream>
#include <stdlib.h>

struct g_layoutKeyboard{
	size_t id;
	std::string layout;

	bool switchStatus;
	struct g_layoutKeyboard* NEXT;
};

class g_switchKeyboard {
private:
	static g_layoutKeyboard* layoutKeyboard;
	
	static void setStatus(bool logic);

public:
	static bool getStatus();	
	static void init();
	static void switchLayout();
};

#endif
