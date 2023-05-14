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

#if BOXMODEL_VUSOLO4K
	initialized = 1;
	caps.has_CI = 1;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 480;
	caps.display_yres = 320;
	caps.display_type = HW_DISPLAY_GFX;
	caps.display_can_deepstandby = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_can_set_brightness = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_has_statusline = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_has_colon = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;
	caps.pip_devs = 2;			// has only 3 real usable video devices
	strcpy(caps.boxvendor, "VU+");
	strcpy(caps.boxname, "SOLO4K");
	strcpy(caps.boxarch, "BCM7376");
#elif BOXMODEL_VUDUO4K
	initialized = 1;
	caps.has_CI = 2;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 480;
	caps.display_yres = 320;
	caps.display_type = HW_DISPLAY_GFX;
	caps.display_can_deepstandby = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_can_set_brightness = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_has_statusline = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_has_colon = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 2;
	caps.has_HDMI_input = 1;
	strcpy(caps.boxvendor, "VU+");
	strcpy(caps.boxname, "DUO4K");
	strcpy(caps.boxarch, "BCM7278");
#elif BOXMODEL_VUDUO4KSE
	initialized = 1;
	caps.has_CI = 2;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 480;
	caps.display_yres = 320;
	caps.display_type = HW_DISPLAY_GFX;
	caps.display_can_deepstandby = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_can_set_brightness = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_has_statusline = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_has_colon = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 2;
	caps.has_HDMI_input = 1;
	strcpy(caps.boxvendor, "VU+");
	strcpy(caps.boxname, "DUO4KSE");
	strcpy(caps.boxarch, "BCM7444S");
#elif BOXMODEL_VUULTIMO4K
	initialized = 1;
	caps.has_CI = 2;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 800;
	caps.display_yres = 480;
	caps.display_type = HW_DISPLAY_GFX;
	caps.display_can_deepstandby = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_can_set_brightness = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_has_statusline = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_has_colon = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 2;
	caps.has_HDMI_input = 1;
	strcpy(caps.boxvendor, "VU+");
	strcpy(caps.boxname, "ULTIMO4K");
	strcpy(caps.boxarch, "BCM7444S");
#elif BOXMODEL_VUZERO4K
	initialized = 1;
	caps.has_CI = 1;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_type = HW_DISPLAY_LED_ONLY;
	caps.display_can_deepstandby = 0;
	caps.display_can_set_brightness = 1;
	caps.display_has_statusline = 0;
	caps.display_has_colon = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;
	strcpy(caps.boxvendor, "VU+");
	strcpy(caps.boxname, "ZERO4K");
	strcpy(caps.boxarch, "BCM72604");
#elif BOXMODEL_VUUNO4KSE
	initialized = 1;
	caps.has_CI = 1;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 400;
	caps.display_yres = 240;
	caps.display_type = HW_DISPLAY_GFX;
	caps.display_can_deepstandby = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_can_set_brightness = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_has_statusline = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_has_colon = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 2;
	caps.has_HDMI_input = 1;
	strcpy(caps.boxvendor, "VU+");
	strcpy(caps.boxname, "UNO4KSE");
	strcpy(caps.boxarch, "BCM7252S");
#elif BOXMODEL_VUUNO4K
	initialized = 1;
	caps.has_CI = 1;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_type = HW_DISPLAY_LED_ONLY;
	caps.display_can_deepstandby = 0;
	caps.display_can_set_brightness = 1;
	caps.display_has_statusline = 0;
	caps.display_has_colon = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;
	strcpy(caps.boxvendor, "VU+");
	strcpy(caps.boxname, "UNO4K");
	strcpy(caps.boxarch, "BCM7252S");
#elif BOXMODEL_BRE2ZE4K
	initialized = 1;
	caps.has_CI = 1;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 4;
	caps.display_type = HW_DISPLAY_LED_NUM;
	caps.display_can_deepstandby = 0;
	caps.display_can_set_brightness = 1;
	caps.display_has_statusline = 0;
	caps.display_has_colon = 1;
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;
	strcpy(caps.boxvendor, "WWIO");
	strcpy(caps.boxname, "BRE2ZE4K");
	strcpy(caps.boxarch, "BCM7251S");
#elif BOXMODEL_HD51
	initialized = 1;
	caps.has_CI = 1;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 16;
	caps.display_type = HW_DISPLAY_LINE_TEXT;
	caps.display_can_deepstandby = 0;
	caps.display_can_set_brightness = 1;
	caps.display_has_statusline = 0;
	caps.display_has_colon = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;
	strcpy(caps.boxvendor, "AX");
	strcpy(caps.boxname, "HD51");
	strcpy(caps.boxarch, "BCM7251S");
#elif BOXMODEL_H7
	initialized = 1;
	caps.has_CI = 1;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 4;
	caps.display_type = HW_DISPLAY_LED_NUM;
	caps.display_can_deepstandby = 0;
	caps.display_can_set_brightness = 1;
	caps.display_has_statusline = 0;
	caps.display_has_colon = 1;
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;
	strcpy(caps.boxvendor, "AirDigital");
	strcpy(caps.boxname, "Zgemma H7");
	strcpy(caps.boxarch, "BCM7251S");
#elif BOXMODEL_E4HDULTRA
	initialized = 1;
	caps.has_CI = 1;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 220;
	caps.display_yres = 176;
	caps.display_type = HW_DISPLAY_GFX;
	caps.display_can_deepstandby = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_can_set_brightness = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_has_statusline = 0;	// 0 because we use graphlcd/lcd4linux
	caps.display_has_colon = 0;
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;
	strcpy(caps.boxvendor, "AXAS");
	strcpy(caps.boxname, "E4HD 4K ULTRA");
	strcpy(caps.boxarch, "BCM7252S");
#else // generic arm box
	initialized = 1;
	caps.has_CI = 1;
	caps.can_cec = 1;
	caps.can_shutdown = 1;
	caps.display_xres = 4;
	caps.display_has_colon = 1;
	caps.display_type = HW_DISPLAY_LED_NUM;
	caps.display_can_deepstandby = 1;	// 0 because we use graphlcd/lcd4linux
	caps.display_can_set_brightness = 1;	// 0 because we use graphlcd/lcd4linux
	caps.display_has_statusline = 0;	// 0 because we use graphlcd/lcd4linux
	caps.has_button_timer = 1;
	caps.has_HDMI = 1;
	caps.has_SCART = 1;
	proc_get("/proc/stb/info/boxtype", caps.boxvendor, 10);
	proc_get("/proc/stb/info/model", caps.boxname, 10);
	proc_get("/proc/stb/info/chipset", caps.boxarch, 10);
#endif
	return &caps;
}
