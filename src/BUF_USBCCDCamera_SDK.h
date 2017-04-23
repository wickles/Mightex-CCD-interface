// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MTUSBDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MTUSBDLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
typedef int SDK_RETURN_CODE;
typedef unsigned int DEV_HANDLE;

#ifdef SDK_EXPORTS
#define SDK_API extern "C" __declspec(dllexport) SDK_RETURN_CODE _cdecl
#define SDK_HANDLE_API extern "C" __declspec(dllexport) DEV_HANDLE _cdecl
#define SDK_POINTER_API extern "C" __declspec(dllexport) unsigned char * _cdecl
#else
#define SDK_API extern "C" __declspec(dllimport) SDK_RETURN_CODE _cdecl
#define SDK_HANDLE_API extern "C" __declspec(dllimport) DEV_HANDLE _cdecl
#define SDK_POINTER_API extern "C" __declspec(dllimport) unsigned char * _cdecl
#endif

#define GRAB_FRAME_FOREVER	0x8888

typedef struct {
    int Revision;
    // For Image Capture
    int Resolution;
    int ExposureTime;
    // GPIO Control
    unsigned char GpioConfigByte; // Config for Input/Output for each pin.
    unsigned char GpioCurrentSet; // For output Pins only.
} TImageControl;

typedef struct {
    int CameraID;
    int Row;
    int Column;
    int Bin;
    int XStart;
    int YStart;
    int ExposureTime;
    int RedGain;
    int GreenGain;
    int BlueGain;
    int TimeStamp;
    int TriggerOccurred;
    int TriggerEventCount;
    int UserMark;
    int FrameTime;
    int CCDFrequency;

    int FrameProcessType;
    int tFilterAcceptForFile;
} TProcessedDataProperty;


typedef TImageControl *PImageCtl;

typedef void (* DeviceFaultCallBack)( int DeviceType );
typedef void (* FrameDataCallBack)( TProcessedDataProperty* Attributes, unsigned char *BytePtr );

// Import functions:
SDK_API BUFCCDUSB_InitDevice( void );
SDK_API BUFCCDUSB_UnInitDevice( void );
SDK_API BUFCCDUSB_GetModuleNoSerialNo( int DeviceID, char *ModuleNo, char *SerialNo);
SDK_API BUFCCDUSB_GetUserSerialNo( int DeviceID, char *UserSerialNo);
SDK_API BUFCCDUSB_SetUserSerialNo( int DeviceID, char *UserSerialNo, int IsStore);
SDK_API BUFCCDUSB_AddDeviceToWorkingSet( int DeviceID );
SDK_API BUFCCDUSB_RemoveDeviceFromWorkingSet( int DeviceID );
SDK_API BUFCCDUSB_ActiveDeviceInWorkingSet( int DeviceID, int Active );
SDK_API BUFCCDUSB_StartCameraEngine( HWND ParentHandle, int CameraBitOption );
SDK_API BUFCCDUSB_StopCameraEngine( void );
SDK_API BUFCCDUSB_SetCameraWorkMode( int DeviceID, int WorkMode );
SDK_API BUFCCDUSB_StartFrameGrab( int TotalFrames );
SDK_API BUFCCDUSB_StopFrameGrab( void );
SDK_API BUFCCDUSB_ShowFactoryControlPanel( int DeviceID, char *passWord );
SDK_API BUFCCDUSB_HideFactoryControlPanel( void );
SDK_API BUFCCDUSB_SetCustomizedResolution( int deviceID, int RowSize, int ColSize, int Bin, int BufferCnt );
SDK_API BUFCCDUSB_SetExposureTime( int DeviceID, int exposureTime );
SDK_API BUFCCDUSB_SetFrameTime( int DeviceID, int frameTime );
SDK_API BUFCCDUSB_SetXYStart( int deviceID, int XStart, int YStart );
SDK_API BUFCCDUSB_SetGains( int deviceID, int RedGain, int GreenGain, int BlueGain );
SDK_API BUFCCDUSB_SetGamma( int Gamma, int Contrast, int Bright, int Sharp );
SDK_API BUFCCDUSB_SetBWMode( int BWMode, int H_Mirror, int V_Flip );
SDK_API BUFCCDUSB_InstallFrameHooker( int FrameType, FrameDataCallBack FrameHooker );
SDK_API BUFCCDUSB_InstallUSBDeviceHooker( DeviceFaultCallBack USBDeviceHooker );
SDK_POINTER_API BUFCCDUSB_GetCurrentFrame( int FrameType, int deviceID, unsigned char* &FrameBuf);
SDK_API BUFCCDUSB_SetGPIOConfig( int DeviceID, unsigned char ConfigByte );
SDK_API BUFCCDUSB_SetGPIOInOut( int DeviceID, unsigned char OutputByte,
                             unsigned char *InputBytePtr );


