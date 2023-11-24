#include <windows.h>
#include <stdio.h>
#include <shellapi.h>

#include <QApplication>
//#include <QPlastiqueStyle>
//#include <QCleanlooksStyle>
//#include <QWindowsVistaStyle>
#include "NFEvalGUIMainWindow.h"
#include "NFEval.h"
#include <NFEvalLogger.h>
#include "NFEvalGlobalConfig.h"
#if defined(VLD_FORCE_ENABLE)
#include <vld.h>
#endif

//extern "C"
//{
//int NFEvalInit();
//}
int main (int argc, char *argv[])
{

  NFEvalInit();
  NF::NFLoggerPushContext("NFEvalGui");
  NFLOG_INFO(NF::getNFLogger(),"Starting GUI");
  QString fn;
  if(argc>1)
  {
    fn = QString( argv[1]);
  }

  QApplication app(argc,argv);
  //app.setStyle(new QPlastiqueStyle);
  /*NF::NFQ3DTopoWidget topo;
  QMainWindow mw;
  mw.show();
  mw.setCentralWidget(&topo);*/

  int ret = -1;
  {
    NFEvalGUIMainWindow mainWin;
    mainWin.show();

    if(fn.size()>4)
    {
      mainWin.openFile(fn.toStdString());
    }

    ret = app.exec();
  }
  /* LocalFree(szArgList);
  for (int i = 0; i<argCount; ++i)
  {
  delete[] argv[i];
  }
  delete [] argv;*/
  NF::NFLoggerPopContext();
  NF::NFEvalGlobalConfig::Pointer conf = NF::NFEvalGlobalConfig::New();
  conf->writeGlobalConfig();
  NFEvalDestroy();
  return ret;
}

#ifdef WIN32
#include <windows.h>
int APIENTRY WinMain( HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR lpCmdLine,
  int nCmdShow)
{
  //Sleep(20000);
  char seps[]   = " \t\n";
  char *next_token1 = NULL;
  char *token1 = strtok_s( lpCmdLine, seps, &next_token1);


  return main(token1 != 0 ? 1:0, &lpCmdLine);
}

#endif