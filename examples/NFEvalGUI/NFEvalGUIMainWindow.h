/*
* myMainDialog.h
*
*  Created on: 18.09.2009
*      Author: ShapeFromShading
*/

#ifndef NFEvalGUIMainWindow_H_
#define NFEvalGUIMainWindow_H_


#include <string>
#include <list>
//#include <pair>
#include <QMainWindow>
#include <QThread>
#include <QInputDialog>


#include "NFTopography.h"
#include "NFEvaluation.h"
#include "ui_NFEvalGui.h"
#include "NFCommand.h"
#include "NFCLogger.h"

namespace NF
{
  class NF3DRenderer;
  class NFHistogramChart;
}
class NFEvaluationThread;



class NFEvalGUIMainWindow : public QMainWindow, public NF::NFLogCallBack
{
  Q_OBJECT

public:
  NFEvalGUIMainWindow(QWidget *parent = 0,Qt::WindowFlags flags=0);
  ~NFEvalGUIMainWindow();
  void openFile(const std::string &stdFn);
  virtual void newLogMessage(int LogLevel, const char* context, const char *message, const NFLogLocationInfo & loc, long long timeStamp, long long timeAfterStart);
  public slots:

    void addLogEntry(QString str);
   
    protected Q_SLOTS:
      void OpenFile();
      void SaveFile();
      void setTopoLayerMode(int idxView = -1);
      void setDisplayMode(int idxView = -1);
      void setProfile(int idxView = -1);
      void setHistogram(int idxView = -1);
      void setProfileAxis();
      void SetScaleFactor(double f);
      void SelectAlgo(QModelIndex idx);
      void SelectTopo(QListWidgetItem *p);
      void clearHistory();
 
      void Eval();
      void IntenstyCalibration();
      void SaveHistory();
      void saveParams();

      void saveParameterSet(NF::NFParameterSet::Pointer pset);

      void loadParams();

      void LoadHistory();
      void threadFinished();
      void showMetaInfo();
      void rereadPlugins();
      void getRenderdImage();
      void showAxis(bool);
      void setDebugData(void* e,const QString  name  );
      void continueEval();
      void readSettings();
      void writeSettings();
      void mousePositionSlot(double xPos, double yPos, double zPos, unsigned int colX, unsigned int rowY, double heightVal, int intensityVal, unsigned int maskVal, int colorVal);
      void mouseLeftButtonDblClickInSceneSlot(double xPos, double yPos);
      void mouseButtonPressInSceneSlot(double xPos, double yPos, NF2DTopoViewer::NFUserMouseButton btn);
      void mouseButtonReleaseInSceneSlot(double xPos, double yPos, NF2DTopoViewer::NFUserMouseButton btn);
      void lineChangedSlot(NFObjectID* topoId, NFObjectID* lineId, double startX, double startY, double endX, double endY);
      void saveOutputParams();
      void reinit3DView();
      void abortEval();
      void setView(int currentView);
      void displayColorChanged(QString s);
      void Mode2Dchanged();
      void evalInit();
      void evalDestroy();

signals:
      void debugDone(int how);
      void appendLogMsg(QString str);

protected:
  void closeEvent(QCloseEvent *event);


private:

  void SelectAlgo(QString str,NF::NFParameterSet::Pointer pset=NF::NFParameterSet::Pointer());
  void SelectAlgo(NF::NFEvaluation::Pointer algo,NF::NFParameterSet::Pointer pset=NF::NFParameterSet::Pointer());
  void setCurrentTopo(NF::NFTopography::Pointer topo,int idxView = -1 );
  Ui:: NFEvalGUIMW ui;
  //NF::NF3DRenderer * renderer;
   
  NFEvaluationThread *thread;
  

  //std::list<QLayoutItem *> m_AlgoParmGuiItems;
  //std::list<std::pair<QLayout *, QWidget *> > m_AlgoParmGuiWidgets;
  //std::map<std::string,QLineEdit *> m_AlgoParamsEdit; 
  NF::NFEvaluation::Pointer m_CurrentAlgo;
  NF::NFTopography:: Pointer m_CurrentTopo;
  int m_algoCount;

  QString m_LastOpenDirTopo;

};


class Command :  public QObject,public NF::NFCommand
{
  Q_OBJECT

signals:
  void progressPercent(int value);
  void debugDataReady(void*,const QString  );
  public slots:
    void debugDone(int how);
public:
  Command():progress(0.0f),m_debugDone(false),m_debugEnable(false),m_abortEval(false){};
  float progress;
  virtual void execute(const NF::NFObject::ConstPointer *caller, const NF::NFEvent & evnt ) const;
  // Method that is executed on event handling
  virtual void execute(NF::NFObject::Pointer *caller,  const NF::NFEvent & evnt);

private:
  volatile bool m_debugDone;
  bool m_debugEnable;
  bool m_abortEval;
};


class NFEvaluationThread : public QThread
{
  Q_OBJECT
public:

  NFEvaluationThread()
    :m_ResultStatus(0)
  {      
  };

  NF::NFParameterSet::Pointer paramSet;
  NF::NFEvaluation::Pointer eval;
  NF::NFTopography::Pointer topo;
  Command cmd;
  NFSetGetConstMacroLight(ResultStatus, int);

protected:
  void run();
private:
  int m_ResultStatus;

};



#endif 
