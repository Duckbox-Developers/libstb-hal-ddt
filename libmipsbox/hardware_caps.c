/*
 * determine the capabilities of the hardware.
 * part of libstb-hal
 *
 * (C) 2010-2012 Stefan Seyfried
 *
 * License: GPL v2 or later
 */

#include <config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <hardware_caps.h>

#define FP_DEV "/dev/dbox/oled0"
static int initialized = 0;
static hw_caps_t caps;

hw_caps_t *get_hwcaps(void)
{
	if (initialized)
		return &caps;

	memset(&caps, 0, sizeof(hw_caps_t));

#if BOXMODEL_VUDUO
	initialized = 1;
	caps.has_CI = 2;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 16;
	caps.display_type = HW_DISPLAY_LINE_TEXT;
	caps.display_can_deepstandby = 1;
	caps.display_can_set_brightness = 1;
	caps.display_has_statusline = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;
	caps.has_SCART = 1;
	strcpy(caps.boxvendor, "VU+");
	strcpy(caps.boxname, "DUO");
	strcpy(caps.boxarch, "BCM7335");
#endif
#if BOXMODEL_DM8000
	initialized = 1;
	caps.has_fan = 1;
	caps.has_CI = 4;
	caps.can_cec = 0;
	caps.can_shutdown = 1;
	caps.display_xres = 400;
	caps.display_yres = 240;
	caps.display_type = HW_DISPLAY_GFX;
	caps.display_can_deepstandby = 1;
	caps.display_can_set_brightness = 1;
	caps.display_has_statusline = 1;
	caps.display_has_colon = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;			// wrong (has only DVI), only for testing
	caps.has_SCART = 2;
//	caps.has_DVI = 1;
	strcpy(caps.boxvendor, "DM");
	strcpy(caps.boxname, "8000HD");
	strcpy(caps.boxarch, "BCM7400D2");
#endif
	return &caps;
}
