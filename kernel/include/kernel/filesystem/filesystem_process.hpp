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

#ifndef __KERNEL_FILESYSTEM_PROCESS__
#define __KERNEL_FILESYSTEM_PROCESS__

#include "ghost/kernel.h"
#include "ghost/fs.h"
#include "kernel/utils/hashmap.hpp"

/**
 * Structure of a file descriptor.
 */
struct g_file_descriptor
{
	g_fd id;
	int64_t offset;
	g_fs_virt_id nodeId;
	int32_t openFlags;
};

/**
 * Per-process file system information structure.
 */
struct g_filesystem_process
{
	g_fd nextDescriptor;
	g_mutex nextDescriptorLock;
	g_hashmap<g_fd, g_file_descriptor>* descriptors;
};

/**
 * Initializes this unit.
 */
void filesystemProcessInitialize();

/**
 * Creates a file system information structure for a process.
 */
void filesystemProcessCreate(g_pid pid);

/**
 * Removes file system information for a process.
 */
void filesystemProcessRemove(g_pid pid);

#endif
