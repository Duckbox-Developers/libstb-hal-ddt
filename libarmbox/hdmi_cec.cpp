/*
	Copyright (C) 2018 TangoCash

	License: GPLv2

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation;

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <errno.h>
#include <ctype.h>

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <linux/input.h>

#include "linux-uapi-cec.h"
#include "hdmi_cec.h"
#include "hdmi_cec_types.h"
#include "hal_debug.h"

#define RED "\x1B[31m"
#define NORMAL "\x1B[0m"

#define hal_debug(args...) _hal_debug(HAL_DEBUG_INIT, this, args)
#define hal_info(args...) _hal_info(HAL_DEBUG_INIT, this, args)
#define hal_debug_c(args...) _hal_debug(HAL_DEBUG_INIT, NULL, args)
#define hal_info_c(args...) _hal_info(HAL_DEBUG_INIT, NULL, args)

#define fop(cmd, args...) ({				\
	int _r;						\
	if (fd >= 0) { 					\
		if ((_r = ::cmd(fd, args)) < 0)		\
			hal_info(#cmd"(fd, "#args")\n");	\
		else					\
			hal_debug(#cmd"(fd, "#args")\n");\
	}						\
	else { _r = fd; } 				\
	_r;						\
})

#define CEC_DEVICE "/dev/cec0"
#define RC_DEVICE  "/dev/input/event1"

hdmi_cec * hdmi_cec::hdmi_cec_instance = NULL;

//hack to get an instance before first call
hdmi_cec * CEC = hdmi_cec::getInstance();

hdmi_cec::hdmi_cec()
{
	standby_cec_activ = autoview_cec_activ = standby = muted = false;
	hdmiFd = -1;
	volume = 0;
}

hdmi_cec::~hdmi_cec()
{
	if (hdmiFd >= 0)
	{
		close(hdmiFd);
		hdmiFd = -1;
	}
}

hdmi_cec* hdmi_cec::getInstance()
{
	if (hdmi_cec_instance == NULL)
	{
		hdmi_cec_instance = new hdmi_cec();
		hal_debug_c("[CEC] new instance created \n");
	}
	return hdmi_cec_instance;
}

bool hdmi_cec::SetCECMode(VIDEO_HDMI_CEC_MODE _deviceType)
{
	physicalAddress[0] = 0x10;
	physicalAddress[1] = 0x00;
	logicalAddress = 1;

	if (_deviceType == VIDEO_HDMI_CEC_MODE_OFF)
	{
		Stop();
		hal_debug("[CEC] switch off %s\n", __func__);
		return false;
	}
	else
		deviceType = _deviceType;

	hal_debug("[CEC] switch on %s\n", __func__);

	if (hdmiFd == -1)
	{
		hdmiFd = open(CEC_DEVICE, O_RDWR | O_CLOEXEC);
	}

	if (hdmiFd >= 0)
	{
		__u32 monitor = CEC_MODE_INITIATOR | CEC_MODE_FOLLOWER;
		struct cec_caps caps = {};

		if (ioctl(hdmiFd, CEC_ADAP_G_CAPS, &caps) < 0)
			hal_debug("[CEC] %s: get caps failed (%m)\n", __func__);

		if (caps.capabilities & CEC_CAP_LOG_ADDRS)
		{
			struct cec_log_addrs laddrs = {};

			if (ioctl(hdmiFd, CEC_ADAP_S_LOG_ADDRS, &laddrs) < 0)
				hal_debug("[CEC] %s: reset log addr failed (%m)\n", __func__);

			memset(&laddrs, 0, sizeof(laddrs));

			/*
			 * NOTE: cec_version, osd_name and deviceType should be made configurable,
			 * CEC_ADAP_S_LOG_ADDRS delayed till the desired values are available
			 * (saves us some startup speed as well, polling for a free logical address
			 * takes some time)
			 */
			laddrs.cec_version = CEC_OP_CEC_VERSION_2_0;
			strcpy(laddrs.osd_name, "neutrino");
			laddrs.vendor_id = CEC_VENDOR_ID_NONE;

			switch (deviceType)
			{
			case CEC_LOG_ADDR_TV:
				laddrs.log_addr_type[laddrs.num_log_addrs] = CEC_LOG_ADDR_TYPE_TV;
				laddrs.all_device_types[laddrs.num_log_addrs] = CEC_OP_ALL_DEVTYPE_TV;
				laddrs.primary_device_type[laddrs.num_log_addrs] = CEC_OP_PRIM_DEVTYPE_TV;
				break;
			case CEC_LOG_ADDR_RECORD_1:
				laddrs.log_addr_type[laddrs.num_log_addrs] = CEC_LOG_ADDR_TYPE_RECORD;
				laddrs.all_device_types[laddrs.num_log_addrs] = CEC_OP_ALL_DEVTYPE_RECORD;
				laddrs.primary_device_type[laddrs.num_log_addrs] = CEC_OP_PRIM_DEVTYPE_RECORD;
				break;
			case CEC_LOG_ADDR_TUNER_1:
				laddrs.log_addr_type[laddrs.num_log_addrs] = CEC_LOG_ADDR_TYPE_TUNER;
				laddrs.all_device_types[laddrs.num_log_addrs] = CEC_OP_ALL_DEVTYPE_TUNER;
				laddrs.primary_device_type[laddrs.num_log_addrs] = CEC_OP_PRIM_DEVTYPE_TUNER;
				break;
			case CEC_LOG_ADDR_PLAYBACK_1:
				laddrs.log_addr_type[laddrs.num_log_addrs] = CEC_LOG_ADDR_TYPE_PLAYBACK;
				laddrs.all_device_types[laddrs.num_log_addrs] = CEC_OP_ALL_DEVTYPE_PLAYBACK;
				laddrs.primary_device_type[laddrs.num_log_addrs] = CEC_OP_PRIM_DEVTYPE_PLAYBACK;
				break;
			case CEC_LOG_ADDR_AUDIOSYSTEM:
				laddrs.log_addr_type[laddrs.num_log_addrs] = CEC_LOG_ADDR_TYPE_AUDIOSYSTEM;
				laddrs.all_device_types[laddrs.num_log_addrs] = CEC_OP_ALL_DEVTYPE_AUDIOSYSTEM;
				laddrs.primary_device_type[laddrs.num_log_addrs] = CEC_OP_PRIM_DEVTYPE_AUDIOSYSTEM;
				break;
			default:
				laddrs.log_addr_type[laddrs.num_log_addrs] = CEC_LOG_ADDR_TYPE_UNREGISTERED;
				laddrs.all_device_types[laddrs.num_log_addrs] = CEC_OP_ALL_DEVTYPE_SWITCH;
				laddrs.primary_device_type[laddrs.num_log_addrs] = CEC_OP_PRIM_DEVTYPE_SWITCH;
				break;
			}
			laddrs.num_log_addrs++;

			if (ioctl(hdmiFd, CEC_ADAP_S_LOG_ADDRS, &laddrs) < 0)
				hal_debug("[CEC] %s: et log addr failed (%m)\n", __func__);
		}

		if (ioctl(hdmiFd, CEC_S_MODE, &monitor) < 0)
			hal_debug("[CEC] %s: monitor failed (%m)\n", __func__);

		GetCECAddressInfo();

		if(autoview_cec_activ)
			SetCECState(false);

		Start();
		return true;

	}
	return false;
}

void hdmi_cec::GetCECAddressInfo()
{
	if (hdmiFd >= 0)
	{
		struct addressinfo addressinfo;

		__u16 phys_addr;
		struct cec_log_addrs laddrs = {};

		::ioctl(hdmiFd, CEC_ADAP_G_PHYS_ADDR, &phys_addr);
		addressinfo.physical[0] = (phys_addr >> 8) & 0xff;
		addressinfo.physical[1] = phys_addr & 0xff;

		::ioctl(hdmiFd, CEC_ADAP_G_LOG_ADDRS, &laddrs);
		addressinfo.logical = laddrs.log_addr[0];

		switch (laddrs.log_addr_type[0])
		{
		case CEC_LOG_ADDR_TYPE_TV:
			addressinfo.type = CEC_LOG_ADDR_TV;
			break;
		case CEC_LOG_ADDR_TYPE_RECORD:
			addressinfo.type = CEC_LOG_ADDR_RECORD_1;
			break;
		case CEC_LOG_ADDR_TYPE_TUNER:
			addressinfo.type = CEC_LOG_ADDR_TUNER_1;
			break;
		case CEC_LOG_ADDR_TYPE_PLAYBACK:
			addressinfo.type = CEC_LOG_ADDR_PLAYBACK_1;
			break;
		case CEC_LOG_ADDR_TYPE_AUDIOSYSTEM:
			addressinfo.type = CEC_LOG_ADDR_AUDIOSYSTEM;
			break;
		case CEC_LOG_ADDR_TYPE_UNREGISTERED:
		default:
			addressinfo.type = CEC_LOG_ADDR_UNREGISTERED;
			break;
		}

		deviceType = addressinfo.type;
		logicalAddress = addressinfo.logical;
		if (memcmp(physicalAddress, addressinfo.physical, sizeof(physicalAddress)))
		{
			hal_info("[CEC] %s: detected physical address change: %02X%02X --> %02X%02X\n", __func__, physicalAddress[0], physicalAddress[1], addressinfo.physical[0], addressinfo.physical[1]);
			memcpy(physicalAddress, addressinfo.physical, sizeof(physicalAddress));
			ReportPhysicalAddress();
		}
	}
}

void hdmi_cec::ReportPhysicalAddress()
{
	struct cec_message txmessage;
	txmessage.initiator = logicalAddress;
	txmessage.destination = CEC_LOG_ADDR_BROADCAST;
	txmessage.data[0] = CEC_MSG_REPORT_PHYSICAL_ADDR;
	txmessage.data[1] = physicalAddress[0];
	txmessage.data[2] = physicalAddress[1];
	txmessage.data[3] = deviceType;
	txmessage.length = 4;
	SendCECMessage(txmessage);
}

void hdmi_cec::SendCECMessage(struct cec_message &txmessage)
{
	if (hdmiFd >= 0)
	{
		char str[txmessage.length*6];
		for (int i = 0; i < txmessage.length; i++)
		{
			sprintf(str+(i*6),"[0x%02X]", txmessage.data[i]);
		}
		hal_info("[CEC] send message %s to %s (0x%02X>>0x%02X) '%s' (%s)\n",ToString((cec_logical_address)txmessage.initiator), txmessage.destination == 0xf ? "all" : ToString((cec_logical_address)txmessage.destination), txmessage.initiator, txmessage.destination, ToString((cec_opcode)txmessage.data[0]), str);
		struct cec_msg msg;
		cec_msg_init(&msg, txmessage.initiator, txmessage.destination);
		memcpy(&msg.msg[1], txmessage.data, txmessage.length);
		msg.len = txmessage.length + 1;
		ioctl(hdmiFd, CEC_TRANSMIT, &msg);
	}
}

void hdmi_cec::SetCECAutoStandby(bool state)
{
	standby_cec_activ = state;
}

void hdmi_cec::SetCECAutoView(bool state)
{
	autoview_cec_activ = state;
}

void hdmi_cec::SetCECState(bool state)
{
	struct cec_message message;

	standby = state;

	if ((standby_cec_activ) && state)
	{
		message.initiator = logicalAddress;
		message.destination = CEC_OP_PRIM_DEVTYPE_TV;
		message.data[0] = CEC_MSG_STANDBY;
		message.length = 1;
		SendCECMessage(message);
	}

	if ((autoview_cec_activ) && !state)
	{
		message.initiator = logicalAddress;
		message.destination = CEC_OP_PRIM_DEVTYPE_AUDIOSYSTEM;
		message.data[0] = CEC_MSG_GIVE_SYSTEM_AUDIO_MODE_STATUS;
		message.length = 1;
		SendCECMessage(message);
		usleep(10000);

		message.initiator = logicalAddress;
		message.destination = CEC_OP_PRIM_DEVTYPE_TV;
		message.data[0] = CEC_MSG_IMAGE_VIEW_ON;
		message.length = 1;
		SendCECMessage(message);
		usleep(10000);

		message.initiator = logicalAddress;
		message.destination = CEC_LOG_ADDR_BROADCAST;
		message.data[0] = CEC_MSG_ACTIVE_SOURCE;
		message.data[1] = physicalAddress[0];
		message.data[2] = physicalAddress[1];
		message.length = 3;
		SendCECMessage(message);
		usleep(10000);

		request_audio_status();
	}

}

long hdmi_cec::translateKey(unsigned char code)
{
	long key = 0;
	switch (code)
	{
	case CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL:
		key = KEY_MENU;
		break;
	case CEC_USER_CONTROL_CODE_NUMBER0:
		key = KEY_0;
		break;
	case CEC_USER_CONTROL_CODE_NUMBER1:
		key = KEY_1;
		break;
	case CEC_USER_CONTROL_CODE_NUMBER2:
		key = KEY_2;
		break;
	case CEC_USER_CONTROL_CODE_NUMBER3:
		key = KEY_3;
		break;
	case CEC_USER_CONTROL_CODE_NUMBER4:
		key = KEY_4;
		break;
	case CEC_USER_CONTROL_CODE_NUMBER5:
		key = KEY_5;
		break;
	case CEC_USER_CONTROL_CODE_NUMBER6:
		key = KEY_6;
		break;
	case CEC_USER_CONTROL_CODE_NUMBER7:
		key = KEY_7;
		break;
	case CEC_USER_CONTROL_CODE_NUMBER8:
		key = KEY_8;
		break;
	case CEC_USER_CONTROL_CODE_NUMBER9:
		key = KEY_9;
		break;
	case CEC_USER_CONTROL_CODE_CHANNEL_UP:
		key = KEY_CHANNELUP;
		break;
	case CEC_USER_CONTROL_CODE_CHANNEL_DOWN:
		key = KEY_CHANNELDOWN;
		break;
	case CEC_USER_CONTROL_CODE_PLAY:
		key = KEY_PLAY;
		break;
	case CEC_USER_CONTROL_CODE_STOP:
		key = KEY_STOP;
		break;
	case CEC_USER_CONTROL_CODE_PAUSE:
		key = KEY_PAUSE;
		break;
	case CEC_USER_CONTROL_CODE_RECORD:
		key = KEY_RECORD;
		break;
	case CEC_USER_CONTROL_CODE_REWIND:
		key = KEY_REWIND;
		break;
	case CEC_USER_CONTROL_CODE_FAST_FORWARD:
		key = KEY_FASTFORWARD;
		break;
	case CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE:
		key = KEY_INFO;
		break;
	case CEC_USER_CONTROL_CODE_TIMER_PROGRAMMING:
		key = KEY_PROGRAM;
		break;
	case CEC_USER_CONTROL_CODE_PLAY_FUNCTION:
		key = KEY_PLAY;
		break;
	case CEC_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION:
		key = KEY_PLAYPAUSE;
		break;
	case CEC_USER_CONTROL_CODE_RECORD_FUNCTION:
		key = KEY_RECORD;
		break;
	case CEC_USER_CONTROL_CODE_STOP_FUNCTION:
		key = KEY_STOP;
		break;
	case CEC_USER_CONTROL_CODE_SELECT:
		key = KEY_OK;
		break;
	case CEC_USER_CONTROL_CODE_LEFT:
		key = KEY_LEFT;
		break;
	case CEC_USER_CONTROL_CODE_RIGHT:
		key = KEY_RIGHT;
		break;
	case CEC_USER_CONTROL_CODE_UP:
		key = KEY_UP;
		break;
	case CEC_USER_CONTROL_CODE_DOWN:
		key = KEY_DOWN;
		break;
	case CEC_USER_CONTROL_CODE_EXIT:
		key = KEY_EXIT;
		break;
	case CEC_USER_CONTROL_CODE_F2_RED:
		key = KEY_RED;
		break;
	case CEC_USER_CONTROL_CODE_F3_GREEN:
		key = KEY_GREEN;
		break;
	case CEC_USER_CONTROL_CODE_F4_YELLOW:
		key = KEY_YELLOW;
		break;
	case CEC_USER_CONTROL_CODE_F1_BLUE:
		key = KEY_BLUE;
		break;
	default:
		key = KEY_MENU;
		break;
	}
	return key;
}

bool hdmi_cec::Start()
{
	if (running)
		return false;

	if (hdmiFd == -1)
		return false;

	running = true;
	return (OpenThreads::Thread::start() == 0);
}

bool hdmi_cec::Stop()
{
	if (!running)
		return false;

	running = false;
	
	OpenThreads::Thread::cancel();

	if (hdmiFd >= 0)
	{
		close(hdmiFd);
		hdmiFd = -1;
	}

	return (OpenThreads::Thread::join() == 0);
}

void hdmi_cec::run()
{
	OpenThreads::Thread::setCancelModeAsynchronous();
	struct pollfd pfd;

	pfd.fd = hdmiFd;
	pfd.events = (POLLIN | POLLPRI);

	while (running)
	{
		if (poll(&pfd, 1, 0) > 0)
			Receive();
	}
}

void hdmi_cec::Receive()
{
	bool hasdata = false;
	struct cec_message rxmessage;
	struct cec_message txmessage;

	struct cec_msg msg;
	if (::ioctl(hdmiFd, CEC_RECEIVE, &msg) >= 0)
	{
		rxmessage.length = msg.len - 1;
		rxmessage.initiator = cec_msg_initiator(&msg);
		rxmessage.destination = cec_msg_destination(&msg);
		rxmessage.opcode = cec_msg_opcode(&msg);
		memcpy(&rxmessage.data, &msg.msg[1], rxmessage.length);
		hasdata = true;
	}

	if (hasdata)
	{
		bool keypressed = false;
		static unsigned char pressedkey = 0;

		char str[rxmessage.length*6];
		for (int i = 0; i < rxmessage.length; i++)
		{
			sprintf(str+(i*6),"[0x%02X]", rxmessage.data[i]);
		}
		hal_info("[CEC] received message %s to %s (0x%02X>>0x%02X) '%s' (%s)\n",ToString((cec_logical_address)rxmessage.initiator), rxmessage.destination == 0xf ? "all" : ToString((cec_logical_address)rxmessage.destination), rxmessage.initiator, rxmessage.destination, ToString((cec_opcode)rxmessage.opcode), str);

		switch (rxmessage.opcode)
		{
		case CEC_OPCODE_REPORT_AUDIO_STATUS:
		{
			muted = ((rxmessage.data[1] & 0x80) == 0x80);
			volume = ((rxmessage.data[1] & 0x7F) / 127.0) * 100.0;
			if (muted)
				hal_debug("[CEC] %s volume muted\n", ToString((cec_logical_address)rxmessage.initiator));
			else
				hal_debug("[CEC] %s volume %d \n", ToString((cec_logical_address)rxmessage.initiator), volume);
			break;
		}
		case CEC_OPCODE_DEVICE_VENDOR_ID:
		case CEC_OPCODE_VENDOR_COMMAND_WITH_ID:
		{
			uint64_t iVendorId =	((uint64_t)rxmessage.data[1] << 16) +
			                        ((uint64_t)rxmessage.data[2] << 8) +
			                        (uint64_t)rxmessage.data[3];
			hal_debug("[CEC] decoded message '%s' (%s)\n", ToString((cec_opcode)rxmessage.opcode), ToString((cec_vendor_id)iVendorId));
			break;
		}
		case CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
		{
			txmessage.destination = rxmessage.initiator;
			txmessage.initiator = rxmessage.destination;
			txmessage.data[0] = GetResponseOpcode((cec_opcode)rxmessage.opcode);
			txmessage.data[1] = standby ? CEC_POWER_STATUS_STANDBY : CEC_POWER_STATUS_ON;
			txmessage.length = 2;
			SendCECMessage(txmessage);
			break;
		}
		case CEC_OPCODE_USER_CONTROL_PRESSED: /* key pressed */
		{
			keypressed = true;
			pressedkey = rxmessage.data[1];
		} // fall through
		case CEC_OPCODE_USER_CONTROL_RELEASE: /* key released */
		{
			long code = translateKey(pressedkey);
			hal_debug("[CEC] decoded key %s (%ld)\n",ToString((cec_user_control_code)pressedkey), code);
			handleCode(code,keypressed);
			break;
		}
		}
	}
}

void hdmi_cec::handleCode(long code, bool keypressed)
{
	int evd = open(RC_DEVICE, O_RDWR);
	if (evd < 0)
	{
		hal_debug("[CEC] opening " RC_DEVICE " failed");
		return;
	}
	if (keypressed)
	{
		if (rc_send(evd, code, CEC_KEY_PRESSED) < 0)
		{
			hal_debug("[CEC] writing 'KEY_PRESSED' event failed");
			close(evd);
			return;
		}
		rc_sync(evd);
	}
	else
	{
		if (rc_send(evd, code, CEC_KEY_RELEASED) < 0)
		{
			hal_debug("[CEC] writing 'KEY_RELEASED' event failed");
			close(evd);
			return;
		}
		rc_sync(evd);
	}
	close(evd);
}

int hdmi_cec::rc_send(int fd, unsigned int code, unsigned int value)
{
	struct input_event ev;

	ev.type = EV_KEY;
	ev.code = code;
	ev.value = value;
	return write(fd, &ev, sizeof(ev));
}

void hdmi_cec::rc_sync(int fd)
{
	struct input_event ev;

	gettimeofday(&ev.time, NULL);
	ev.type = EV_SYN;
	ev.code = SYN_REPORT;
	ev.value = 0;
	write(fd, &ev, sizeof(ev));
}

void hdmi_cec::send_key(unsigned char key, unsigned char destination)
{
	struct cec_message txmessage;
	txmessage.destination = destination;
	txmessage.initiator = logicalAddress;
	txmessage.data[0] = CEC_OPCODE_USER_CONTROL_PRESSED;
	txmessage.data[1] = key;
	txmessage.length = 2;
	SendCECMessage(txmessage);
	usleep(10000);

	txmessage.destination = destination;
	txmessage.initiator = logicalAddress;
	txmessage.data[0] = CEC_OPCODE_USER_CONTROL_RELEASE;
	txmessage.length = 1;
	SendCECMessage(txmessage);
}

void hdmi_cec::request_audio_status()
{
	struct cec_message txmessage;
	txmessage.destination = CEC_OP_PRIM_DEVTYPE_AUDIOSYSTEM;
	txmessage.initiator = logicalAddress;
	txmessage.data[0] = CEC_OPCODE_GIVE_AUDIO_STATUS;
	txmessage.length = 1;
	SendCECMessage(txmessage);
}

void hdmi_cec::vol_up()
{
	send_key(CEC_USER_CONTROL_CODE_VOLUME_UP, CEC_OP_PRIM_DEVTYPE_AUDIOSYSTEM);
	usleep(50000);
	request_audio_status();
}
void hdmi_cec::vol_down()
{
	send_key(CEC_USER_CONTROL_CODE_VOLUME_DOWN, CEC_OP_PRIM_DEVTYPE_AUDIOSYSTEM);
	usleep(50000);
	request_audio_status();
}
void hdmi_cec::toggle_mute()
{
	send_key(CEC_USER_CONTROL_CODE_MUTE, CEC_OP_PRIM_DEVTYPE_AUDIOSYSTEM);
	usleep(50000);
	request_audio_status();
}
