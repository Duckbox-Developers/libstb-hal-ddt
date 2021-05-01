/*
 * determine the capabilities of the hardware.
 * part of libstb-hal
 *
 * (C) 2010-2012 Stefan Seyfried
 *
 * License: GPL v2 or later
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <hardware_caps.h>

static int initialized = 0;
static hw_caps_t caps;

hw_caps_t *get_hwcaps(void)
{
	if (initialized)
		return &caps;

	memset(&caps, 0, sizeof(hw_caps_t));

	initialized = 1;
	char buf[64];
	int len = -1;
	int fd = open("/proc/stb/info/model", O_RDONLY);
	caps.display_can_set_brightness = 0;
	caps.display_can_deepstandby = 0;
	caps.display_has_statusline = 0;
	if (fd != -1) {
		len = read(fd, buf, sizeof(buf) - 1);
		close(fd);
	}
	if (len > 0) {
		buf[len] = 0;
		if (!strncmp(buf, "atevio7500", 10)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "atevio7500");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 1;
			caps.has_fan = 0;
			caps.has_CI = 2;
			caps.display_can_set_brightness = 1;
			caps.display_type = HW_DISPLAY_LINE_TEXT;
			caps.display_xres = 12;
		}
		else if (!strncmp(buf, "ufs912", 6)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "ufs912");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 1;
			caps.has_fan = 0;
			caps.has_CI = 2;
			caps.display_can_set_brightness = 1;
			caps.display_type = HW_DISPLAY_LINE_TEXT;
			caps.display_xres = 16;
		}
		else if (!strncmp(buf, "ufs913", 6)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "ufs913");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 1;
			caps.has_fan = 0;
			caps.has_CI = 2;
			caps.display_can_set_brightness = 1;
			caps.display_type = HW_DISPLAY_LINE_TEXT;
			caps.display_xres = 16;
		}
		else if (!strncmp(buf, "ufs922", 6)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "ufs922");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 1;
			caps.has_CI = 2;
			caps.display_can_set_brightness = 1;
			caps.display_type = HW_DISPLAY_LINE_TEXT;
			caps.display_xres = 16;
		}
		else if (!strncmp(buf, "ufs910", 6)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "ufs910");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 0;
			caps.has_CI = 2;
			caps.display_can_set_brightness = 1;
			caps.display_type = HW_DISPLAY_LINE_TEXT;
			caps.display_xres = 16;
		}
		else if (!strncmp(buf, "hdbox", 5)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "hdbox");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 0;
			caps.has_CI = 2;
			caps.display_can_set_brightness = 1;
			caps.display_type = HW_DISPLAY_LINE_TEXT;
			caps.display_xres = 12;
		}
		else if (!strncmp(buf, "octagon1008", 11)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "octagon1008");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 0;
			caps.has_CI = 2;
			caps.display_can_set_brightness = 1;
			caps.display_type = HW_DISPLAY_LINE_TEXT;
			caps.display_xres = 8;
		}
		else if (!strncmp(buf, "cuberevo-mini", 14)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "cuberevo-mini");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 0;
			caps.has_CI = 2;
		}
		else if (!strncmp(buf, "cuberevo-mini2", 14)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "cuberevo-mini2");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 1;
			caps.has_CI = 2;
			caps.display_can_set_brightness = 1;
			caps.display_type = HW_DISPLAY_LINE_TEXT;
			caps.display_xres = 14;
		}
		else if (!strncmp(buf, "cuberevo-250hd", 4)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "cuberevo-250hd");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 1;
			caps.has_CI = 0;
		}
		else if (!strncmp(buf, "cuberevo-2000hd", 15)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "cuberevo-2000hd");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 0;
			caps.has_CI = 0;
		}
		else if (!strncmp(buf, "cuberevo", 8)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "cuberevo");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 1;
			caps.has_CI = 2;
		}
		else if (!strncmp(buf, "cuberevo-3000hd", 14)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "cuberevo-3000hd");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 1;
			caps.has_CI = 2;
		}
		else if (!strncmp(buf, "ipbox9900", 9)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "ipbox9900");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 2;
			caps.can_cec = 0;
			caps.has_fan = 1;
			caps.has_CI = 2;
		}
		else if (!strncmp(buf, "ipbox99", 7)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "ipbox99");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 1;
			caps.has_CI = 0;
		}
		else if (!strncmp(buf, "ipbox55", 7)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "ipbox55");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 0;
			caps.has_CI = 0;
		}
		else if (!strncmp(buf, "tf7700", 6)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, "tf7700");
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 0;
			caps.has_CI = 2;
		}
		else {
			strcpy(caps.boxvendor, "unknown");
			strcpy(caps.boxname, buf);
		}
	}
	else
		strcpy(caps.boxname, "(unknown model)");

	strcpy(caps.boxarch,caps.boxname);
	return &caps;
}
