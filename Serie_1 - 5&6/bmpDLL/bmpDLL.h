// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the BMPDLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// BMPDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef BMPDLL_EXPORTS
#define BMPDLL_API __declspec(dllexport)
#else
#define BMPDLL_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif
BMPDLL_API
BOOL WINAPI ChangeBitMapTo(LPCTSTR srcBitmap, LPCTSTR dstBitmap, RGBTRIPLE oldColour, RGBTRIPLE newColour);

#ifdef __cplusplus
}
#endif

// This class is exported from the bmpDLL.dll
/*
class BMPDLL_API CbmpDLL {
public:
	CbmpDLL(void);
	// TODO: add your methods here.
};

extern BMPDLL_API int nbmpDLL;

BMPDLL_API int fnbmpDLL(void);
*/