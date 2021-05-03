#ifndef __VIDEO_LIB_H__
#define __VIDEO_LIB_H__

#include <linux/dvb/video.h>
#include "cs_types.h"
#include "dmx_hal.h"

typedef struct cs_vs_format_t
{
	char format[16];
} cs_vs_format_struct_t;

typedef enum {
	ANALOG_SD_RGB_CINCH = 0x00,
	ANALOG_SD_YPRPB_CINCH,
	ANALOG_HD_RGB_CINCH,
	ANALOG_HD_YPRPB_CINCH,
	ANALOG_SD_RGB_SCART = 0x10,
	ANALOG_SD_YPRPB_SCART,
	ANALOG_HD_RGB_SCART,
	ANALOG_HD_YPRPB_SCART,
	ANALOG_SCART_MASK = 0x10
} analog_mode_t;

typedef enum {
	COLORFORMAT_RGB = 0x10, // keep compatible with analog_mode_t
	COLORFORMAT_YUV,
	COLORFORMAT_CVBS,
	COLORFORMAT_SVIDEO,
	COLORFORMAT_HDMI_RGB,
	COLORFORMAT_HDMI_YCBCR444,
	COLORFORMAT_HDMI_YCBCR422
} COLOR_FORMAT;

typedef enum {
	VIDEO_FORMAT_MPEG2 = 0,
	VIDEO_FORMAT_MPEG4_H264,
	VIDEO_FORMAT_VC1,
	VIDEO_FORMAT_JPEG,
	VIDEO_FORMAT_GIF,
	VIDEO_FORMAT_PNG,
	VIDEO_FORMAT_MPEG4_H265,
	VIDEO_FORMAT_AVS = 16
} VIDEO_FORMAT;

typedef enum {
	VIDEO_SD = 0,
	VIDEO_HD,
	VIDEO_120x60i,
	VIDEO_320x240i,
	VIDEO_1440x800i,
	VIDEO_360x288i
} VIDEO_DEFINITION;

typedef enum {
	VIDEO_FRAME_RATE_23_976 = 0,
	VIDEO_FRAME_RATE_24,
	VIDEO_FRAME_RATE_25,
	VIDEO_FRAME_RATE_29_97,
	VIDEO_FRAME_RATE_30,
	VIDEO_FRAME_RATE_50,
	VIDEO_FRAME_RATE_59_94,
	VIDEO_FRAME_RATE_60
} VIDEO_FRAME_RATE;

typedef enum {
	DISPLAY_AR_1_1,
	DISPLAY_AR_4_3,
	DISPLAY_AR_14_9,
	DISPLAY_AR_16_9,
	DISPLAY_AR_20_9,
	DISPLAY_AR_RAW
} DISPLAY_AR;

typedef enum {
	DISPLAY_AR_MODE_PANSCAN = 0,
	DISPLAY_AR_MODE_LETTERBOX,
	DISPLAY_AR_MODE_NONE,
	DISPLAY_AR_MODE_PANSCAN2
} DISPLAY_AR_MODE;

typedef enum {
	VIDEO_DB_DR_NEITHER = 0,
	VIDEO_DB_ON,
	VIDEO_DB_DR_BOTH
} VIDEO_DB_DR;

typedef enum {
	VIDEO_PLAY_STILL = 0,
	VIDEO_PLAY_CLIP,
	VIDEO_PLAY_TRICK,
	VIDEO_PLAY_MOTION,
	VIDEO_PLAY_MOTION_NO_SYNC
} VIDEO_PLAY_MODE;

typedef enum {
	VIDEO_STD_NTSC,
	VIDEO_STD_SECAM,
	VIDEO_STD_PAL,
	VIDEO_STD_480P,
	VIDEO_STD_576P,
	VIDEO_STD_720P60,
	VIDEO_STD_1080I60,
	VIDEO_STD_720P50,
	VIDEO_STD_1080I50,
	VIDEO_STD_1080P30,
	VIDEO_STD_1080P24,
	VIDEO_STD_1080P25,
	VIDEO_STD_AUTO,
	VIDEO_STD_1080P50,	/* SPARK only */
	VIDEO_STD_MAX
} VIDEO_STD;

/* not used, for dummy functions */
typedef enum {
	VIDEO_HDMI_CEC_MODE_OFF = 0,
	VIDEO_HDMI_CEC_MODE_TUNER,
	VIDEO_HDMI_CEC_MODE_RECORDER
} VIDEO_HDMI_CEC_MODE;

typedef enum
{
	VIDEO_CONTROL_BRIGHTNESS = 0,
	VIDEO_CONTROL_CONTRAST,
	VIDEO_CONTROL_SATURATION,
	VIDEO_CONTROL_HUE,
	VIDEO_CONTROL_SHARPNESS,
	VIDEO_CONTROL_MAX = VIDEO_CONTROL_SHARPNESS
} VIDEO_CONTROL;

class cVideo
{
	friend class cPlayback;
	friend class cDemux;
	private:
		/* video device */
		int fd;
		unsigned int devnum;
		/* apparently we cannot query the driver's state
		   => remember it */
		video_play_state_t playstate;
		int /*vidDispMode_t*/ croppingMode;
		int /*vidOutFmt_t*/ outputformat;
		int scartvoltage;

		VIDEO_FORMAT StreamType;
		VIDEO_DEFINITION VideoDefinition;
		DISPLAY_AR DisplayAR;
		VIDEO_PLAY_MODE SyncMode;
		DISPLAY_AR_MODE ARMode;
		VIDEO_DB_DR eDbDr;
		DISPLAY_AR PictureAR;
		VIDEO_FRAME_RATE FrameRate;
		int video_standby;
		int64_t GetPTS(void);

		int brightness, contrast, saturation, hue;

		/* used internally by playback */
		void openDevice(void);
		void closeDevice(void);
	public:
		/* constructor & destructor */
		cVideo(int mode, void *, void *, unsigned int unit = 0);
		~cVideo(void);

		void open_AVInput_Device(void) { return; };	// Dummy
		void close_AVInput_Device(void) { return; };	// Dummy

		void setAVInput(int val);

		void * GetTVEnc() { return NULL; };
		void * GetTVEncSD() { return NULL; };

		/* aspect ratio */
		int getAspectRatio(void);
		void getPictureInfo(int &width, int &height, int &rate);
		int setAspectRatio(int aspect, int mode);

		/* cropping mode */
		int setCroppingMode(int x = 0 /*vidDispMode_t x = VID_DISPMODE_NORM*/);

		/* get play state */
		int getPlayState(void);

		/* blank on freeze */
		int getBlank(void);
		int setBlank(int enable);

		/* change video play state. Parameters are all unused. */
		int Start(void *PcrChannel = NULL, unsigned short PcrPid = 0, unsigned short VideoPid = 0, void *x = NULL);
		int Stop(bool blank = true);
		bool Pause(void);

		/* get video system infos */
		int GetVideoSystem(void);
		/* when system = -1 then use current video system */
		void GetVideoSystemFormatName(cs_vs_format_t* format, int system = -1);

		/* set video_system */
		int SetVideoSystem(int video_system, bool remember = true);
		int SetStreamType(VIDEO_FORMAT type);
		void SetSyncMode(AVSYNC_TYPE mode);
		bool SetCECMode(VIDEO_HDMI_CEC_MODE) { return true; };
		void SetCECAutoView(bool) { return; };
		void SetCECAutoStandby(bool) { return; };
		bool ShowPicture(const char * fname, const char *_destname = NULL);
		void StopPicture();
		void Standby(unsigned int bOn);
		void ShowPig(int _x);
		void Pig(int x, int y, int w, int h, int osd_w = 1064, int osd_h = 600, int startx = 0, int starty = 0, int endx = 1279, int endy = 719);
		void SetControl(int, int);
		void VideoParamWatchdog(void);
		void setContrast(int val);
		void SetVideoMode(analog_mode_t mode);
		void SetDBDR(int) { return; };
		void SetAudioHandle(void *) { return; };
		void SetAutoModes(int [VIDEO_STD_MAX]) { return; };
		int  OpenVBI(int) { return 0; };
		int  CloseVBI(void) { return 0; };
		int  StartVBI(unsigned short) { return 0; };
		int  StopVBI(void) { return 0; };
		void SetDemux(cDemux *dmx);
		void SetColorFormat(COLOR_FORMAT color_format);
		bool GetScreenImage(unsigned char * &data, int &xres, int &yres, bool get_video = true, bool get_osd = false, bool scale_to_video = false);
};

#endif // __VIDEO_LIB_H__
