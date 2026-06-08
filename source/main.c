#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <setjmp.h>
#include <3ds.h>
#include <sys/dirent.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <stdbool.h>
/**
RGB introduce:
https://baike.baidu.com/item/BMP/35116?fr=aladdin
https://www.cnblogs.com/lzlsky/archive/2012/08/16/2641698.html
WINGDI.h
**/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BI_BITFIELDS 0x3

typedef char BYTE;
typedef short WORD;
typedef int DWORD;
typedef int LONG;

typedef struct tagBITMAPFILEHEADER{
	WORD bfType;//位图文件的类型，在Windows中，此字段的值总为‘BM’(1-2字节）
	DWORD bfSize;//位图文件的大小，以字节为单位（3-6字节，低位在前）
	WORD bfReserved1;//位图文件保留字，必须为0(7-8字节）
	WORD bfReserved2;//位图文件保留字，必须为0(9-10字节）
	DWORD bfOffBits;//位图数据的起始位置，以相对于位图（11-14字节，低位在前）
	//文件头的偏移量表示，以字节为单位
}__attribute__((packed)) BitMapFileHeader;	//BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
	DWORD biSize;//本结构所占用字节数（15-18字节）
	LONG biWidth;//位图的宽度，以像素为单位（19-22字节）
	LONG biHeight;//位图的高度，以像素为单位（23-26字节）
	WORD biPlanes;//目标设备的级别，必须为1(27-28字节）
	WORD biBitCount;//每个像素所需的位数，必须是1（双色），（29-30字节）
	//4(16色），8(256色）16(高彩色)或24（真彩色）之一
	DWORD biCompression;//位图压缩类型，必须是0（不压缩），（31-34字节）
	//1(BI_RLE8压缩类型）或2(BI_RLE4压缩类型）之一
	DWORD biSizeImage;//位图的大小(其中包含了为了补齐行数是4的倍数而添加的空字节)，以字节为单位（35-38字节）
	LONG biXPelsPerMeter;//位图水平分辨率，像素数（39-42字节）
	LONG biYPelsPerMeter;//位图垂直分辨率，像素数（43-46字节)
	DWORD biClrUsed;//位图实际使用的颜色表中的颜色数（47-50字节）
	DWORD biClrImportant;//位图显示过程中重要的颜色数（51-54字节）
}__attribute__((packed)) BitMapInfoHeader;	//BITMAPINFOHEADER;


typedef struct tagRGBQUAD{
	BYTE rgbBlue;//蓝色的亮度（值范围为0-255)
	BYTE rgbGreen;//绿色的亮度（值范围为0-255)
	BYTE rgbRed;//红色的亮度（值范围为0-255)
	BYTE rgbReserved;//保留，必须为0
}__attribute__((packed)) RgbQuad;	//RGBQUAD;

int Rgb565ConvertBmp(char *buf,int width,int height, const char *filename)
{
	FILE* fp;

	BitMapFileHeader bmfHdr; //定义文件头
	BitMapInfoHeader bmiHdr; //定义信息头
	RgbQuad bmiClr[3]; //定义调色板

	bmiHdr.biSize = sizeof(BitMapInfoHeader);
	bmiHdr.biWidth = width;//指定图像的宽度，单位是像素
	bmiHdr.biHeight = height;//指定图像的高度，单位是像素
	bmiHdr.biPlanes = 1;//目标设备的级别，必须是1
	bmiHdr.biBitCount = 16;//表示用到颜色时用到的位数 16位表示高彩色图
	bmiHdr.biCompression = BI_BITFIELDS;//BI_RGB仅有RGB555格式
	bmiHdr.biSizeImage = (width * height * 2);//指定实际位图所占字节数
	bmiHdr.biXPelsPerMeter = 0;//水平分辨率，单位长度内的像素数
	bmiHdr.biYPelsPerMeter = 0;//垂直分辨率，单位长度内的像素数
	bmiHdr.biClrUsed = 0;//位图实际使用的彩色表中的颜色索引数（设为0的话，则说明使用所有调色板项）
	bmiHdr.biClrImportant = 0;//说明对图象显示有重要影响的颜色索引的数目，0表示所有颜色都重要

	//RGB565格式掩码
	bmiClr[0].rgbBlue = 0;
	bmiClr[0].rgbGreen = 0xF8;
	bmiClr[0].rgbRed = 0;
	bmiClr[0].rgbReserved = 0;

	bmiClr[1].rgbBlue = 0xE0;
	bmiClr[1].rgbGreen = 0x07;
	bmiClr[1].rgbRed = 0;
	bmiClr[1].rgbReserved = 0;

	bmiClr[2].rgbBlue = 0x1F;
	bmiClr[2].rgbGreen = 0;
	bmiClr[2].rgbRed = 0;
	bmiClr[2].rgbReserved = 0;


	bmfHdr.bfType = (WORD)0x4D42;//文件类型，0x4D42也就是字符'BM'
	bmfHdr.bfSize = (DWORD)(sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader) + sizeof(RgbQuad) * 3 + bmiHdr.biSizeImage);//文件大小
	bmfHdr.bfReserved1 = 0;//保留，必须为0
	bmfHdr.bfReserved2 = 0;//保留，必须为0
	bmfHdr.bfOffBits = (DWORD)(sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader)+ sizeof(RgbQuad) * 3);//实际图像数据偏移量

	if (!(fp = fopen(filename, "wb"))){
		return -1;
	} else {
		printf("file %s open success\n",filename);
	}

	fwrite(&bmfHdr, 1, sizeof(BitMapFileHeader), fp);
	fwrite(&bmiHdr, 1, sizeof(BitMapInfoHeader), fp);
	fwrite(&bmiClr, 1, 3*sizeof(RgbQuad), fp);

//	fwrite(buf, 1, bmiHdr.biSizeImage, fp);	//mirror
	for(int i=0; i<height; i++){
		fwrite(buf+(width*(height-i-1)*2), 2, width, fp);
	}

	printf("Image size=%d, file size=%d, width=%d, height=%d\n", bmiHdr.biSizeImage, bmfHdr.bfSize, width, height);
	printf("%s over\n", __FUNCTION__);
	fclose(fp);

	 return 0;


}
#define WAIT_TIMEOUT 300000000ULL

#define WIDTH 400
#define HEIGHT 240
#define SCREEN_SIZE WIDTH *HEIGHT * 2
#define BUF_SIZE SCREEN_SIZE * 2

static jmp_buf exitJmp;

inline void clearScreen(void)
{
	u8 *frame = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	memset(frame, 0, 320 * 240 * 3);
}

void hang(char *message)
{
	clearScreen();
	printf("%s", message);
	printf("Press start to exit");

	while (aptMainLoop())
	{
		hidScanInput();

		u32 kHeld = hidKeysHeld();
		if (kHeld & KEY_START)
			longjmp(exitJmp, 1);
	}
}
// static void rotate90_ccw(const uint16_t *src, int w, int h, uint16_t *dst)
// {
// 	// src: w x h (400x240)
// 	// dst: h x w (240x400)
//
// 	for (int y = 0; y < h; y++)
// 	{
// 		for (int x = 0; x < w; x++)
// 		{
// 			dst[(w - 1 - x) * h + y] = src[y * w + x];
// 		}
// 	}
// }
void cleanup()
{
	camExit();
	gfxExit();
	acExit();
}

void writePictureToFramebufferRGB565(void *fb, void *img, u16 x, u16 y, u16 width, u16 height)
{
	u8 *fb_8 = (u8 *)fb;
	u16 *img_16 = (u16 *)img;
	int i, j, draw_x, draw_y;
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			draw_y = y + height - j;
			draw_x = x + i;
			u32 v = (draw_y + draw_x * height) * 3;
			u16 data = img_16[j * width + i];
			uint8_t b = ((data >> 11) & 0x1F) << 3;
			uint8_t g = ((data >> 5) & 0x3F) << 2;
			uint8_t r = (data & 0x1F) << 3;
			fb_8[v] = r;
			fb_8[v + 1] = g;
			fb_8[v + 2] = b;
		}
	}
}

// TODO: Figure out how to use CAMU_GetStereoCameraCalibrationData
void takePicture3D(u8 *buf)
{
	u32 bufSize;
	printf("CAMU_GetMaxBytes: 0x%08X\n", (unsigned int)CAMU_GetMaxBytes(&bufSize, WIDTH, HEIGHT));
	printf("CAMU_SetTransferBytes: 0x%08X\n", (unsigned int)CAMU_SetTransferBytes(PORT_BOTH, bufSize, WIDTH, HEIGHT));

	printf("CAMU_Activate: 0x%08X\n", (unsigned int)CAMU_Activate(SELECT_OUT1_OUT2));

	Handle camReceiveEvent = 0;
	Handle camReceiveEvent2 = 0;

	printf("CAMU_ClearBuffer: 0x%08X\n", (unsigned int)CAMU_ClearBuffer(PORT_BOTH));
	printf("CAMU_SynchronizeVsyncTiming: 0x%08X\n", (unsigned int)CAMU_SynchronizeVsyncTiming(SELECT_OUT1, SELECT_OUT2));

	printf("CAMU_StartCapture: 0x%08X\n", (unsigned int)CAMU_StartCapture(PORT_BOTH));

	printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int)CAMU_SetReceiving(&camReceiveEvent, buf, PORT_CAM1, SCREEN_SIZE, (s16)bufSize));
	printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int)CAMU_SetReceiving(&camReceiveEvent2, buf + SCREEN_SIZE, PORT_CAM2, SCREEN_SIZE, (s16)bufSize));
	printf("svcWaitSynchronization: 0x%08X\n", (unsigned int)svcWaitSynchronization(camReceiveEvent, WAIT_TIMEOUT));
	printf("svcWaitSynchronization: 0x%08X\n", (unsigned int)svcWaitSynchronization(camReceiveEvent2, WAIT_TIMEOUT));

	printf("CAMU_StopCapture: 0x%08X\n", (unsigned int)CAMU_StopCapture(PORT_BOTH));

	svcCloseHandle(camReceiveEvent);
	svcCloseHandle(camReceiveEvent2);

	printf("CAMU_Activate: 0x%08X\n", (unsigned int)CAMU_Activate(SELECT_NONE));
}

void savePictureWithTimestamp(u8 *buffer, size_t size)
{
	// Get the current date and time
	time_t rawtime;
	struct tm *timeinfo;
	char timeStr[32];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(timeStr, sizeof(timeStr), "%Y-%m-%d_%H-%M-%S", timeinfo);
	mkdir("sdmc:/DCIM/108NIN03", 0777);
	mkdir("sdmc:/DCIM/108NIN03/rgb", 0777);

	char filename[256];
	snprintf(filename, sizeof(filename), "sdmc:/DCIM/108NIN03/rgb/%s.rgb565", timeStr);

	FILE *file = fopen(filename, "wb");
	if (!file)
	{
		printf("Failed to open file for writing: %s\n", filename);
		return;
	}

	size_t written = fwrite(buffer, 1, size, file);
	if (written != size)
	{
		printf("Failed to write data to file. Only %zu bytes written.\n", written);
	}
	else
	{
		printf("Image successfully saved to %s\n", filename);

	}

	fclose(file);
}
int SaveInstaxRotatedBmp(const uint16_t *src)
{
    const int srcW = 400;
    const int srcH = 240;

    time_t rawtime;
    struct tm *timeinfo;
    char timeStr[32];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d_%H-%M-%S", timeinfo);

    mkdir("sdmc:/DCIM/108NIN03", 0777);
    mkdir("sdmc:/DCIM/108NIN03/instax", 0777);
    mkdir("sdmc:/DCIM/108NIN03/stretch", 0777);

    char cropFilename[256];
    char stretchFilename[256];

    snprintf(cropFilename, sizeof(cropFilename),
             "sdmc:/DCIM/108NIN03/instax/%s.bmp", timeStr);

    snprintf(stretchFilename, sizeof(stretchFilename),
             "sdmc:/DCIM/108NIN03/stretch/%s.bmp", timeStr);

    const int rotW = srcH; // 240
    const int rotH = srcW; // 400

    uint16_t *rot = malloc(rotW * rotH * sizeof(uint16_t));
    if (!rot) return -1;

    for (int y = 0; y < srcH; y++)
    {
        for (int x = 0; x < srcW; x++)
        {
            rot[(srcW - 1 - x) * srcH + y] = src[y * srcW + x];
        }
    }

    int cropResult = 0;
    int stretchResult = 0;

    // Cropped Instax aspect: fit width 240, crop height to about 323
    {
        const int cropW = rotW; // 240
        const int cropH = (int)(cropW * 62.0f / 46.0f); // about 323
        const int y0 = (rotH - cropH) / 2;

        uint16_t *out = malloc(cropW * cropH * sizeof(uint16_t));
        if (!out)
        {
            free(rot);
            return -1;
        }

        for (int y = 0; y < cropH; y++)
        {
            for (int x = 0; x < cropW; x++)
            {
                out[y * cropW + x] = rot[(y + y0) * rotW + x];
            }
        }

        cropResult = Rgb565ConvertBmp((char *)out, cropW, cropH, cropFilename);
        free(out);
    }

    // Stretched Instax aspect: resize full rotated image into 240x323
    {
        const int outW = rotW; // 240
        const int outH = (int)(outW * 62.0f / 46.0f); // about 323

        uint16_t *out = malloc(outW * outH * sizeof(uint16_t));
        if (!out)
        {
            free(rot);
            return -1;
        }

        for (int y = 0; y < outH; y++)
        {
            for (int x = 0; x < outW; x++)
            {
                int sx = x * rotW / outW;
                int sy = y * rotH / outH;

                out[y * outW + x] = rot[sy * rotW + sx];
            }
        }

        stretchResult = Rgb565ConvertBmp((char *)out, outW, outH, stretchFilename);
        free(out);
    }

    free(rot);

    if (cropResult != 0) return cropResult;
    if (stretchResult != 0) return stretchResult;

    return 0;
}

int saveBmpWithTimestamp(u8 *buffer)
{
	time_t rawtime;
	struct tm *timeinfo;
	char timeStr[32];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(timeStr, sizeof(timeStr), "%Y-%m-%d_%H-%M-%S", timeinfo);

	mkdir("sdmc:/DCIM/108NIN03", 0777);
	mkdir("sdmc:/DCIM/108NIN03/instax", 0777);

	char filename[256];
	snprintf(filename,
			 sizeof(filename),
			"sdmc:/DCIM/108NIN03/%s.bmp", timeStr);


	int result = Rgb565ConvertBmp(
		(char *)buffer,
		WIDTH,
		HEIGHT,
		filename);

	if (result == 0)
	{
		printf("BMP successfully saved to %s\n", filename);
	}
	else
	{
		printf("Failed to save BMP: %s\n", filename);
	}

	snprintf(filename, sizeof(filename),
		 "sdmc:/DCIM/108NIN03/instax/%s.bmp", timeStr);

	SaveInstaxRotatedBmp((uint16_t*)buffer);

	return result;
}

int main()
{
	// Initializations
	acInit();
	gfxInitDefault();
	consoleInit(GFX_BOTTOM, NULL);

	// Enable double buffering to remove screen tearing
	gfxSetDoubleBuffering(GFX_TOP, true);
	gfxSetDoubleBuffering(GFX_BOTTOM, false);

	// Save current stack frame for easy exit
	if (setjmp(exitJmp))
	{
		cleanup();
		return 0;
	}

	u32 kDown;
	u32 kHeld;

	printf("Initializing camera\n");

	printf("camInit: 0x%08X\n", (unsigned int)camInit());

	printf("CAMU_SetSize: 0x%08X\n", (unsigned int)CAMU_SetSize(SELECT_OUT1_OUT2, SIZE_CTR_TOP_LCD, CONTEXT_A));
	printf("CAMU_SetOutputFormat: 0x%08X\n", (unsigned int)CAMU_SetOutputFormat(SELECT_OUT1_OUT2, OUTPUT_RGB_565, CONTEXT_A));

	printf("CAMU_SetNoiseFilter: 0x%08X\n", (unsigned int)CAMU_SetNoiseFilter(SELECT_OUT1_OUT2, true));
	printf("CAMU_SetAutoExposure: 0x%08X\n", (unsigned int)CAMU_SetAutoExposure(SELECT_OUT1_OUT2, true));
	printf("CAMU_SetAutoWhiteBalance: 0x%08X\n", (unsigned int)CAMU_SetAutoWhiteBalance(SELECT_OUT1_OUT2, true));
	// printf("CAMU_SetEffect: 0x%08X\n", (unsigned int) CAMU_SetEffect(SELECT_OUT1_OUT2, EFFECT_MONO, CONTEXT_A));

	printf("CAMU_SetTrimming: 0x%08X\n", (unsigned int)CAMU_SetTrimming(PORT_CAM1, false));
	printf("CAMU_SetTrimming: 0x%08X\n", (unsigned int)CAMU_SetTrimming(PORT_CAM2, false));
	// printf("CAMU_SetTrimmingParamsCenter: 0x%08X\n", (unsigned int) CAMU_SetTrimmingParamsCenter(PORT_CAM1, 512, 240, 512, 384));

	u8 *buf = malloc(BUF_SIZE);
	if (!buf)
	{
		hang("Failed to allocate memory!");
	}

	gfxFlushBuffers();
	gspWaitForVBlank();
	gfxSwapBuffers();

	bool held_R = false;
	bool held_L = false;

	printf("\nPress R to take a new picture\n");
	printf("Press Start to exit to Homebrew Launcher\n");

	// Main loop
	while (aptMainLoop())
	{
		// Read which buttons are currently pressed or not
		hidScanInput();
		kDown = hidKeysDown();
		kHeld = hidKeysHeld();

		// If START button is pressed, break loop and quit
		if (kDown & KEY_START)
		{
			break;
		}
		// if (kDown & KEY_SELECT)
		// {
		// 	break;
		// }

		Handle camReceiveEvent = 0;
		u32 bufSize;

		printf("CAMU_GetMaxBytes: 0x%08X\n", (unsigned int)CAMU_GetMaxBytes(&bufSize, WIDTH, HEIGHT));
		printf("CAMU_SetTransferBytes: 0x%08X\n", (unsigned int)CAMU_SetTransferBytes(PORT_CAM1, bufSize, WIDTH, HEIGHT));
		printf("CAMU_Activate: 0x%08X\n", (unsigned int)CAMU_Activate(SELECT_OUT1_OUT2));
		printf("CAMU_ClearBuffer: 0x%08X\n", (unsigned int)CAMU_ClearBuffer(PORT_CAM1));
		printf("CAMU_StartCapture: 0x%08X\n", (unsigned int)CAMU_StartCapture(PORT_CAM1));
		printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int)CAMU_SetReceiving(&camReceiveEvent, buf, PORT_CAM1, SCREEN_SIZE, (s16)bufSize));
		printf("svcWaitSynchronization: 0x%08X\n", (unsigned int)svcWaitSynchronization(camReceiveEvent, WAIT_TIMEOUT));
		printf("CAMU_StopCapture: 0x%08X\n", (unsigned int)CAMU_StopCapture(PORT_CAM1));
		svcCloseHandle(camReceiveEvent);
		writePictureToFramebufferRGB565(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), buf, 0, 0, WIDTH, HEIGHT);
		if ((kHeld & KEY_R) && !held_R)
		{
			printf("Capturing new image\n");
			gfxFlushBuffers();
			gspWaitForVBlank();
			gfxSwapBuffers();
			held_R = true;
			takePicture3D(buf);
			savePictureWithTimestamp(buf, BUF_SIZE);
			saveBmpWithTimestamp(buf);
		}
		else if (!(kHeld & KEY_R))
		{
			held_R = false;
		}
		else if ((kHeld & KEY_L) && !held_L) {
			printf("Capturing new image\n");
			gfxFlushBuffers();
			gspWaitForVBlank();
			gfxSwapBuffers();
			held_R = true;
			takePicture3D(buf);
			savePictureWithTimestamp(buf, BUF_SIZE);
		}
		else if (!(kHeld & KEY_L)) {
			held_L = false;
		}

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gspWaitForVBlank();
		gfxSwapBuffers();
	}

	// Exit
	free(buf);
	cleanup();

	// Return to hbmenu
	return 0;
}
