#ifndef __VIDEO_LIB_H__
#define __VIDEO_LIB_H__

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>
#include <vector>
#include <linux/dvb/video.h>
#include "cs_types.h"
#include "dmx_hal.h"
extern "C" {
#include <libavutil/rational.h>
}

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


#define VDEC_MAXBUFS 0x40
class cVideo : public OpenThreads::Thread
{
	friend class GLFbPC;
	friend class cDemux;
	private:
		/* called from GL thread */
		class SWFramebuffer : public std::vector<unsigned char>
		{
		public:
			SWFramebuffer() : mWidth(0), mHeight(0) {}
			void width(int w) { mWidth = w; }
			void height(int h) { mHeight = h; }
			void pts(uint64_t p) { mPts = p; }
			void AR(AVRational a) { mAR = a; }
			int width() const { return mWidth; }
			int height() const { return mHeight; }
			int64_t pts() const { return mPts; }
			AVRational AR() const { return mAR; }
		private:
			int mWidth;
			int mHeight;
			int64_t mPts;
			AVRational mAR;
		};
		int buf_in, buf_out, buf_num;
		int64_t GetPTS(void);
	public:
		/* constructor & destructor */
		cVideo(int mode, void *, void *, unsigned int unit = 0);
		~cVideo(void);

		void * GetTVEnc() { return NULL; };
		void * GetTVEncSD() { return NULL; };

		/* aspect ratio */
		int getAspectRatio(void);
		int setAspectRatio(int aspect, int mode);
		void getPictureInfo(int &width, int &height, int &rate);

		/* cropping mode */
		int setCroppingMode(int x = 0 /*vidDispMode_t x = VID_DISPMODE_NORM*/);

		/* get play state */
		int getPlayState(void);

		/* blank on freeze */
		int getBlank(void);
		int setBlank(int enable);

		/* set video_system */
		int SetVideoSystem(int video_system, bool remember = true);
		int GetVideoSystem();

		/* change video play state. Parameters are all unused. */
		int Start(void *PcrChannel = NULL, unsigned short PcrPid = 0, unsigned short VideoPid = 0, void *x = NULL);
		int Stop(bool blank = true);
		bool Pause(void);

		int SetStreamType(VIDEO_FORMAT type);
		void ShowPicture(const char * fname);

		void SetSyncMode(AVSYNC_TYPE mode);
		bool SetCECMode(VIDEO_HDMI_CEC_MODE) { return true; };
		void SetCECAutoView(bool) { return; };
		void SetCECAutoStandby(bool) { return; };
		void StopPicture();
		void Standby(unsigned int bOn);
		void Pig(int x, int y, int w, int h, int osd_w = 1064, int osd_h = 600, int startx = 0, int starty = 0, int endx = 1279, int endy = 719);
		void SetControl(int, int) { return; };
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
		bool GetScreenImage(unsigned char * &data, int &xres, int &yres, bool get_video = true, bool get_osd = false, bool scale_to_video = false);
		SWFramebuffer *getDecBuf(void);
	private:
		void run();
		SWFramebuffer buffers[VDEC_MAXBUFS];
		int dec_w, dec_h;
		int dec_r;
		bool w_h_changed;
		bool thread_running;
		VIDEO_FORMAT v_format;
		VIDEO_STD v_std;
		OpenThreads::Mutex buf_m;
		DISPLAY_AR display_aspect;
		DISPLAY_AR_MODE display_crop;
		int output_h;
		int pig_x;
		int pig_y;
		int pig_w;
		int pig_h;
		bool pig_changed;
		OpenThreads::Mutex still_m;
		bool stillpicture;
};

#endif // __VIDEO_LIB_H__
