
#include "NFEvalFileVersion.h"
#include "NFContour_Config.h"

#include "NFEval.h"
#include "NFEvaluation.h"
#include "NFEvaluationFactory.h"
#include "NFContour.h"

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
 
 BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{

  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
   
      break;
    case DLL_THREAD_ATTACH:

      break;
    case DLL_THREAD_DETACH:

      break;
    case DLL_PROCESS_DETACH:
      break;
  }

  return TRUE;
}
#endif

extern "C"
{
  // return Version of NFEval.dll
   NFCONTUR_API const char * NFCONTUR_CALLING_CONVENTION NFEVAL_GETVERSION() 
   {
     return ("8.5.0.0");
   }
  // Do initialization and adding of Objects to Factory here
   NFCONTUR_API int  NFCONTUR_CALLING_CONVENTION NFLoad()
  {
    int ret = 0;
	

    NF::NFEvaluationFactory::Pointer facEval = NF::NFEvaluationFactory::New();

    NF::NFEvaluation::Pointer NFContour = NF::NFContour::New();
   
   bool success = facEval->addObject(NF::NFLightObject::Pointer(NFContour));
       
   if(success) ret++;

    return (ret);
  }
}

