/*
 * cDemux implementation for the Tripledragon dbs3000 receiver
 *
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <inttypes.h>
#include <unistd.h>

#include <cstring>
#include <cstdio>
#include <string>
#include <hardware/tddevices.h>
#include "dmx_hal.h"
#include "hal_debug.h"

/* Ugh... see comment in destructor for details... */
#include "video_td.h"
extern cVideo *videoDecoder;

#define hal_debug(args...) _hal_debug(HAL_DEBUG_DEMUX, this, args)
#define hal_info(args...) _hal_info(HAL_DEBUG_DEMUX, this, args)

#define dmx_err(_errfmt, _errstr, _revents) do { \
	hal_info("%s " _errfmt " fd:%d, ev:0x%x %s pid:0x%04hx flt:0x%02hx\n", \
		__func__, _errstr, fd, _revents, DMX_T[dmx_type], pid, flt); \
} while(0);

cDemux *videoDemux = NULL;
cDemux *audioDemux = NULL;
//cDemux *pcrDemux = NULL;

static const char *DMX_T[] = {
	"DMX_INVALID",
	"DMX_VIDEO",
	"DMX_AUDIO",
	"DMX_PES",
	"DMX_PSI",
	"DMX_PIP",
	"DMX_TP",
	"DMX_PCR"
};

/* map the device numbers as used to the TD devices */
static const char *devname[] = {
	"/dev/" DEVICE_NAME_DEMUX "0",
	"/dev/" DEVICE_NAME_DEMUX "1",
	"/dev/" DEVICE_NAME_DEMUX "2",
};

/* uuuugly */
static int dmx_tp_count = 0;
#define MAX_TS_COUNT 1

typedef struct dmx_pdata {
	bool measure;
	int last_measure;
	int last_data;
	int devnum;
	bool running;
} dmx_pdata;
#define P ((dmx_pdata *)pdata)

cDemux::cDemux(int n)
{
	if (n < 0 || n > 2)
	{
		hal_info("%s ERROR: n invalid (%d)\n", __FUNCTION__, n);
		num = 0;
	}
	else
		num = n;
	fd = -1;
	pdata = calloc(1, sizeof(dmx_pdata));
	P->measure = false;
	P->last_measure = 0;
	P->last_data = 0;
	P->running = false;
}

cDemux::~cDemux()
{
	hal_debug("%s #%d fd: %d\n", __FUNCTION__, num, fd);
	Close();
	free(pdata);
	pdata = NULL;
}

bool cDemux::Open(DMX_CHANNEL_TYPE pes_type, void * /*hVideoBuffer*/, int uBufferSize)
{
	P->devnum = num;
	int flags = O_RDWR;
	if (fd > -1)
		hal_info("%s FD ALREADY OPENED? fd = %d\n", __FUNCTION__, fd);
	if (pes_type == DMX_TP_CHANNEL)
	{
		/* see neutrino's src/gui/streaminfo2.cpp for the buffer size */
		if (num == 0 && uBufferSize == 3 * 3008 * 62) /* streaminfo measurement, let's cheat... */
		{
			hal_info("%s num=0 and DMX_TP_CHANNEL => measurement demux\n", __func__);
			P->devnum = 2; /* demux 0 is used for live, demux 1 for recording */
			P->measure = true;
			P->last_measure = 0;
			P->last_data = 0;
			flags |= O_NONBLOCK;
		}
		else
		{
			/* it looks like the drivers can only do one TS at a time */
			if (dmx_tp_count >= MAX_TS_COUNT)
			{
				hal_info("%s too many DMX_TP_CHANNEL requests :-(\n", __FUNCTION__);
				dmx_type = DMX_INVALID;
				fd = -1;
				return false;
			}
			dmx_tp_count++;
			P->devnum = dmx_tp_count;
		}
	}
	fd = open(devname[P->devnum], flags);
	if (fd < 0)
	{
		hal_info("%s %s: %m\n", __FUNCTION__, devname[P->devnum]);
		return false;
	}
	fcntl(fd, F_SETFD, FD_CLOEXEC);
	hal_debug("%s #%d pes_type: %s(%d), uBufferSize: %d dev:%s fd: %d\n", __func__,
		 num, DMX_T[pes_type], pes_type, uBufferSize, devname[P->devnum] + strlen("/dev/stb/"), fd);

	dmx_type = pes_type;

	if (!pesfds.empty())
	{
		hal_info("%s ERROR! pesfds not empty!\n", __FUNCTION__); /* TODO: error handling */
		return false;
	}
	if (pes_type == DMX_TP_CHANNEL)
	{
		if (P->measure)
			return true;
		struct demux_bucket_para bp;
		bp.unloader.unloader_type = UNLOADER_TYPE_TRANSPORT;
		bp.unloader.threshold     = 128;
		ioctl(fd, DEMUX_SELECT_SOURCE, INPUT_FROM_CHANNEL0);
		ioctl(fd, DEMUX_SET_BUFFER_SIZE, 230400);
		ioctl(fd, DEMUX_FILTER_BUCKET_SET, &bp);
		return true;
	}
	if (uBufferSize > 0)
	{
		/* probably uBufferSize == 0 means "use default size". TODO: find a reasonable default */
		if (ioctl(fd, DEMUX_SET_BUFFER_SIZE, uBufferSize) < 0)
			hal_info("%s DEMUX_SET_BUFFER_SIZE failed (%m)\n", __FUNCTION__);
	}
	buffersize = uBufferSize;

	return true;
}

void cDemux::Close(void)
{
	hal_debug("%s #%d, fd = %d\n", __FUNCTION__, num, fd);
	if (fd < 0)
	{
		hal_info("%s #%d: not open!\n", __FUNCTION__, num);
		return;
	}

	for (std::vector<pes_pids>::const_iterator i = pesfds.begin(); i != pesfds.end(); ++i)
	{
		hal_debug("%s stopping and closing demux fd %d pid 0x%04x\n", __FUNCTION__, (*i).fd, (*i).pid);
		if (ioctl((*i).fd, DEMUX_STOP) < 0)
			perror("DEMUX_STOP");
		if (close((*i).fd) < 0)
			perror("close");
	}
	pesfds.clear();
	ioctl(fd, DEMUX_STOP);
	close(fd);
	fd = -1;
	if (P->measure)
		return;
	if (dmx_type == DMX_TP_CHANNEL)
	{
		dmx_tp_count--;
		if (dmx_tp_count < 0)
		{
			hal_info("%s dmx_tp_count < 0!!\n", __func__);
			dmx_tp_count = 0;
		}
	}
}

bool cDemux::Start(bool)
{
	if (fd < 0)
	{
		hal_info("%s #%d: not open!\n", __FUNCTION__, num);
		return false;
	}

	for (std::vector<pes_pids>::const_iterator i = pesfds.begin(); i != pesfds.end(); ++i)
	{
		hal_debug("%s starting demux fd %d pid 0x%04x\n", __FUNCTION__, (*i).fd, (*i).pid);
		if (ioctl((*i).fd, DEMUX_START) < 0)
			perror("DEMUX_START");
	}
	ioctl(fd, DEMUX_START);
	P->running = true;
	return true;
}

bool cDemux::Stop(void)
{
	if (fd < 0)
	{
		hal_info("%s #%d: not open!\n", __FUNCTION__, num);
		return false;
	}
	for (std::vector<pes_pids>::const_iterator i = pesfds.begin(); i != pesfds.end(); ++i)
	{
		hal_debug("%s stopping demux fd %d pid 0x%04x\n", __FUNCTION__, (*i).fd, (*i).pid);
		if (ioctl((*i).fd, DEMUX_STOP) < 0)
			perror("DEMUX_STOP");
	}
	ioctl(fd, DEMUX_STOP);
	P->running = false;
	return true;
}

int cDemux::Read(unsigned char *buff, int len, int timeout)
{
#if 0
	if (len != 4095 && timeout != 10)
		fprintf(stderr, "cDemux::%s #%d fd: %d type: %s len: %d timeout: %d\n",
			__FUNCTION__, num, fd, DMX_T[dmx_type], len, timeout);
#endif
	int rc;
	struct pollfd ufds;
	ufds.fd = fd;
	ufds.events = POLLIN;
	ufds.revents = 0;

	if (dmx_type == DMX_INVALID)	/* happens, if too many DMX_TP are requested, because */
	{				/* nobody checks the return value of Open or Start... */
		hal_debug("%s #%d: DMX_INVALID\n", __func__, num);
		usleep(timeout * 1000);	/* rate-limit the debug message */
		errno = EINVAL;
		return -1;
	}

	if (P->measure)
	{
		if (timeout)
			usleep(timeout * 1000);
		uint64_t now;
		struct timespec t;
		clock_gettime(CLOCK_MONOTONIC, &t);
		now = t.tv_sec * 1000;
		now += t.tv_nsec / 1000000;
		if (now - P->last_measure < 333)
			return 0;
		unsigned char dummy[12];
		unsigned long long bit_s = 0;
		S_STREAM_MEASURE m;
		ioctl(fd, DEMUX_STOP);
		rc = read(fd, dummy, 12);
		hal_debug("%s measure read: %d\n", __func__, rc);
		if (rc == 12)
		{
			ioctl(fd, DEMUX_GET_MEASURE_TIMING, &m);
			if (m.rx_bytes > 0 && m.rx_time_us > 0)
			{
				// -- current bandwidth in kbit/sec
				// --- cast to unsigned long long so it doesn't overflow as
				// --- early, add time / 2 before division for correct rounding
				/* the correction factor is found out like that:
				   - with 8000 (guessed), a 256 kbit radio stream shows as 262kbit...
				   - 8000*256/262 = 7816.793131
				   BUT! this is only true for some Radio stations (DRS3 for example), for
				        others (DLF) 8000 does just fine.
				bit_s = (m.rx_bytes * 7816793ULL + (m.rx_time_us / 2ULL)) / m.rx_time_us;
				 */
				bit_s = (m.rx_bytes * 8000ULL + (m.rx_time_us / 2ULL)) / m.rx_time_us;
				if (now - P->last_data < 5000)
					rc = bit_s * (now - P->last_data) / 8ULL;
				else
					rc = 0;
				hal_debug("%s measure bit_s: %llu rc: %d timediff: %lld\n",
					  __func__, bit_s, rc, (now - P->last_data));
				P->last_data = now;
			} else
				rc = 0;
		}
		P->last_measure = now;
		ioctl(fd, DEMUX_START);
		return rc;
	}
	if (timeout > 0)
	{
 retry:
		rc = ::poll(&ufds, 1, timeout);
		if (!rc)
			return 0; // timeout
		else if (rc < 0)
		{
			dmx_err("poll: %s,", strerror(errno), 0)
			//hal_info("%s poll: %m\n", __FUNCTION__);
			/* happens, when running under gdb... */
			if (errno == EINTR)
				goto retry;
			return -1;
		}
		if (ufds.revents & POLLERR) /* POLLERR means buffer error, i.e. buffer overflow */
		{
			dmx_err("received %s,", "POLLERR", ufds.revents);
			/* this seems to happen sometimes at recording start, without bad effects */
			return 0;
		}
		if (ufds.revents & POLLHUP) /* we get POLLHUP if e.g. a too big DMX_BUFFER_SIZE was set */
		{
			dmx_err("received %s,", "POLLHUP", ufds.revents);
			return -1;
		}
		if (!(ufds.revents & POLLIN)) /* we requested POLLIN but did not get it? */
		{
			dmx_err("received %s, please report!", "POLLIN", ufds.revents);
			return 0;
		}
	}

	rc = ::read(fd, buff, len);
	//fprintf(stderr, "fd %d ret: %d\n", fd, rc);
	if (rc < 0)
		dmx_err("read: %s", strerror(errno), 0);

	return rc;
}

bool cDemux::sectionFilter(unsigned short _pid, const unsigned char * const filter,
			   const unsigned char * const mask, int len, int timeout,
			   const unsigned char * const negmask)
{
	int length;
	struct demux_filter_para s_flt;
	memset(&s_flt, 0, sizeof(s_flt));
	pid = _pid;
	if (len > FILTER_LENGTH - 2)
		hal_info("%s #%d: len too long: %d, FILTER_LENGTH: %d\n", __func__, num, len, FILTER_LENGTH);
	if (len < 1) /* memcpy below will be unhappy */
		hal_info("%s #%d: len too small: %d\n", __func__, num, len);

	length = (len + 2 + 1) & 0xfe;	/* reportedly, the TD drivers don't handle odd filter  */
	if (length > FILTER_LENGTH)	/* lengths well. So make sure the length is a multiple */
		length = FILTER_LENGTH;	/* of 2. The unused mask is zeroed anyway.             */
	flt = filter[0];
	s_flt.pid = pid;
	s_flt.filter_length = length;
	s_flt.filter[0] = filter[0];
	s_flt.mask[0] = mask[0];
	s_flt.timeout = timeout;
	memcpy(&s_flt.filter[3], &filter[1], len - 1);
	memcpy(&s_flt.mask[3],   &mask[1],   len - 1);
	if (negmask != NULL)
	{
		s_flt.positive[0] = negmask[0];
		memcpy(&s_flt.positive[3], &negmask[1], len - 1);
	}

	s_flt.flags = XPDF_IMMEDIATE_START;

	int to = 0;
	switch (filter[0]) {
	case 0x00: /* program_association_section */
		to = 2000;
		break;
	case 0x01: /* conditional_access_section */
		to = 6000;
		break;
	case 0x02: /* program_map_section */
		to = 1500;
		break;
	case 0x03: /* transport_stream_description_section */
		to = 10000;
		break;
	/* 0x04 - 0x3F: reserved */
	case 0x40: /* network_information_section - actual_network */
		to = 10000;
		break;
	case 0x41: /* network_information_section - other_network */
		to = 15000;
		break;
	case 0x42: /* service_description_section - actual_transport_stream */
		to = 10000;
		break;
	/* 0x43 - 0x45: reserved for future use */
	case 0x46: /* service_description_section - other_transport_stream */
		to = 10000;
		break;
	/* 0x47 - 0x49: reserved for future use */
	case 0x4A: /* bouquet_association_section */
		to = 11000;
		break;
	/* 0x4B - 0x4D: reserved for future use */
	case 0x4E: /* event_information_section - actual_transport_stream, present/following */
		to = 2000;
		break;
	case 0x4F: /* event_information_section - other_transport_stream, present/following */
		to = 10000;
		break;
	/* 0x50 - 0x5F: event_information_section - actual_transport_stream, schedule */
	/* 0x60 - 0x6F: event_information_section - other_transport_stream, schedule */
	case 0x70: /* time_date_section */
		s_flt.flags  |= (XPDF_NO_CRC); /* section has no CRC */
		//s_flt.pid     = 0x0014;
		to = 30000;
		break;
	case 0x71: /* running_status_section */
		s_flt.flags  |= (XPDF_NO_CRC); /* section has no CRC */
		to = 0;
		break;
	case 0x72: /* stuffing_section */
		s_flt.flags  |= (XPDF_NO_CRC); /* section has no CRC */
		to = 0;
		break;
	case 0x73: /* time_offset_section */
		//s_flt.pid     = 0x0014;
		to = 30000;
		break;
	/* 0x74 - 0x7D: reserved for future use */
	case 0x7E: /* discontinuity_information_section */
		s_flt.flags  |= (XPDF_NO_CRC); /* section has no CRC */
		to = 0;
		break;
	case 0x7F: /* selection_information_section */
		to = 0;
		break;
	/* 0x80 - 0x8F: ca_message_section */
	/* 0x90 - 0xFE: user defined */
	/*        0xFF: reserved */
	default:
		break;
//		return -1;
	}
	if (timeout == 0)
		s_flt.timeout = to;

	hal_debug("%s #%d pid:0x%04hx fd:%d type:%s len:%d/%d to:%d flags:%x flt[0]:%02x\n", __func__, num,
		pid, fd, DMX_T[dmx_type], len,s_flt.filter_length, s_flt.timeout,s_flt.flags, s_flt.filter[0]);
#if 0
	fprintf(stderr,"filt: ");for(int i=0;i<FILTER_LENGTH;i++)fprintf(stderr,"%02hhx ",s_flt.filter[i]);fprintf(stderr,"\n");
	fprintf(stderr,"mask: ");for(int i=0;i<FILTER_LENGTH;i++)fprintf(stderr,"%02hhx ",s_flt.mask  [i]);fprintf(stderr,"\n");
	fprintf(stderr,"posi: ");for(int i=0;i<FILTER_LENGTH;i++)fprintf(stderr,"%02hhx ",s_flt.positive[i]);fprintf(stderr,"\n");
#endif
	ioctl (fd, DEMUX_STOP);
	if (ioctl(fd, DEMUX_FILTER_SET, &s_flt) < 0)
		return false;

	P->running = true;
	return true;
}

bool cDemux::pesFilter(const unsigned short _pid)
{
	demux_pes_para p_flt;
	pid = _pid;
	flt = 0;
	/* allow PID 0 for web streaming e.g.
	 * this check originally is from tuxbox cvs but I'm not sure
	 * what it is good for...
	if (pid <= 0x0001 && dmx_type != DMX_PCR_ONLY_CHANNEL)
		return false;
	 */
	if ((pid >= 0x0002 && pid <= 0x000f) || pid >= 0x1fff)
		return false;

	hal_debug("%s #%d pid: 0x%04hx fd: %d type: %s\n", __FUNCTION__, num, pid, fd, DMX_T[dmx_type]);

	if (dmx_type == DMX_TP_CHANNEL && !P->measure)
	{
		unsigned int n = pesfds.size();
		addPid(pid);
		return (n != pesfds.size());
	}
	memset(&p_flt, 0, sizeof(p_flt));
	p_flt.pid = pid;
	p_flt.output = OUT_DECODER;
	switch (dmx_type) {
	case DMX_PCR_ONLY_CHANNEL:
		p_flt.pesType = DMX_PES_PCR;
		break;
	case DMX_AUDIO_CHANNEL:
		p_flt.pesType = DMX_PES_AUDIO;
		break;
	case DMX_VIDEO_CHANNEL:
		p_flt.pesType = DMX_PES_VIDEO;
		break;
	case DMX_PES_CHANNEL:
		p_flt.unloader.unloader_type = UNLOADER_TYPE_PAYLOAD;
		if (buffersize <= 0x10000) // dvbsubtitle, instant delivery...
			p_flt.unloader.threshold = 1;
		else
			p_flt.unloader.threshold = 8; // 1k, teletext
		p_flt.pesType = DMX_PES_OTHER;
		p_flt.output  = OUT_MEMORY;
		break;
	case DMX_TP_CHANNEL:
		/* must be measure == true or we would have returned above */
		p_flt.output = OUT_MEMORY;
		p_flt.pesType = DMX_PES_OTHER;
		p_flt.unloader.threshold = 1;
		p_flt.unloader.unloader_type = UNLOADER_TYPE_MEASURE_DUMMY;
		ioctl(fd, DEMUX_SET_MEASURE_TIME, 250000);
		break;
	default:
		p_flt.pesType = DMX_PES_OTHER;
	}
	return (ioctl(fd, DEMUX_FILTER_PES_SET, &p_flt) >= 0);
}

void cDemux::SetSyncMode(AVSYNC_TYPE /*mode*/)
{
	hal_debug("%s #%d\n", __FUNCTION__, num);
}

void *cDemux::getBuffer()
{
	hal_debug("%s #%d\n", __FUNCTION__, num);
	if (P->running)
		return (void *)1;
	return NULL;
}

void *cDemux::getChannel()
{
	hal_debug("%s #%d\n", __FUNCTION__, num);
	return NULL;
}

bool cDemux::addPid(unsigned short Pid)
{
	pes_pids pfd;
	int ret;
	struct demux_pes_para p;
	if (dmx_type != DMX_TP_CHANNEL)
	{
		hal_info("%s pes_type %s not implemented yet! pid=%hx\n", __FUNCTION__, DMX_T[dmx_type], Pid);
		return false;
	}
	if (P->measure)
	{
		hal_info("%s measurement demux -> skipping\n", __func__);
		return true;
	}
	if (fd == -1)
		hal_info("%s bucketfd not yet opened? pid=%hx\n", __FUNCTION__, Pid);
	pfd.fd = open(devname[P->devnum], O_RDWR);
	if (pfd.fd < 0)
	{
		hal_info("%s #%d Pid = %hx open failed (%m)\n", __FUNCTION__, num, Pid);
		return false;
	}
	fcntl(pfd.fd, F_SETFD, FD_CLOEXEC);
	hal_debug("%s #%d Pid = %hx pfd = %d\n", __FUNCTION__, num, Pid, pfd.fd);

	p.pid = Pid;
	p.pesType = DMX_PES_OTHER;
	p.output  = OUT_NOTHING;
	p.flags   = 0;
	p.unloader.unloader_type = UNLOADER_TYPE_BUCKET;
	p.unloader.threshold     = 128;

	ioctl(pfd.fd, DEMUX_SELECT_SOURCE, INPUT_FROM_CHANNEL0);
	ret = ioctl(pfd.fd, DEMUX_SET_BUFFER_SIZE, 0x10000); // 64k
	if (ret == -1)
		perror("DEMUX_SET_BUFFER_SIZE");
	else
	{
		ret = ioctl(pfd.fd, DEMUX_FILTER_PES_SET, &p);
		if (ret == -1)
			perror("DEMUX_FILTER_PES_SET");
	}
	pfd.pid = Pid;
	if (ret != -1)
		/* success! */
		pesfds.push_back(pfd);
	else
		/* error! */
		close(pfd.fd);
	return (ret != -1);
}

void cDemux::removePid(unsigned short Pid)
{
	if (dmx_type != DMX_TP_CHANNEL)
	{
		hal_info("%s pes_type %s not implemented yet! pid=%hx\n", __FUNCTION__, DMX_T[dmx_type], Pid);
		return;
	}
	for (std::vector<pes_pids>::iterator i = pesfds.begin(); i != pesfds.end(); ++i)
	{
		if ((*i).pid == Pid) {
			hal_debug("removePid: removing demux fd %d pid 0x%04x\n", (*i).fd, Pid);
			if (ioctl((*i).fd, DEMUX_STOP) < 0)
				perror("DEMUX_STOP");
			if (close((*i).fd) < 0)
				perror("close");
			pesfds.erase(i);
			return; /* TODO: what if the same PID is there multiple times */
		}
	}
	hal_info("%s pid 0x%04x not found\n", __FUNCTION__, Pid);
}

void cDemux::getSTC(int64_t * STC)
{
	hal_debug("%s #%d\n", __FUNCTION__, num);
	/* this is a guess, but seems to work... int32_t gives errno 515... */
#define STC_TYPE uint64_t
	STC_TYPE stc;
	if (ioctl(fd, DEMUX_GET_CURRENT_STC, &stc))
		perror("cDemux::getSTC DEMUX_GET_CURRENT_STC");
	*STC = (stc >> 32);
}

int cDemux::getUnit(void)
{
	hal_debug("%s #%d\n", __FUNCTION__, num);
	/* just guessed that this is the right thing to do.
	   right now this is only used by the CA code which is stubbed out
	   anyway */
	return num;
}
