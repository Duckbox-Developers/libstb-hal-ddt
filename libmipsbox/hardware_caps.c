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
#include <proc_tools.h>

#define FP_DEV "/dev/dbox/oled0"
static int initialized = 0;
static hw_caps_t caps;

hw_caps_t *get_hwcaps(void)
{
	if (initialized)
		return &caps;

	memset(&caps, 0, sizeof(hw_caps_t));

	caps.pip_devs = 0;
	if (access("/dev/dvb/adapter0/video1", F_OK) != -1) caps.pip_devs = 1;
	if (access("/dev/dvb/adapter0/video2", F_OK) != -1) caps.pip_devs = 2;
	if (access("/dev/dvb/adapter0/video3", F_OK) != -1) caps.pip_devs = 3;
	if (caps.pip_devs > 0) caps.can_pip = 1;

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
//	caps.has_SCART_input = 1;
	strcpy(caps.boxvendor, "VU+");
	strcpy(caps.boxname, "DUO");
	strcpy(caps.boxarch, "BCM7335");
#elif BOXMODEL_VUUNO
	initialized = 1;
	caps.has_CI = 2;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 12;
	caps.display_type = HW_DISPLAY_LINE_TEXT;
	caps.display_can_deepstandby = 1;
	caps.display_can_set_brightness = 1;
	caps.display_has_statusline = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;
	caps.has_SCART = 1;
	strcpy(caps.boxvendor, "VU+");
	strcpy(caps.boxname, "UNO");
	strcpy(caps.boxarch, "BCM7413");
#elif BOXMODEL_VUDUO2
	initialized = 1;
	caps.has_CI = 2;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 140;		// VFD, 400 - 2nd (color)
	caps.display_yres = 32;			// VFD, 240 - 2nd (color)
	caps.display_type = HW_DISPLAY_GFX;
	caps.display_can_deepstandby = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_can_set_brightness = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_has_statusline = 0;	// 0 because we use graphlcd/lcd4linux
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;
	caps.has_SCART = 1;
	strcpy(caps.boxvendor, "VU+");
	strcpy(caps.boxname, "DUO2");
	strcpy(caps.boxarch, "BCM7424");
#elif BOXMODEL_VUULTIMO
	initialized = 1;
	caps.has_CI = 2;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 256;
	caps.display_yres = 64;
	caps.display_type = HW_DISPLAY_GFX;
	caps.display_can_deepstandby = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_can_set_brightness = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_has_statusline = 0;	// 0 because we use graphlcd/lcd4linux
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;
	caps.has_SCART = 1;
	strcpy(caps.boxvendor, "VU+");
	strcpy(caps.boxname, "ULTIMO");
	strcpy(caps.boxarch, "BCM7413");
#elif BOXMODEL_DM8000
	initialized = 1;
	caps.has_CI = 2;
	caps.can_cec = 0;
	caps.can_shutdown = 1;
	caps.display_xres = 132;
	caps.display_yres = 64;
//	caps.display_xres = 400;		Grautec lcd
//	caps.display_yres = 240;		Grautec lcd
	caps.display_type = HW_DISPLAY_GFX;
	caps.display_can_deepstandby = 1;
	caps.display_can_set_brightness = 1;
	caps.display_has_statusline = 1;
	caps.display_has_colon = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;			// has only DVI, but works
	caps.has_SCART = 2;
//	caps.has_SCART_input = 1;
//	caps.has_DVI = 1;
	strcpy(caps.boxvendor, "DM");
	strcpy(caps.boxname, "8000HD");
	strcpy(caps.boxarch, "BCM7400D2");
#elif BOXMODEL_DM820
	initialized = 1;
	caps.has_CI = 0;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 96;
	caps.display_yres = 64;
	caps.display_type = HW_DISPLAY_GFX;
	caps.display_can_deepstandby = 1;
	caps.display_can_set_brightness = 1;
	caps.display_has_statusline = 1;
	caps.display_has_colon = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;		// 2
	caps.has_HDMI_input = 0;	// 1
	strcpy(caps.boxvendor, "DM");
	strcpy(caps.boxname, "820HD");
	strcpy(caps.boxarch, "BCM7434");
#elif BOXMODEL_DM7080
	initialized = 1;
	caps.has_CI = 2;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 132;
	caps.display_yres = 64;
//	caps.display_xres = 400;		Grautec lcd
//	caps.display_yres = 240;		Grautec lcd
	caps.display_type = HW_DISPLAY_GFX;
	caps.display_can_deepstandby = 1;
	caps.display_can_set_brightness = 1;
	caps.display_has_statusline = 1;
	caps.display_has_colon = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;		// 2
	caps.has_HDMI_input = 0;	// 1
	strcpy(caps.boxvendor, "DM");
	strcpy(caps.boxname, "7080HD");
	strcpy(caps.boxarch, "BCM7434");
#else // generic mips box
	initialized = 1;
	caps.has_CI = 1;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 4;
	caps.display_has_colon = 1;
	caps.display_type = HW_DISPLAY_LED_NUM;
	caps.display_can_deepstandby = 1;
	caps.display_can_set_brightness = 1;
	caps.display_has_statusline = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;
	caps.has_SCART = 1;
	proc_get("/proc/stb/info/boxtype", caps.boxvendor, 10);
	proc_get("/proc/stb/info/model", caps.boxname, 10);
	proc_get("/proc/stb/info/chipset", caps.boxarch, 10);
#endif
	return &caps;
}
