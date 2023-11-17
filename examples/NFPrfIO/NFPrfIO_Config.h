#ifndef NFPrfIO_CONFIG_H_INCL
#define NFPrfIO_CONFIG_H_INCL
#if (defined(WIN32) || defined(_WIN32))  && !defined(SWIG) && defined(NFPrfIO_EXPORTS) && !defined(NFPrfIO_API)
  #define WIN32_LEAN_AND_MEAN
  #include<windows.h>
  #define NFPrfIO_API __declspec(dllexport)
#elif !defined(NFPrfIO_API) && defined(SWIG)
 #define  NFPrfIO_API 
#elif !defined(NFPrfIO_API)
#define  NFPrfIO_API __declspec(dllimport)
#endif


#ifndef NFPrfIO_CALLING_CONVENTION
#define NFPrfIO_CALLING_CONVENTION __cdecl
#endif
#endif