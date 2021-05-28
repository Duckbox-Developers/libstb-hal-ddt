/*
 * (C) 2010-2013 Stefan Seyfried
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>

#include "pwrmngr.h"
#include "hal_debug.h"

#include <stdio.h>
#include <cstdlib>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define hal_debug(args...) _hal_debug(HAL_DEBUG_PWRMNGR, this, args)
#define hal_info(args...) _hal_info(HAL_DEBUG_PWRMNGR, this, args)

/* powermanager */
bool cPowerManager::Open(void)
{
	hal_debug("%s\n", __func__);
	return true;
}

void cPowerManager::Close(void)
{
	hal_debug("%s\n", __func__);
}

bool cPowerManager::SetStandby(bool Active, bool Passive)
{
	hal_debug("%s(%d, %d)\n", __func__, Active, Passive);
	return true;
}

cPowerManager::cPowerManager(void)
{
	hal_debug("%s\n", __func__);
}

cPowerManager::~cPowerManager()
{
	hal_debug("%s\n", __func__);
}
