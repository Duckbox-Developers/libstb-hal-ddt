/* public header file */

#ifndef __AUDIO_LIB_H__
#define __AUDIO_LIB_H__

#include <stdint.h>
#include <OpenThreads/Thread>
#include "cs_types.h"

typedef enum
{
	AUDIO_SYNC_WITH_PTS,
	AUDIO_NO_SYNC,
	AUDIO_SYNC_AUDIO_MASTER
} AUDIO_SYNC_MODE;

typedef enum {
	HDMI_ENCODED_OFF,
	HDMI_ENCODED_AUTO,
	HDMI_ENCODED_FORCED
} HDMI_ENCODED_MODE;

typedef enum
{
	AUDIO_FMT_AUTO = 0,
	AUDIO_FMT_MPEG,
	AUDIO_FMT_MP3,
	AUDIO_FMT_DOLBY_DIGITAL,
	AUDIO_FMT_BASIC = AUDIO_FMT_DOLBY_DIGITAL,
	AUDIO_FMT_AAC,
	AUDIO_FMT_AAC_PLUS,
	AUDIO_FMT_DD_PLUS,
	AUDIO_FMT_DTS,
	AUDIO_FMT_AVS,
	AUDIO_FMT_MLP,
	AUDIO_FMT_WMA,
	AUDIO_FMT_MPG1, // TD only. For Movieplayer / cPlayback
	AUDIO_FMT_ADVANCED = AUDIO_FMT_MLP
} AUDIO_FORMAT;

class cAudio : public OpenThreads::Thread
{
	friend class cPlayback;
	private:
		int fd;
		bool Muted;

		int clipfd; /* for pcm playback */
		int mixer_fd;  /* if we are using the OSS mixer */
		int mixer_num; /* oss mixer to use, if any */

		AUDIO_FORMAT	StreamType;
		AUDIO_SYNC_MODE    SyncMode;
		bool started;
		bool thread_started;

		int volume;
		int64_t curr_pts;

		void openDevice(void);
		void closeDevice(void);

		int do_mute(bool enable, bool remember);
		void setBypassMode(bool disable);
		void run();

	public:
		/* construct & destruct */
		cAudio(void *, void *, void *);
		~cAudio(void);
		int64_t getPts() { return curr_pts; }

		void *GetHandle() { return NULL; };
		/* shut up */
		int mute(bool remember = true) { return do_mute(true, remember); };
		int unmute(bool remember = true) { return do_mute(false, remember); };

		/* volume, min = 0, max = 255 */
		int setVolume(unsigned int left, unsigned int right);
		int getVolume(void) { return volume;}
		bool getMuteStatus(void) { return Muted; };

		/* start and stop audio */
		int Start(void);
		int Stop(void);
		bool Pause(bool Pcm = true);
		void SetStreamType(AUDIO_FORMAT type);
		void SetSyncMode(AVSYNC_TYPE Mode);

		/* select channels */
		int setChannel(int channel);
		int PrepareClipPlay(int uNoOfChannels, int uSampleRate, int uBitsPerSample, int bLittleEndian);
		int WriteClip(unsigned char * buffer, int size);
		int StopClip();
		void getAudioInfo(int &type, int &layer, int& freq, int &bitrate, int &mode);
		void SetSRS(int iq_enable, int nmgr_enable, int iq_mode, int iq_level);
		bool IsHdmiDDSupported();
		void SetHdmiDD(bool enable);
		void SetSpdifDD(bool enable);
		void ScheduleMute(bool On);
		void EnableAnalogOut(bool enable);
		int my_read(uint8_t *buf, int buf_size);
};

#endif // __AUDIO_LIB_H__
