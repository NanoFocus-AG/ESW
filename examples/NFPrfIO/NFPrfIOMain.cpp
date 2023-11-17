
#include "NFPrfIO_Config.h"
 
#include "NFEvaluationFactory.h"

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include "NFFileIOFactory.h"
#include "NFPrfIO.h"
#include "NFFileWriterTemplate.h"
#include "NFFileReaderTemplate.h"

#if defined(VLD_FORCE_ENABLE)
#include <vld.h>
#endif
namespace NF
{
typedef ::NF::NFFileWriterTemplate<::NF::NFPrfIO> NFFileWriterPrf_Type;

typedef ::NF::NFFileReaderTemplate<::NF::NFPrfIO> NFFileReaderPrf_Type;
} // namespace NF

template class ::NF::NFFileWriterTemplate<::NF::NFPrfIO>;
template class ::NF::NFFileReaderTemplate<::NF::NFPrfIO>;

const size_t MAX_MODULENAME_SIZE = 4095;
static TCHAR NFPrfIODLLmoduleName[MAX_MODULENAME_SIZE] = {0};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{

  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
      NFLOG_TRACE(NF::getNFLogger("NFEvaluation.FileIO.NFPrfIO"), "NFPrfIO: DLL_PROCESS_ATTACH");
      GetModuleFileName(hModule, NFPrfIODLLmoduleName, MAX_MODULENAME_SIZE);
      break;
    case DLL_THREAD_ATTACH:

      break;
    case DLL_THREAD_DETACH:

      break;
    case DLL_PROCESS_DETACH:
      NFLOG_TRACE(NF::getNFLogger("NFEvaluation.FileIO.NFPrfIO"), "NFPrfIO: DLL_PROCESS_DETACH");
      break;
  }

  return TRUE;
}
#endif

extern "C"
{
  // return Version of NFEval.dll
  NFPrfIO_API const char *NFPrfIO_CALLING_CONVENTION NFEVAL_GETVERSION()
  {
    return ("8.5.0.0");
	 
  }
  // Do initialization and adding of Objects to Factory here
  NFPrfIO_API int NFPrfIO_CALLING_CONVENTION NFLoad()
  {
    int ret = 0;
       
    NF::NFFileIOFactory::Pointer fileIoFactory = NF::NFFileIOFactory::New();
    // Add algos to factory eg.
    ADD_EVAL_TO_FACTORY_WITH_SOURCE(fileIoFactory , NFPrfIO, ++ret, NFLOG_ERROR(NF::getNFLogger("NFEvaluation.FileIO.NFPrfIO"), "Could not Add  NFPrfIO"),
                                    NFPrfIODLLmoduleName);

    NF::NFEvaluationFactory::Pointer facEval = NF::NFEvaluationFactory::New();
    ADD_EVAL_TO_FACTORY_WITH_SOURCE(facEval, NFFileWriterPrf_Type, ++ret,
                                    NFLOG_ERROR(NF::getNFLogger("NFEvaluation.FileIO.NFPrfIO"), "Could not Add  NFFileWriterFits_Type"),
                                    NFPrfIODLLmoduleName);
    ADD_EVAL_TO_FACTORY_WITH_SOURCE(facEval, NFFileReaderPrf_Type, ++ret,
                                    NFLOG_ERROR(NF::getNFLogger("NFEvaluation.FileIO.NFPrfIO"), "Could not Add  NFFileReaderFits_Type"),
                                    NFPrfIODLLmoduleName);

    return (ret);
  }
}
