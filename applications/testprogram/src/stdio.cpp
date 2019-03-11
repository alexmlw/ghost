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

#include "tester.hpp"
#include <ghost.h>
#include <stdlib.h>

void openReadClose()
{
	g_fd fd = g_open("/system/lib/crti.o");
	ASSERT(fd != -1);

	int buflen = 128;
	uint8_t* buffer = (uint8_t*) malloc(buflen);
	ASSERT(buffer != 0);

	int len;
	int total = 0;
	while((len = g_read(fd, buffer, len)) > 0)
	{
		total += len;
	}

	ASSERT(total == 620);
	g_close(fd);

	TEST_SUCCESSFUL;
}

void runStdioTest()
{
	openReadClose();
}
