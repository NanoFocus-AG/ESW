#ifndef NFCONTUR_CONFIG_H_INCL
#define  NFCONTUR_CONFIG_H_INCL
#if (defined(WIN32) || defined(_WIN32))  && !defined(SWIG) && defined( NFCONTUR_EXPORTS) && !defined( NFCONTUR_API)
  #define WIN32_LEAN_AND_MEAN
  #include<windows.h>
  #define  NFCONTUR_API __declspec(dllexport)
#elif !defined( NFCONTUR_API) && defined(SWIG)
 #define  NFCONTUR_API 
#elif !defined( NFCONTUR_API)
#define   NFCONTUR_API __declspec(dllimport)
#endif


#ifndef  NFCONTUR_CALLING_CONVENTION
#define  NFCONTUR_CALLING_CONVENTION __cdecl
#endif
#endif