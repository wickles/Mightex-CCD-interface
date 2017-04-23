#include "stdafx.h"

#include <Windows.h>
#include "camera.h"
#include "ini_reader.h"
#include <direct.h>
#include <cstdio>

// clamps x < a to x = a, x > b to x = b
static int clamp(int x, int a, int b)
{
	if (b < a)
	{
		int tmp = a;
		a = b;
		b = tmp;
	}
	if (x < a)
		return a;
	if (x > b)
		return b;
	return x;
}

// Function for loading program settings
void InitCameraSettings( MightexInterface& ifc, FrameDataCallBack FrameHook, DeviceFaultCallBack CameraFaultCallBack, const char* filename )
{
	ifc.Cameras = NULL;
	ifc.Timer = 0;

	FILE* file = fopen( filename, "r" );
	if ( file == NULL )
	{
		printf( "Error opening settings file.\n" );
		goto exit;
	}
	
	ifc.NumberOfCameras =	IniGetInt( file, "NumberOfCameras", 0 );
	ifc.TriggerMode =		IniGetBool( file, "TriggerMode", false );
	ifc.CaptureDelay =		IniGetInt( file, "CaptureDelay", 1000 );
	ifc.BitMode =			IniGetInt( file, "BitMode", 12 );

	if ( ifc.BitMode != 12 )
	{
		printf( "Error: Program only supports 12 bit images for now.\n" );
		goto exit;
	}

	int ret;

	// Initialize camera engine
	printf( "Initializing drivers.\n" );
	ret = BUFCCDUSB_InitDevice();
	printf( "There are %d devices connected.\n", ret );
	if ( ret <= 0 )
	{
		printf( "Error: No devices.\n" );
		goto exit;
	}

	// create file structure name base
	char dir_base[512];
	SYSTEMTIME systime;
	GetLocalTime( &systime );
	sprintf_s( dir_base, 512, "data_%04d%02d%02d_%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay,
			systime.wHour, systime.wMinute, systime.wSecond );

	ifc.Cameras = new camera_s[ifc.NumberOfCameras];
	if ( ifc.Cameras == NULL )
	{
		printf( "Error: Failed to allocate memory for cameras." );
		goto exit;
	}

	char buffer[512];
	camera_s* cam;
	int idx, width, height;
	for ( idx = 1; idx <= ifc.NumberOfCameras; idx++ )
	{
		cam = &ifc.Cameras[idx-1];
		cam->DarkImage.data = NULL;

#define PREPEND(IDX,STR) strncat(buffer, STR, 512-4)

		sprintf_s(buffer, 512, "C%02d_", idx);
		cam->Settings.ResHoriz =		IniGetInt( file, PREPEND(idx,"ResHoriz"), 1280 );
		sprintf_s(buffer, 512, "C%02d_", idx);
		cam->Settings.ResVert =			IniGetInt( file, PREPEND(idx,"ResVert"), 960 );
		sprintf_s(buffer, 512, "C%02d_", idx);
		cam->Settings.BinMode =			clamp( IniGetInt( file, PREPEND(idx,"BinMode"), 1 ), 1, 4 );
		sprintf_s(buffer, 512, "C%02d_", idx);
		cam->Settings.ExposureTime =	IniGetInt( file, PREPEND(idx,"ExposureTime"), 20 );
		sprintf_s(buffer, 512, "C%02d_", idx);
		cam->Settings.RedGain =			clamp( IniGetInt( file, PREPEND(idx,"RedGain"), 14 ), 6, 41 );
		sprintf_s(buffer, 512, "C%02d_", idx);
		cam->Settings.GreenGain =		clamp( IniGetInt( file, PREPEND(idx,"GreenGain"), 14 ), 6, 41 );
		sprintf_s(buffer, 512, "C%02d_", idx);
		cam->Settings.BlueGain =		clamp( IniGetInt( file, PREPEND(idx,"BlueGain"), 14 ), 6, 41 );
		sprintf_s(buffer, 512, "C%02d_", idx);
		cam->Settings.DarkCoeff =		IniGetFloat( file, PREPEND(idx,"DarkCoeff"), 1.0f );
		sprintf_s(buffer, 512, "C%02d_", idx);
		cam->Settings.DumpImages =		IniGetBool( file, PREPEND(idx,"DumpImages"), false );

		sprintf_s( cam->DumpDirectory, 512, "%s_C%02d", dir_base, idx );
		mkdir( cam->DumpDirectory );

		width = cam->Settings.ResHoriz;
		height = cam->Settings.ResVert;	

		// Load the dark image
		cam->DarkImage.width = width;
		cam->DarkImage.height = height;
		cam->DarkImage.bitmode = ifc.BitMode;
		cam->DarkImage.data = new unsigned short[width * height];

		if (cam->DarkImage.data != NULL)
		{
			sprintf_s( buffer, 512, "dark%02d.raw", idx );
			FILE* file = fopen( buffer, "rb");
			if (file != NULL)
			{
				fread(cam->DarkImage.data, width * height * sizeof(unsigned short), 1, file);
				fclose(file);
				if ( cam->Settings.BinMode > 1 )
				{
					// If binning is enabled, simulate binning on the dark image
					unsigned short* DarkPtr = (unsigned short*)cam->DarkImage.data;
					unsigned int sum;
					int x, y, i, j;
					for ( y = 0; y < height; y++ )
					{
						for ( x = 0; x < width; x++ )
						{
							sum = 0;
							for ( i = 0; i < cam->Settings.BinMode; i++ )
								for ( j = 0; j < cam->Settings.BinMode; j++ )
									sum += DarkPtr[(y+j)*width + (x+i)];
							DarkPtr[y*width+x] = sum / cam->Settings.BinMode;
						}
					}
				}

			}
			else
			{
				delete[] cam->DarkImage.data;
				cam->DarkImage.data = NULL;
			}
		}

		printf( "Settings up device #%02d.\n", idx );
		ret = BUFCCDUSB_GetModuleNoSerialNo( idx, cam->ModuleNo, cam->SerialNo );
		if ( ret == 1 )
		{
			printf( "Module number: %s\n", cam->ModuleNo );
			printf( "Serial number: %s\n", cam->SerialNo );
		}
		else
		{
			printf( "Error: Couldn't retrieve module or serial number.\n" );
			goto exit;
		}

		ret = BUFCCDUSB_AddDeviceToWorkingSet( idx );
		ret = BUFCCDUSB_SetCameraWorkMode( idx, CAMERA_MODE(ifc.TriggerMode) );
		ret = BUFCCDUSB_SetGains( idx, cam->Settings.RedGain, cam->Settings.GreenGain, cam->Settings.BlueGain );

		// Note -- Not sure what the last parameter to this function is for, something to do with camera buffer count. 4 works for now.
		ret = BUFCCDUSB_SetCustomizedResolution( idx, width, height, BIN_MODE(cam->Settings.BinMode), 4 );

		// exposure input is in 50 microsecond units
		ret = BUFCCDUSB_SetExposureTime( idx, cam->Settings.ExposureTime*1000/50 );
	}

	fclose(file);


	printf( "Starting camera engine.\n" );
	ret = BUFCCDUSB_StartCameraEngine( NULL, ifc.BitMode );
	if ( ret == 2 )
	{
		printf("Error: Tried setting to 12 bit mode, but not supported -- setting to 8 bit instead.\n");
		ifc.BitMode = 8;
	}
	else if ( ret == -1 )
	{
		printf("Error: Camera engine could not start.\n");
		goto exit;
	}

	printf( "Installing hookers, start grabbing!\n" );
	ret = BUFCCDUSB_InstallFrameHooker( FRAME_TYPE, FrameHook );
	printf( "BUFCCDUSB_InstallFrameHooker returns %d\n", ret );
	ret = BUFCCDUSB_InstallUSBDeviceHooker( CameraFaultCallBack );

	if ( ifc.TriggerMode == true )
		BUFCCDUSB_StartFrameGrab( GRAB_FRAME_FOREVER );
	
	return;

exit:
	delete[] ifc.Cameras;
	ifc.Cameras = NULL;
}

void CleanupCameraSettings( MightexInterface& ifc )
{
	BUFCCDUSB_InstallFrameHooker( FRAME_TYPE, NULL );
	BUFCCDUSB_StopFrameGrab();
	if ( ifc.Cameras != NULL )
	{
		BUFCCDUSB_StopCameraEngine();
		BUFCCDUSB_UnInitDevice();

		// Free all cameras
		int idx;
		for ( idx = 1; idx <= ifc.NumberOfCameras; idx++ )
		{
			if (ifc.Cameras[idx].DarkImage.data != NULL)
				delete[] ifc.Cameras[idx].DarkImage.data;
		}
	}
}