#include <pspkernel.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspdisplay.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <pspaudiolib.h>
#include "multimp3player.h"

#define FALSE 0
#define TRUE !FALSE
#define OUTPUT_BUFFER_SIZE	2048

u8 *data[8];
long size[8];
long samplesInOutput[8] = {0,0,0,0,0,0,0,0};
struct mad_stream Stream[8];
struct mad_frame Frame[8];
struct mad_synth Synth[8];
signed short OutputBuffer[8][OUTPUT_BUFFER_SIZE];
int i;
static int isPlaying[8];
static int eos[8];
int initialized[8]={0,0,0,0,0,0,0,0};
int numStream=0;

static signed short MadFixedToSshort(mad_fixed_t Fixed)
{
	if (Fixed >= MAD_F_ONE)
	return (SHRT_MAX);
	if (Fixed <= -MAD_F_ONE)
	return (-SHRT_MAX);
	Fixed = Fixed >> (MAD_F_FRACBITS - 15);
	return ((signed short) Fixed);
}

static void audioCallback(void *_buf2, unsigned int numSamples, void *pdata,int channel)
{
	short *_buf = (short *)_buf2;
	unsigned long samplesOut = 0;
	if (isPlaying[channel] == TRUE) 
	{
		if (samplesInOutput[channel] > 0) 
		{
			if (samplesInOutput[channel] > numSamples) 
			{
				memcpy((char *) _buf, (char *) OutputBuffer[channel], numSamples * 2 * 2);
				samplesOut = numSamples;
			samplesInOutput[channel] -= numSamples;
			}
			else
			{
				memcpy((char *) _buf, (char *) OutputBuffer[channel], samplesInOutput[channel] * 2 * 2);
				samplesOut = samplesInOutput[channel];
				samplesInOutput[channel] = 0;
			}
		}
		while (samplesOut < numSamples)
		{
			if (Stream[channel].buffer == NULL || Stream[channel].error == MAD_ERROR_BUFLEN) 
			{
				mad_stream_buffer(&Stream[channel], data[channel], size[channel]);
				Stream[channel].error = 0;
			}
			if (mad_frame_decode(&Frame[channel], &Stream[channel]))
			{
				if (MAD_RECOVERABLE(Stream[channel].error))
				{
					return;
				}
				else if (Stream[channel].error == MAD_ERROR_BUFLEN) 
				{
					eos[channel] = 1;
					isPlaying[channel]=FALSE;
					return;
				}
				else
				{
					MP3Stop();
				}
			}
			mad_synth_frame(&Synth[channel], &Frame[channel]);
			for (i = 0; i < Synth[channel].pcm.length; i++)
			{
				signed short Sample;
				if (samplesOut < numSamples) 
				{
					Sample = MadFixedToSshort(Synth[channel].pcm.samples[0][i]);
					_buf[samplesOut * 2] = Sample;
					if (MAD_NCHANNELS(&Frame[channel].header) == 2) Sample = MadFixedToSshort(Synth[channel].pcm.samples[1][i]);
					_buf[samplesOut * 2 + 1] = Sample;
					samplesOut++;
				}
				else
				{
					Sample = MadFixedToSshort(Synth[channel].pcm.samples[0][i]);
					OutputBuffer[channel][samplesInOutput[channel] * 2] = Sample;
					if (MAD_NCHANNELS(&Frame[channel].header) == 2) Sample = MadFixedToSshort(Synth[channel].pcm.samples[1][i]);
					OutputBuffer[channel][samplesInOutput[channel] * 2 + 1] = Sample;
					samplesInOutput[channel]++;
				}
			}
		}
	}
	else
	{
		int count;
		for (count = 0; count < numSamples * 2; count++)
		*(_buf + count) = 0;
	}
}

void audioCallback0(void *_buf2, unsigned int numSamples, void *pdata) {audioCallback(_buf2,numSamples,pdata,0);}
void audioCallback1(void *_buf2, unsigned int numSamples, void *pdata) {audioCallback(_buf2,numSamples,pdata,1);} 
void audioCallback2(void *_buf2, unsigned int numSamples, void *pdata) {audioCallback(_buf2,numSamples,pdata,2);}
void audioCallback3(void *_buf2, unsigned int numSamples, void *pdata) {audioCallback(_buf2,numSamples,pdata,3);}
void audioCallback4(void *_buf2, unsigned int numSamples, void *pdata) {audioCallback(_buf2,numSamples,pdata,4);}
void audioCallback5(void *_buf2, unsigned int numSamples, void *pdata) {audioCallback(_buf2,numSamples,pdata,5);}
void audioCallback6(void *_buf2, unsigned int numSamples, void *pdata) {audioCallback(_buf2,numSamples,pdata,6);}
void audioCallback7(void *_buf2, unsigned int numSamples, void *pdata) {audioCallback(_buf2,numSamples,pdata,7);}

void MP3FreeTune()
{
	if (!initialized[numStream]) return;
	if (data[numStream])
	free(data[numStream]);
	mad_synth_finish(&Synth[numStream]);
	mad_frame_finish(&Frame[numStream]);
	mad_stream_finish(&Stream[numStream]);
}

void MP3End()
{
	if (!initialized[numStream]) return;
	MP3Stop();
	pspAudioSetChannelCallback(numStream, 0,0);
	MP3FreeTune();
}

int MP3Load(char *filename,int channel)
{
	if (channel>7) return FALSE;
	int f;
	eos[channel] = 0;
	if ((f = sceIoOpen(filename, PSP_O_RDONLY, 0777)) > 0)
	{
		size[channel] = sceIoLseek(f, 0, PSP_SEEK_END);
		sceIoLseek(f, 0, PSP_SEEK_SET);
		data[channel] = (unsigned char *) malloc(size[channel] + 8);
		memset(data[channel], 0, size[channel] + 8);
		if (data[channel] != 0) 
		{
			sceIoRead(f, data[channel], size[channel]);
		}
		else
		{
			sceIoClose(f);
			return 0;
		}
		sceIoClose(f);
	}
	else
	{
		return 0;
	}
	isPlaying[channel] = FALSE;
	mad_stream_init(&Stream[channel]);
	mad_frame_init(&Frame[channel]);
	mad_synth_init(&Synth[channel]);
	initialized[channel]=TRUE;
	switch(channel)
	{
	case 0:
		pspAudioSetChannelCallback(0,audioCallback0,0);
		break;
	case 1:
		pspAudioSetChannelCallback(1,audioCallback1,0);
		break;
	case 2:
		pspAudioSetChannelCallback(2,audioCallback2,0);
		break;
	case 3:
		pspAudioSetChannelCallback(3,audioCallback3,0);
		break;
	case 4:
		pspAudioSetChannelCallback(4,audioCallback4,0);
		break;
	case 5:
		pspAudioSetChannelCallback(5,audioCallback5,0);
		break;
	case 6:
		pspAudioSetChannelCallback(6,audioCallback6,0);
		break;
	case 7:
		pspAudioSetChannelCallback(7,audioCallback7,0);
		break;
	}
   return TRUE;
}

int MP3Play()
{
	if (!initialized[numStream]) return FALSE;
	isPlaying[numStream] = TRUE;
	return TRUE;
}

void MP3Pause()
{
	isPlaying[numStream] = !isPlaying[numStream];
}

int MP3Stop()
{
	if (!initialized[numStream]) return FALSE;
	isPlaying[numStream] = FALSE;
	memset(OutputBuffer[numStream], 0, OUTPUT_BUFFER_SIZE);
	mad_stream_buffer(&Stream[numStream], (void*)0, 0);
	samplesInOutput[numStream]=0;
	return TRUE;
}

int MP3EndOfStream()
{
	if (!initialized[numStream]) return FALSE;
	return eos[numStream];
}

void MP3SetStream(int n)
{
	numStream=n;
}
