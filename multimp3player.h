#ifndef _MULTIMP3PLAYER_H_
#define _MULTIMP3PLAYER_H_

#include <mad.h>

#ifdef __cplusplus
extern "C" {
#endif

int MP3Play();
void MP3Pause();
int MP3Stop();
void MP3End();
void MP3FreeTune();
int MP3Load(char *filename,int channel);
void MP3GetTimeString(char *dest);
int MP3EndOfStream();
void MP3SetStream(int s);

#ifdef __cplusplus
}
#endif
#endif
