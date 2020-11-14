//#include <kubridge.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspaudio.h>
#include <pspaudiolib.h>
#include <psppower.h>
#include <pspiofilemgr.h>
#include <pspinit.h>
#include <pspthreadman_kernel.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <psploadexec_kernel.h>
#include <psputility.h>
#include <pspgu.h>
#include <pspsdk.h>
//#include "systemctrl.h"
//#include "systemctrl_se.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include "multimp3player.h"
#include "graphics.h"
#include "rdriver.h"

#define FALSE 0
#define TRUE 1
#define printText pspDebugScreenPrintf
#define RGB(r, g, b) ((r)|((g)<<8)|((b)<<16))
#define lineClear(a_line) pspDebugScreenSetXY(0, a_line); pspDebugScreenPuts("                                                                   "); pspDebugScreenSetXY(0, a_line);
#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4)
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2)
PSP_MODULE_INFO("Pig Races",0,1,1);
//Curl Save File
struct FtpFile {
    char *filename;
    FILE *stream;
  };

int my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
  struct FtpFile *out=(struct FtpFile *)stream;
  if(out && !out->stream) {
    /* open file for writing */
    out->stream=fopen(out->filename, "wb");
    if(!out->stream)
      return -1; /* failure, can't open file to write */
  }
  return fwrite(buffer, size, nmemb, out->stream);
}
//End
void netEnd(void)
{
	sceNetApctlTerm();
	
	sceNetInetTerm();
	
	sceNetTerm();
}
int exit_callback(int arg1, int arg2, void *common) {
	MP3SetStream(1);
	MP3Stop();
	MP3FreeTune();
	MP3SetStream(2);
	MP3Stop();
	MP3FreeTune();
	sceKernelExitGame();
    return 0;
}

int CallbackThread(SceSize args, void *argp) {
    int cbid;
    cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int SetupCallbacks(void) {
    int thid = 0;
    thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
    }
	return thid;
}
/* Graphics stuff, based on cube sample */
static unsigned int __attribute__((aligned(16))) list[262144];

struct Vertex
{
	unsigned int color;
	float x,y,z;
};
struct Vertex __attribute__((aligned(16))) vertices[12*3] =
{
		{0xff7f0000,-1,-1, 1}, // 0
    	{0xff7f0000,-1, 1, 1}, // 4
    	{0xff7f0000, 1, 1, 1}, // 5

    	{0xff7f0000,-1,-1, 1}, // 0
    	{0xff7f0000, 1, 1, 1}, // 5
    	{0xff7f0000, 1,-1, 1}, // 1

    	{0xff7f0000,-1,-1,-1}, // 3
    	{0xff7f0000, 1,-1,-1}, // 2
    	{0xff7f0000, 1, 1,-1}, // 6

    	{0xff7f0000,-1,-1,-1}, // 3
    	{0xff7f0000, 1, 1,-1}, // 6
    	{0xff7f0000,-1, 1,-1}, // 7

    	{0xff007f00, 1,-1,-1}, // 0
    	{0xff007f00, 1,-1, 1}, // 3
    	{0xff007f00, 1, 1, 1}, // 7

    	{0xff007f00, 1,-1,-1}, // 0
    	{0xff007f00, 1, 1, 1}, // 7
    	{0xff007f00, 1, 1,-1}, // 4

    	{0xff007f00,-1,-1,-1}, // 0
    	{0xff007f00,-1, 1,-1}, // 3
    	{0xff007f00,-1, 1, 1}, // 7

    	{0xff007f00,-1,-1,-1}, // 0
    	{0xff007f00,-1, 1, 1}, // 7
    	{0xff007f00,-1,-1, 1}, // 4

    	{0xff00007f,-1, 1,-1}, // 0
    	{0xff00007f, 1, 1,-1}, // 1
    	{0xff00007f, 1, 1, 1}, // 2

    	{0xff00007f,-1, 1,-1}, // 0
    	{0xff00007f, 1, 1, 1}, // 2
    	{0xff00007f,-1, 1, 1}, // 3

    	{0xff00007f,-1,-1,-1}, // 4
    	{0xff00007f,-1,-1, 1}, // 7
    	{0xff00007f, 1,-1, 1}, // 6

    	{0xff00007f,-1,-1,-1}, // 4
    	{0xff00007f, 1,-1, 1}, // 6
    	{0xff00007f, 1,-1,-1}, // 5
};
static int running = 1;
//Rand Number
static int GetRandomNum1(int lo, int hi)
{
 SceKernelUtilsMt19937Context ctx;
 sceKernelUtilsMt19937Init(&ctx, time(NULL)); //SEED TO TIME
 u32 rand_val = sceKernelUtilsMt19937UInt(&ctx);
 rand_val = lo + rand_val % hi;
 return (int)rand_val;
}
static int GetRandomNum2(int lo, int hi)
{
 SceKernelUtilsMt19937Context ctx;
 sceKernelUtilsMt19937Init(&ctx, time(NULL)); //SEED TO TIME
 u32 rand_val = sceKernelUtilsMt19937UInt(&ctx);
 rand_val = lo + rand_val % hi;
 return (int)rand_val;
}
static int GetRandomNum3(int lo, int hi)
{
 SceKernelUtilsMt19937Context ctx;
 sceKernelUtilsMt19937Init(&ctx, time(NULL)); //SEED TO TIME
 u32 rand_val = sceKernelUtilsMt19937UInt(&ctx);
 rand_val = lo + rand_val % hi;
 return (int)rand_val;
}
static int GetRandomNum4(int lo, int hi)
{
 SceKernelUtilsMt19937Context ctx;
 sceKernelUtilsMt19937Init(&ctx, time(NULL)); //SEED TO TIME
 u32 rand_val = sceKernelUtilsMt19937UInt(&ctx);
 rand_val = lo + rand_val % hi;
 return (int)rand_val;
}
//End RandNumber

int StillPlaying()
{
	MP3SetStream(1);
	if (!MP3EndOfStream()) return TRUE;
	MP3SetStream(2);
	if (!MP3EndOfStream()) return TRUE;
	return FALSE;
}

void ErrorExit(int milisecs, char *fmt, ...)
{
	va_list list;
	char msg[256];	

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	printf(msg);
	
	sceKernelDelayThread(milisecs*1000);
	sceKernelExitGame();

	asm("break\n");
	while (1);
}

int WriteFile(char *file, void *buf, int size)
{
	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	
	if (fd < 0)
	{
		return fd;
	}

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}

int Dialog(char *msg)
{
	printf("%s", msg);

	while (1)
	{
		SceCtrlData pad;

		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS)
			return 1;

		if (pad.Buttons & PSP_CTRL_RTRIGGER)
			return 0;

		sceKernelDelayThread(50000);
	}

	return -1;
}


//Global variables
int exitGame = 0;
SceCtrlData pad,lastPad;
struct SceKernelLoadExecVSHParam param;
int apitype = 0;
char *program = NULL;
char *mode = NULL;
int startMenu = 1;
int startMenuSel = 1;
int crossPushed = 0;
int menuOpen = 1;
int startMenuPig = 0;
int Pig1 = 0;
int Pig2 = 0;
int Pig3 = 0;
int Pig4 = 0;
int startRace = 0;
int raceMenu = 1;
char* charMoney = "0";
char curlSaveData[100];
char charMoneyArray[50];
char newCharMoneyArray[50];
int Money = 0;
long saveGameSize;
int i = 0;
int placeBetSubMenu = 0;
int betPig = 1;
int betAmount = 100;
int tempBetAmount = 0;
int moneyAdded = 0;
int winPig = 0;
int choosePig = 0;
char betAmoutText[100];
char betPigText[100];
char moneyText[100];
int jackpotAmount = 20000;
char jackpotText[100];
int showJackpotText = 0;
int gameLoaded = 1;
int jackpotPig = 0;
int getJackpotPig = 1;
int jackpotRound = 0;
char* charJackpot = "0";
int addJackpotMoneyAfterLoss = 0;
char *loadData;
char *tempMoney = "0";
char *tempJP = "0";
char *userAccount = "Guest";

//Colors
Color Pink = RGB(255,192,255);
Color Grey = RGB(161, 161, 161);
Color Orange = RGB(255, 140, 0);
Color startColor = RGB(161, 161, 161);
Color creditColor = RGB(161, 161, 161);
Color startRaceColor = RGB(161, 161, 161);
Color saveColor = RGB(161, 161, 161);
Color placeBetColor = RGB(161, 161, 161);
Color jackpotColor = RGB(161, 161, 161);

int trophyFileExist(char* Trophy){
	char nameTrophy[50];
	char newTrophyArray[50];
	
	sprintf(nameTrophy, "Trophy: '%s'\n", Trophy);
	
	//Encrypt data
	strncpy(newTrophyArray, nameTrophy, 50); 
	newTrophyArray[50 - 1] = 0;
	
	FILE *trophyData;
	trophyData = fopen("ms0:/PSP/GAME/PS3ForPSP/System/Saves/PigRace-UCUS84850/trophy.db", "a+");
	fwrite(newTrophyArray, 1, sizeof(newTrophyArray), trophyData);
	fclose(trophyData);
	return 0;
}
//Save URL File
int saveURLFile(char *url, char *filename, char *dataPost){
	CURL *curl;
 	CURLcode res;
 	struct FtpFile ftpfile={
 		filename, /* name to store the file as if succesful */
 		NULL
 	};

 	curl_global_init(CURL_GLOBAL_DEFAULT);

 	curl = curl_easy_init();
 	if(curl) {

     		curl_easy_setopt(curl, CURLOPT_URL, url);
     		/* Post data to login and what not haha */
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dataPost);
				/* Define our callback to get called when there's data to be written */
     		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
     		/* Set a pointer to our struct to pass to the callback */
     		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);

     		/* Switch on full protocol/debug output */
     		curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);
     		//curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);

     		res = curl_easy_perform(curl);

     		/* always cleanup */
     		curl_easy_cleanup(curl);

     		if(CURLE_OK != res) {
       			/* we failed */
       			pspDebugScreenPrintf("Error: curl told us %d\n", res);
    		}
   	}

   	if(ftpfile.stream)
     		fclose(ftpfile.stream); /* close the local file */

   	curl_global_cleanup();
		int line = 0;
		char data[80];
		FILE *saveFile;
		saveFile = fopen("saveData.prx","r");
		while (feof(saveFile) == 0){
			line++;
			fgets(data, 80, saveFile);	/* Read next record			*/
			if (data[0] == '#') continue; /* filter out the comments		*/ 	
			switch(line){
				case 1:
					userAccount = data;	
				case 2:
					tempMoney = "100";
				case 3:
					tempJP = "100";
			}
		}
		fclose(saveFile);
		remove("saveData.prx");
   	return 0;
}
//End
int postData(char *webSite, char *dataPost){
	CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if(curl) {
    /* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. */
    curl_easy_setopt(curl, CURLOPT_URL, webSite);
    /* Now specify the POST data */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dataPost);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);

    /* always cleanup */
    curl_easy_cleanup(curl);
    printf("Posted");
  }else{
  	printf("Post Failed");
  }
  return 0;
}
void *myrealloc(void *ptr, size_t size)
{
  /* There might be a realloc() out there that doesn't like reallocing
     NULL pointers, so we take care of it here */
  if(ptr)
    return realloc(ptr, size);
  else
    return malloc(size);
}
void netInit(void)
{
	sceNetInit(128*1024, 42, 4*1024, 42, 4*1024);
	
	sceNetInetInit();
	
	sceNetApctlInit(0x8000, 48);
}
int netDialog()
{
	int done = 0;

   	pspUtilityNetconfData data;

	memset(&data, 0, sizeof(data));
	data.base.size = sizeof(data);
	data.base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
	data.base.buttonSwap = PSP_UTILITY_ACCEPT_CROSS;
	data.base.graphicsThread = 17;
	data.base.accessThread = 19;
	data.base.fontThread = 18;
	data.base.soundThread = 16;
	data.action = PSP_NETCONF_ACTION_CONNECTAP;
	
	struct pspUtilityNetconfAdhoc adhocparam;
	memset(&adhocparam, 0, sizeof(adhocparam));
	data.adhocparam = &adhocparam;

	sceUtilityNetconfInitStart(&data);
	
	while(running)
	{
		switch(sceUtilityNetconfGetStatus())
		{
			case PSP_UTILITY_DIALOG_NONE:
				break;

			case PSP_UTILITY_DIALOG_VISIBLE:
				sceUtilityNetconfUpdate(1);
				break;

			case PSP_UTILITY_DIALOG_QUIT:
				sceUtilityNetconfShutdownStart();
				break;
				
			case PSP_UTILITY_DIALOG_FINISHED:
				done = 1;
				break;

			default:
				break;
		}

		sceDisplayWaitVblankStart();
		sceGuSwapBuffers();
		
		if(done)
			break;
	}
	
	return 1;
}
static void setupGu()
{
		sceGuInit();

    	sceGuStart(GU_DIRECT,list);
    	sceGuDrawBuffer(GU_PSM_8888,(void*)0,BUF_WIDTH);
    	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)0x88000,BUF_WIDTH);
    	sceGuDepthBuffer((void*)0x110000,BUF_WIDTH);
    	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
    	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
    	sceGuDepthRange(0xc350,0x2710);
    	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
    	sceGuEnable(GU_SCISSOR_TEST);
    	sceGuDepthFunc(GU_GEQUAL);
    	sceGuEnable(GU_DEPTH_TEST);
    	sceGuFrontFace(GU_CW);
    	sceGuShadeModel(GU_SMOOTH);
    	sceGuEnable(GU_CULL_FACE);
    	sceGuEnable(GU_CLIP_PLANES);
    	sceGuFinish();
    	sceGuSync(0,0);

    	sceDisplayWaitVblankStart();
    	sceGuDisplay(GU_TRUE);
}
int checkTrophies(){
	if(Money >= 500){
			if(trophyFileExist("$5000 Down") == 0){
				
			}
	}
	return 0;
}

void startGame(){
	
	//Load saved data
	/*FILE *saveData;
	saveData = fopen("ms0:/PSP/GAME/PS3ForPSP/System/Saves/PigRace-UCUS84850/SaveGame.db", "r");
	fseek(saveData, 0, SEEK_END);
	saveGameSize = ftell(saveData);
	rewind(saveData);
	charMoney = (char*)malloc(sizeof(char)*saveGameSize);
	fread(charMoney, 1, saveGameSize, saveData);
	fclose(saveData);
	
	FILE *jackpotData;
	jackpotData = fopen("ms0:/PSP/GAME/PS3ForPSP/System/Saves/PigRace-UCUS84850/SDFHFGHD3255676.db", "r");
	fseek(jackpotData, 0, SEEK_END);
	saveGameSize = ftell(jackpotData);
	rewind(jackpotData);
	charJackpot = (char*)malloc(sizeof(char)*saveGameSize);
	fread(charJackpot, 1, saveGameSize, jackpotData);
	fclose(jackpotData);*/
	
	//Convert data
	/*strncpy(newCharMoneyArray, charMoney, 50); 
	newCharMoneyArray[50 - 1] = 0; 

	for(i = 0; i < 50; ++i){
		if (newCharMoneyArray[i] == 'K'){
			newCharMoneyArray[i] = '9';
		}else if(newCharMoneyArray[i] == 'S'){
			newCharMoneyArray[i] = '8';
		}else if(newCharMoneyArray[i] == 'C'){
			newCharMoneyArray[i] = '7';
		}else if(newCharMoneyArray[i] == 'R'){
			newCharMoneyArray[i] = '6';
		}else if(newCharMoneyArray[i] == 'B'){
			newCharMoneyArray[i] = '5';
		}else if(newCharMoneyArray[i] == 'D'){
			newCharMoneyArray[i] = '4';
		}else if(newCharMoneyArray[i] == 'Q'){
			newCharMoneyArray[i] = '3';
		}else if(newCharMoneyArray[i] == 'N'){
			newCharMoneyArray[i] = '2';
		}else if(newCharMoneyArray[i] == 'U'){
			newCharMoneyArray[i] = '1';
		}else if(newCharMoneyArray[i] == 'L'){
			newCharMoneyArray[i] = '0';
		}
	}*/
	
	//Declare money and jackpot
	Money = atoi(tempMoney);
	jackpotAmount = atoi(tempJP);
	
	if(jackpotAmount > 900000000){
		jackpotAmount = 900000000;
	}
	
	//Check Trophies
	checkTrophies();
	
	//Make sure crossPushed has been reset
	crossPushed = 0;
	//Make Sure Start Menu Is Closed
	startMenu = 0;
	//Images
	Image* Theme[7];
	Theme[0] = loadImage("./Grass.png");
	Theme[1] = loadImage("./Fence.png");
	Theme[2] = loadImage("./Pig1.png");
	Theme[3] = loadImage("./Pig2.png");
	Theme[4] = loadImage("./Pig3.png");
	Theme[5] = loadImage("./Pig4.png");
	Theme[6] = loadImage("./Finish.png");

	
	while(1){
		
		//Get Jackpot Pig
		if(getJackpotPig == 1){
			jackpotPig = GetRandomNum4(1, 4);
			getJackpotPig = 0;
		}
		
		if(jackpotAmount > 900000000){
			jackpotAmount = 900000000;
		}
		//Get text values
		sprintf(betAmoutText, "Bet Amount: %d", betAmount);
		sprintf(betPigText, "Bet On Pig: %d", betPig);
		sprintf(moneyText, "You have %d dollars!", Money);
		sprintf(jackpotText, "There is %d dollars in the jackpot on pig %d.", jackpotAmount, jackpotPig);
		
		//Clear screen
		clearScreen(RGB(0,0,0));
		
		sceCtrlReadBufferPositive(&pad,1);
	
		if(pad.Buttons != lastPad.Buttons){
			if(startRace != 1){
				if(pad.Buttons&PSP_CTRL_LEFT){
					crossPushed = 0;
					raceMenu -= 1;
					if(raceMenu < 1){
						raceMenu = 1;
					}
					placeBetSubMenu = 0;
					showJackpotText = 0;
				}
					
				if(pad.Buttons&PSP_CTRL_RIGHT){
					crossPushed = 0;
					raceMenu += 1;
					if(raceMenu > 4){
						raceMenu = 4;
					}
					placeBetSubMenu = 0;
					showJackpotText = 0;
				}
					
				if(pad.Buttons&PSP_CTRL_UP){
					crossPushed = 0;
					if(placeBetSubMenu == 1){
						betPig += 1;
						if(betPig > 4){
							betPig = 4;
						}
					}else if(placeBetSubMenu == 2){
						betAmount += 1;
						if(betAmount > 250){
							betAmount = 250;
						}
					}
				}
					
				if(pad.Buttons&PSP_CTRL_DOWN){
					crossPushed = 0;
					if(placeBetSubMenu == 1){
						betPig -= 1;
						if(betPig < 1){
							betPig = 1;
						}
					}else if(placeBetSubMenu == 2){
						betAmount -= 1;
						if(betAmount < 50){
							betAmount = 50;
						}else if(betAmount > 250){
							betAmount = 250;
						}
					}
				}
					
				if(pad.Buttons&PSP_CTRL_CIRCLE){
					/*apitype = 0x141;
					program = "ms0:/PSP/GAME/PS3ForPSP/System/Addons/BuiltInGames/PigRaces/EBOOT.PBP";
					mode = "game";
					exitGame = 1;*/
				}
				if(pad.Buttons&PSP_CTRL_SQUARE){
					if((raceMenu == 4) && (showJackpotText == 1)){
						betPig = jackpotPig;
						betAmount = jackpotAmount / 2;
						Pig1 = 0;
						Pig2 = 0;
						Pig3 = 0;
						Pig4 = 0;
						jackpotRound = 1;
						startRace = 1;
					}
				}
				if(pad.Buttons & PSP_CTRL_START){
					/*apitype = 0x141;
					program = "ms0:/PSP/GAME/PS3ForPSP/EBOOT.PBP";
					mode = "game";
					exitGame = 1;*/
				}
				if(pad.Buttons&PSP_CTRL_CROSS){
					crossPushed = 1;
				}
					
				if(pad.Buttons&PSP_CTRL_LTRIGGER){
					crossPushed = 0;
					placeBetSubMenu -= 1;
					if(placeBetSubMenu < 1){
						placeBetSubMenu = 1;
					}
				}
				
				if(pad.Buttons&PSP_CTRL_RTRIGGER){
					crossPushed = 0;
					placeBetSubMenu += 1;
					if(placeBetSubMenu > 2){
						placeBetSubMenu = 2;
					}
				}
				
				lastPad = pad;
			}
		}
		
		if(Money > 999999999){
			Money = 999999999;
		}
		
		printTextScreen(2, 2, "Created By Chris9606", Orange);
		printTextScreen(2, 12, "===========================================================================", Grey);
		printTextScreen(2, 22, "{Start}", startRaceColor); 
		printTextScreen(58, 22, "{Save}", saveColor);
		printTextScreen(105, 22, "{Place Bet}", placeBetColor);
		printTextScreen(193, 22, "{Jackpot}", jackpotColor);
		printTextScreen(1, 32, moneyText, RGB( 250, 222, 34));
		//Print Images
		fillScreenRect(RGB(0, 128, 0), 0, 55, 480, 217);
		//Line 1
		drawLineScreen(4, 227, 428, 227, RGB(255, 255, 255));
		//Line 2
		drawLineScreen(4, 197, 428, 197, RGB(255, 255, 255));
		//Line 3
		drawLineScreen(4, 167, 428, 167, RGB(255, 255, 255));
		blitAlphaImageToScreen(0, 0, 40, 100, Theme[6], 430, 150);
		blitAlphaImageToScreen(0, 0, 60, 25, Theme[2], Pig1, 230);
		blitAlphaImageToScreen(0, 0, 60, 25, Theme[2], Pig2, 200);
		blitAlphaImageToScreen(0, 0, 60, 25, Theme[2], Pig3, 170);
		blitAlphaImageToScreen(0, 0, 60, 25, Theme[2], Pig4, 140);
		blitAlphaImageToScreen(0, 0, 480, 40, Theme[1], 0, 97);
		
		if(raceMenu == 1){
			startRaceColor = RGB(255, 255, 255);
			saveColor = RGB(161, 161, 161);
			placeBetColor = RGB(161, 161, 161);
			jackpotColor = RGB(161, 161, 161);
			if(crossPushed == 1){
				Pig1 = 0;
				Pig2 = 0;
				Pig3 = 0;
				Pig4 = 0;
				startRace = 1;
				crossPushed = 0;
			}
		}else if(raceMenu == 2){
			startRaceColor = RGB(161, 161, 161);
			saveColor = RGB(255, 255, 255);
			placeBetColor = RGB(161, 161, 161);
			jackpotColor = RGB(161, 161, 161);
			if(crossPushed == 1){
				sprintf(curlSaveData, "user=chris9606&score=%d&jackpot=%d", jackpotAmount, Money);
				postData("http://72.205.227.156:65000/server/pigPost.php", curlSaveData);
				/*sprintf(charMoneyArray, "%d", Money);
				
				//Encrypt data
				strncpy(newCharMoneyArray, charMoneyArray, 50); 
				newCharMoneyArray[50 - 1] = 0; 

				for(i = 0; i < 50; ++i){
					if (newCharMoneyArray[i] == '9'){
						newCharMoneyArray[i] = 'K';
					}else if(newCharMoneyArray[i] == '8'){
						newCharMoneyArray[i] = 'S';
					}else if(newCharMoneyArray[i] == '7'){
						newCharMoneyArray[i] = 'C';
					}else if(newCharMoneyArray[i] == '6'){
						newCharMoneyArray[i] = 'R';
					}else if(newCharMoneyArray[i] == '5'){
						newCharMoneyArray[i] = 'B';
					}else if(newCharMoneyArray[i] == '4'){
						newCharMoneyArray[i] = 'D';
					}else if(newCharMoneyArray[i] == '3'){
						newCharMoneyArray[i] = 'Q';
					}else if(newCharMoneyArray[i] == '2'){
						newCharMoneyArray[i] = 'N';
					}else if(newCharMoneyArray[i] == '1'){
						newCharMoneyArray[i] = 'U';
					}else if(newCharMoneyArray[i] == '0'){
						newCharMoneyArray[i] = 'L';
					}
				}*/
				
				//FILE *saveGameData;
				//saveGameData = fopen("ms0:/PSP/GAME/PS3ForPSP/System/Saves/PigRace-UCUS84850/SaveGame.db" , "wb");
				//fwrite(loadData, 1, sizeof(loadData), saveGameData);
				//fclose(saveGameData);
				//free(curlSaveData);
				crossPushed = 0;
			}
		}else if(raceMenu == 3){
			startRaceColor = RGB(161, 161, 161);
			saveColor = RGB(161, 161, 161);
			placeBetColor = RGB(255, 255, 255);
			jackpotColor = RGB(161, 161, 161);
			if(crossPushed == 1){
				placeBetSubMenu = 1;
				crossPushed = 0;
			}
			if(placeBetSubMenu == 1){
				printTextScreen(2, 42, betPigText, RGB(240, 0, 15));
				printTextScreen(110, 42, betAmoutText, RGB(161, 161, 161));
			}else if(placeBetSubMenu == 2){
				printTextScreen(2, 42, betPigText, RGB(161, 161, 161));
				printTextScreen(110, 42, betAmoutText, RGB(240, 0, 15));
			}
		}else if(raceMenu == 4){
			startRaceColor = RGB(161, 161, 161);
			saveColor = RGB(161, 161, 161);
			placeBetColor = RGB(161, 161, 161);
			jackpotColor = RGB(255, 255, 255);
			if(crossPushed){
				showJackpotText = 1;
				crossPushed = 0;
			}
			if(showJackpotText == 1){
				printTextScreen(2, 42, jackpotText, RGB(240, 0, 15));
			}
		}
		
		if(startRace == 1){
			choosePig = betPig;
			moneyAdded = 2;
			addJackpotMoneyAfterLoss = 2;
			gameLoaded = 0;
			getJackpotPig = 1;
			raceMenu = 1;
			Pig1 = Pig1 + GetRandomNum1(-1, 5);
			Pig2 = Pig2 + GetRandomNum2(-1, 6);
			Pig3 = Pig3 + GetRandomNum3(0, 4);
			Pig4 = Pig4 + GetRandomNum4(-2, 7);
		}
		
		//Display winning text
		if(Pig1 >= 430){
			printTextScreen(50, 62, "Pig One Wins!", Pink);
			startRace = 0;
			if(moneyAdded == 2){
				moneyAdded = 1;
			}
			if(moneyAdded == 1){
				winPig = 1;
				if(betPig == 1){
					tempBetAmount = betAmount * 2;
					Money += tempBetAmount;
					moneyAdded = 0;
				}else{
					Money -= betAmount;
					moneyAdded = 0;
				}
			}
		}else if(Pig2 >= 430){
			printTextScreen(50, 62, "Pig Two Wins!", Pink);
			startRace = 0;
			if(moneyAdded == 2){
				moneyAdded = 1;
			}
			if(moneyAdded != 0){
				winPig = 2;
				if(betPig == 2){
					tempBetAmount = betAmount * 2;
					Money += tempBetAmount;
					moneyAdded = 0;
				}else{
					Money -= betAmount;
					moneyAdded = 0;
				}
			}
		}else if(Pig3 >= 430){
			printTextScreen(50, 62, "Pig Three Wins!", Pink);
			startRace = 0;
			if(moneyAdded == 2){
				moneyAdded = 1;
			}
			if(moneyAdded == 1){
				winPig = 3;
				if(betPig == 3){
					tempBetAmount = betAmount * 2;
					Money += tempBetAmount;
					moneyAdded = 0;
				}else{
					Money -= betAmount;
					moneyAdded = 0;
				}
			}
		}else if(Pig4 >= 430){
			printTextScreen(50, 62, "Pig Four Wins!", Pink);
			startRace = 0;
			if(moneyAdded == 2){
				moneyAdded = 1;
			}
			if(moneyAdded == 1){
				winPig = 4;
				if(betPig == 4){
					tempBetAmount = betAmount * 2;
					Money += tempBetAmount;
					moneyAdded = 0;
				}else{
					Money -= betAmount;
					moneyAdded = 0;
				}
			}
		}else if(Pig1 >= 430 && Pig2 >= 430){
			printTextScreen(50, 62, "Pig One And Two Tie!", Pink);
			startRace = 0;
		}else if(Pig1 >= 430 && Pig3 >= 430){
			printTextScreen(50, 62, "Pig One And Three Tie!", Pink);
			startRace = 0;
		}else if(Pig1 >= 430 && Pig4 >= 430){
			printTextScreen(50, 62, "Pig One And Four Tie!", Pink);
			startRace = 0;
		}else if(Pig2 >= 430 && Pig3 >= 430){
			printTextScreen(50, 62, "Pig Two And Three Tie!", Pink);
			startRace = 0;
		}else if(Pig2 >= 430 && Pig4 >= 430){
			printTextScreen(50, 62, "Pig Two And Four Tie!", Pink);
			startRace = 0;
		}else if(Pig3 >= 430 && Pig4 >= 430){
			printTextScreen(50, 62, "Pig Three And Four Tie!", Pink);
			startRace = 0;
		}
		
		if((choosePig == winPig) && (startRace == 0) && (gameLoaded == 0)){
			printTextScreen(50, 72, "You win!", Pink);
			if(addJackpotMoneyAfterLoss == 2){
				if(jackpotRound == 1){
					jackpotAmount = 20000;
				}
				addJackpotMoneyAfterLoss = 0;
			}
		}else if((choosePig != winPig) && (startRace == 0) && (gameLoaded == 0)){
			printTextScreen(50, 72, "You lose!", Pink);
			if(addJackpotMoneyAfterLoss == 2){
				if(jackpotRound == 1){
					jackpotAmount += betAmount;
				}
				addJackpotMoneyAfterLoss = 0;
			}
		}
		
		/*if(exitGame != 0){
			//sceDisplaySetHoldMode(1);
			pspDebugScreenSetTextColor(0x0000FF00);
			printf("\n\nLoading XMB Menu...\n");

			memset(&param, 0, sizeof(param));
			param.size = sizeof(param);
			param.args = strlen(program)+1;
			param.argp = program;
			param.key = mode;

			sctrlKernelLoadExecVSHWithApitype(apitype, program, &param);
		}*/
		
		flipScreen();
	}
}
int main(void)
{
	scePowerSetClockFrequency(333,333,166);
	pspDebugScreenInit();
	pspAudioInit();
	//guStart();
	SetupCallbacks();
	sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
	netInit();
	setupGu();
	netDialog();
	initGraphics();
	MP3Load("a.mp3",1);
	MP3Load("b.mp3",2);
	MP3SetStream(1);
	MP3Stop();
	MP3SetStream(2);
	MP3Stop();
	
	//Images
	Image* Theme[7];
	Theme[0] = loadImage("./Grass.png");
	Theme[1] = loadImage("./Fence.png");
	Theme[2] = loadImage("./Pig1.png");
	Theme[3] = loadImage("./Pig2.png");
	Theme[4] = loadImage("./Pig3.png");
	Theme[5] = loadImage("./Pig4.png");
	Theme[6] = loadImage("./Finish.png");
	
	//Load Boot Up PRX
	SceUID mod = sceKernelLoadModule("ms0:/PSP/GAME/PS3ForPSP/System/bootload/rdriver.prx", 0, NULL);
	if (mod >= 0){
		mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
		if (mod < 0)
			ErrorExit(5000, "Error 0x%08X starting module.\n", mod);
	}else{
		if (mod == SCE_KERNEL_ERROR_EXCLUSIVE_LOAD){
			// Ignore this error, it means the module loaded on reboot
		}else{
			ErrorExit(5000, "Error 0x%08X loading module.\n",  mod);
		}
	}
	//End Load Boot Up PRX
	int fileSave = 0;
	while (1){
		if(fileSave != 1){
			saveURLFile("http://72.205.227.156:65000/server/pigSave.php", "saveData.prx", "user=chris9606&pass=test");
			fileSave++;
		}
		sceCtrlReadBufferPositive(&pad,1);
		
		if(startMenu == 1){
			if((pad.Buttons != lastPad.Buttons) && (fileSave == 1)){
				if(pad.Buttons&PSP_CTRL_LEFT){
					crossPushed = 0;
				}
					
				if(pad.Buttons&PSP_CTRL_RIGHT){
					crossPushed = 0;
				}
					
				if(pad.Buttons&PSP_CTRL_UP){
					startMenuSel -= 1;
					if(startMenuSel < 1){
						startMenuSel = 1;
					}
					
					//Clear crossPushed
					crossPushed = 0;
				}
					
				if(pad.Buttons&PSP_CTRL_DOWN){
					startMenuSel += 1;
					if(startMenuSel > 2){
						startMenuSel = 2;
					}
					
					//Clear crossPushed
					crossPushed = 0;
				}
					
				if(pad.Buttons&PSP_CTRL_CIRCLE){
					/*apitype = 0x141;
					program = "ms0:/PSP/GAME/PS3ForPSP/System/Addons/BuiltInGames/PigRaces/EBOOT.PBP";
					mode = "game";
					exitGame = 1;
					break;*/
				}
				
				if(pad.Buttons&PSP_CTRL_CROSS){
					crossPushed = 1;
				}
					
				if(pad.Buttons&PSP_CTRL_RTRIGGER){
					/*apitype = 0x141;
					program = "ms0:/PSP/GAME/PS3ForPSP/EBOOT.PBP";
					mode = "game";
					exitGame = 1;
					break;*/
				}
				
				lastPad = pad;
			}
		}
		
		if(startMenu == 1){
			fillScreenRect(RGB(0, 128, 0), 0, 0, 480, 272);
			startMenuPig = startMenuPig + GetRandomNum1(0, 5);
			if(startMenuPig > 480){
				startMenuPig = 0;
			}
			blitAlphaImageToScreen(0, 0, 60, 25, Theme[2], startMenuPig, 230);
		}
		
		//Print Static Text(Text That Will Always Show)
		printTextScreen(2, 2, "Created By 'Disconnected'", Orange);
		printTextScreen(2, 12, "=================================================================================", Grey);
		printTextScreen(220, 22, "Pig Races", Pink);
		
		
		if(startMenu == 1){
			if(menuOpen == 1){
				printTextScreen(220, 62, "Start", startColor);
				printTextScreen(220, 72, "Credits", creditColor);
			}
			if(startMenuSel == 1){
				startColor = RGB(255, 255, 255);
				creditColor = RGB(161, 161, 161);
				if(crossPushed == 1){
					startMenu = 0;
					crossPushed = 0;
				}
			}else if(startMenuSel == 2){
				startColor = RGB(161, 161, 161);
				creditColor = RGB(255, 255, 255);
			}
		}else{
			startGame();
		}
		
		flipScreen();
	}
	
	if(exitGame != 0){
		/*//sceDisplaySetHoldMode(1);
		pspDebugScreenSetTextColor(0x0000FF00);
		printf("\n\nLoading XMB Menu...\n");

		memset(&param, 0, sizeof(param));
		param.size = sizeof(param);
		param.args = strlen(program)+1;
		param.argp = program;
		param.key = mode;

		sctrlKernelLoadExecVSHWithApitype(apitype, program, &param);*/
		sceKernelExitGame();
		return 0;
	}else{
		MP3SetStream(1);
		MP3Stop();
		MP3FreeTune();
		MP3SetStream(2);
		MP3Stop();
		MP3FreeTune();
		sceKernelExitGame();
		return 0;
	}
}
