#ifndef WRITER_H_
#define WRITER_H_

#include <sys/uio.h>
#include <stdio.h>
#include <stdint.h>
#include "common.h"

typedef enum { eNone, eAudio, eVideo} eWriterType_t;
typedef ssize_t (* WriteV_t)(int, const struct iovec *, int);

typedef struct
{
	int                    fd;
	uint8_t               *data;
	unsigned int           len;
	uint64_t               Pts;
	unsigned long long int Dts;
	uint8_t               *private_data;
	unsigned int           private_size;
	unsigned int           FrameRate;
	unsigned int           FrameScale;
	unsigned int           Width;
	unsigned int           Height;
	unsigned char          Version;
	unsigned int           InfoFlags;
	WriteV_t               WriteV;
} WriterAVCallData_t;



typedef struct WriterCaps_s
{
	char          *name;
	eWriterType_t  type;
	char          *textEncoding;
	/* fixme: revise if this is an enum! */
	int            dvbEncoding;   /* For sh4    */
	int            dvbStreamType; /* For mipsel */
	int            dvbCodecType;  /* For mipsel */
} WriterCaps_t;

typedef struct Writer_s
{
	int (* reset)();
	int (* writeData)(WriterAVCallData_t *);
	WriterCaps_t *caps;
} Writer_t;

extern Writer_t WriterAudioLPCM;
extern Writer_t WriterAudioIPCM;
extern Writer_t WriterAudioPCM;
extern Writer_t WriterAudioMP3;
extern Writer_t WriterAudioMPEGL3;
extern Writer_t WriterAudioAC3;
extern Writer_t WriterAudioEAC3;
extern Writer_t WriterAudioAAC;
extern Writer_t WriterAudioAACLATM;
extern Writer_t WriterAudioAACPLUS;
extern Writer_t WriterAudioDTS;
extern Writer_t WriterAudioWMA;
extern Writer_t WriterAudioWMAPRO;
extern Writer_t WriterAudioFLAC;
extern Writer_t WriterAudioVORBIS;
extern Writer_t WriterAudioAMR;

extern Writer_t WriterVideoMPEG1;
extern Writer_t WriterVideoMPEG2;
extern Writer_t WriterVideoMPEG4;
extern Writer_t WriterVideoMPEGH264;
extern Writer_t WriterVideoH264;
extern Writer_t WriterVideoDIVX3;
extern Writer_t WriterVideoWMV;
extern Writer_t WriterVideoDIVX;
extern Writer_t WriterVideoFOURCC;
extern Writer_t WriterVideoMSCOMP;
extern Writer_t WriterVideoH263;
extern Writer_t WriterVideoH265;
extern Writer_t WriterVideoFLV;
extern Writer_t WriterVideoVC1;
extern Writer_t WriterVideoVP6;
extern Writer_t WriterVideoVP8;
extern Writer_t WriterVideoVP9;
extern Writer_t WriterVideoMJPEG;
extern Writer_t WriterFramebuffer;
extern Writer_t WriterPipe;

Writer_t *getWriter(char *encoding);

Writer_t *getDefaultVideoWriter();
Writer_t *getDefaultAudioWriter();
ssize_t write_with_retry(int fd, const void *buf, int size);
ssize_t writev_with_retry(int fd, const struct iovec *iov, int ic);

ssize_t WriteWithRetry(Context_t *context, int pipefd, int fd, void *pDVBMtx, const void *buf, int size);
void FlushPipe(int pipefd);

ssize_t WriteExt(WriteV_t _call, int fd, void *data, size_t size);
#endif
