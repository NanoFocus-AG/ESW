/*
* myMainDialog.cpp
*
*  Created on: 18.09.2009
*      Author: stieger
*/

#include <sstream>
#include <string>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QSettings>
 

#include "NFParameterSetFileIO.h"
#include "NFEvalLogger.h"
#include "NFQParamsetEditDialog.h"
#include "NFAlgoIOFactory.h"

#include <NFEval.h>
#include "NFFileReader.h"

#include "NFFileWriter.h"
#include "NFEvaluationFactory.h"

#include "NFHistogram.h"
 
#include "NFAlgoIO.h"
#include "NFAlgoTopoMinMax.h"
#include "NFComplexEvaluation.h"
#include "NFAlgoIOFactory.h"
#include "NFParameterSetIOFactory.h"

#include "NFEvalGUIMainWindow.h"
#include "NFUnits.h"
 
#include "NF3DEnums.h"
#include "NFTopoViewer.h"
#include "NF2DTopoViewer.h"
#include "NFRect.h"
#include "NFCLogger.h"
#include "NFQtWidgetsUtils.h"
#include <QDebug>

static const char * AppName ="NFEvalGui.config";

static const char * LAST_OPEN_DIR_TOPO = "LastOpenDirTopo";


class AlgoListItem:public QListWidgetItem
{
  //Q_OBJECT

public:
  NF::NFParameterSet::Pointer ParamSet;
  NF::NFEvaluation::Pointer Algo;

};

void addAlgo(QAbstractItemModel *model, const std::string &name)
{
  model->insertRow(0);
  model->setData(model->index(0, 0), QString(name.c_str()));
}

QAbstractItemModel *createAlgoList(QObject *parent)
{
  QStandardItemModel *model = new QStandardItemModel(1, 1, parent);

  model->setHeaderData(0, Qt::Horizontal, QObject::tr("Algoname"));

  NF::NFEvaluationFactory::Pointer fac = NF::NFEvaluationFactory::New();
  const NF::NFEvaluationFactory::LightObjectListType &li=fac->getLightObjectList();
  NF::NFEvaluationFactory::LightObjectListType::const_iterator it=li.begin();
 
  for(;it!=li.end();++it)
  {
    addAlgo(model,(*it)->getName());
  }

  return model;
}

QAbstractItemModel *createAlgoItem(QObject *parent,NF::NFLightObject::Pointer ptr)
{
  QStandardItemModel *model = new QStandardItemModel(1, 2, parent);

  model->setHeaderData(0, Qt::Horizontal, QObject::tr("Algoname"));
  model->setHeaderData(1, Qt::Horizontal, QObject::tr("Pointer"));


  model->insertRow(0);
  model->setData(model->index(0, 0), QString(ptr->getName().c_str()));
  model->setData(model->index(0, 1), size_t(ptr.get()));

  return model;
}

void Command::debugDone(int how)
{
  m_debugEnable = (0!= (how&2));
  m_abortEval = (0!=(how&4));
  m_debugDone = (0!= (how&1));
}

void Command::execute(const NF::NFObject::ConstPointer *caller, const NF::NFEvent & evnt ) const
{
  NFLOG_WARN(NF::getNFLogger(),"Irgendwas läuft hier schief");
}

// Method that is executed on event handling
void Command::execute(NF::NFObject::Pointer *caller,  const NF::NFEvent & evnt )
{
  int evtTyp= evnt.getType();
  QMessageBox msgBox;
  int ret=0;
  NF::NFEvaluation * eval;
  switch(evtTyp)
  {
  case  NF::NFEvent::PROGRESS_EVENT:
    NF::NFProcessObject* obj;
    if(obj=dynamic_cast<NF::NFProcessObject*>(caller->get()))
    {
      progress= obj->getProgress();
      emit progressPercent(progress *100);
    }
    break;
  case  NF::NFEvent::DEBUG_EVENT:

    if(eval=dynamic_cast<NF::NFEvaluation*>(caller->get()))
    {
      m_debugDone=false;
      NF::NFEvaluationDebugEvent ev(*((NF::NFEvaluationDebugEvent*)&evnt));
      QString a(ev.getDebugEventName().c_str());
      emit  debugDataReady(eval,a ) ;
      
      while(false == m_debugDone)
      {
        Sleep(50);
      }
      eval->setDebugEnabled(m_debugEnable);
    }
    break;
  default:
    break;
  }
}

void NFEvaluationThread::run()
{
  if(eval->getNumberOfInputTopos() == 1)
  {
    eval->setInputTopo(topo);
  }
  eval->setParameterSet(paramSet);

  eval->addObserver(NF::NFProgressEvent(),&cmd);
  eval->addObserver(NF::NFEvaluationDebugEvent(""),&cmd);
  //eval->setDebugEnabled(true);
  eval->setReleaseInputsAfterUse(true);
  m_ResultStatus =eval->evaluate();
}

void NFEvalGUIMainWindow::continueEval()
{
  ui.btnContinue->setEnabled(false);
  int val = 0;
  val |=1;
  if (ui.chk_Debug->isChecked())
  {
    val |=2;
  }
  emit debugDone(val);
}

void NFEvalGUIMainWindow::setDebugData(void* e, const QString name )
{

  NF::NFEvaluation::Pointer eval( (NF::NFEvaluation*)e);

  ui.debugParams->setParameterSet(eval->getDebugParameterSet());
  if (eval->getDebugParameterSet()->containsParameter(NF::NFEvaluation_LatestDebugTopos))
  {
    NF::NFVariant var  =eval->getDebugParameter(NF::NFEvaluation_LatestDebugTopos);
    if (var.getNumberOfElements() >0 && var.getType() == NF::NFVariant::INT_VECTOR_TYPE)
    {
      setCurrentTopo(eval->getDebugOutput(var.getIntVector()[0]));
    }
  }
 
  ui.tabsAlgoParams->setCurrentIndex(2);
  ui.tabsAlgoParams->setTabText(2,QString(name) );
  ui.btnContinue->setEnabled(true);
 }

NFEvalGUIMainWindow::NFEvalGUIMainWindow(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent,flags)
{
  m_algoCount=0;

  ui.setupUi(this);

  readSettings();

  //ui.TopoView->SetUseTDx(true);

 
  thread = new NFEvaluationThread();
  QObject::connect(thread,SIGNAL(	finished()),this,SLOT(threadFinished()));
  QObject::connect(&(thread->cmd),SIGNAL(	progressPercent(int)),this->ui.progressBar,SLOT(setValue(int )));

  QObject::connect(&(thread->cmd),SIGNAL(	debugDataReady(void*,const QString )),this,SLOT(setDebugData(void*,const QString  )));

  QObject::connect(this,SIGNAL(debugDone(int)),&(thread->cmd),SLOT(debugDone(int)));
  QObject::connect(ui.TopoView,SIGNAL(mousePosition(double, double, double, unsigned int, unsigned int, double, int, unsigned int, int)),this,SLOT(mousePositionSlot(double, double, double, unsigned int, unsigned int, double, int, unsigned int, int )));
  QObject::connect(ui.btn_ColorRangeSync, SIGNAL(clicked()), this , SLOT(syncColor()));
  QObject::connect(ui.actionNFEvalInit, SIGNAL(triggered()), this, SLOT(evalInit()));
  QObject::connect(ui.actionNFEvalDestroy, SIGNAL(triggered()), this, SLOT(evalDestroy()));
  ui.TopoView->setNumberOfViews(1);
  ui.TopoView->setAdvancedMode(true,-1);
 // ui.TopoView->GetRenderWindow()->Render();
  ui.TopoView->repaint(-1);
  ui.TopoView->connectTo3DTopoWidget(0);
  
  NFTopoViewer * v=ui.TopoView->getTopoViewer(0);
  NF2DTopoViewer *topo2DViewer = v->getTopoViewer2D();
  if (v)
  {
    topo2DViewer = v->getTopoViewer2D();
    if (topo2DViewer)
    {
      QObject::connect(topo2DViewer, SIGNAL(mouseLeftButtonDblClickInScene(double, double)), this, SLOT(mouseLeftButtonDblClickInSceneSlot(double, double)));
      QObject::connect(topo2DViewer, SIGNAL(mouseButtonPressInScene(double, double, NF2DTopoViewer::NFUserMouseButton)), this, SLOT(mouseButtonPressInSceneSlot(double, double, NF2DTopoViewer::NFUserMouseButton)));
      QObject::connect(topo2DViewer, SIGNAL(mouseButtonReleaseInScene(double, double, NF2DTopoViewer::NFUserMouseButton)), this, SLOT(mouseButtonReleaseInSceneSlot(double, double, NF2DTopoViewer::NFUserMouseButton)));
      QObject::connect(topo2DViewer, SIGNAL(lineChanged(NFObjectID*, NFObjectID*, double, double, double, double)), this, SLOT(lineChangedSlot(NFObjectID*, NFObjectID*, double, double, double, double)));
    }
  }

  //if (v)
  //{
  //NFRect newSceneSize;
  //v->getTopoViewer2D()->setSceneSize(0, 0, 50000, 50000, &newSceneSize);
  //v->getTopoViewer2D()->addRectangle(0, 0, 50000, 50000, QColor(Qt::gray).rgba(), QColor(Qt::black).rgba());
  //}

  /*QObject::connect(ui.actionOpen, SIGNAL(triggered()), this,
  SLOT(OpenFile()));*/
 // vtkOpenGLHardwareSupport * hardware = 
 //   vtkOpenGLRenderWindow::SafeDownCast(ui.TopoView->GetRenderWindow())->GetHardwareSupport();

 // vtkOpenGLExtensionManager *extensions = vtkOpenGLExtensionManager::New();
 // extensions->SetRenderWindow(ui.TopoView->GetRenderWindow());
  // Force a Render here so that we can call glGetString reliably:
  //
 // ui.TopoView->GetRenderWindow()->Render();
  //ui.AvailableAlgosList->setNumberOfInputTopos(1);
  ui.AvailableAlgosList_2->setNumberOfInputTopos(2);
  //const char *gl_vendor =
  //	reinterpret_cast<const char *>(glGetString(GL_VENDOR));
  //const char *gl_version =
  //	reinterpret_cast<const char *>(glGetString(GL_VERSION));
  //const char *gl_renderer =
  //	reinterpret_cast<const char *>(glGetString(GL_RENDERER));

  //cout << endl;
  //cout << "GL_VENDOR: " << (gl_vendor ? gl_vendor : "(null)") << endl;
  //cout << "GL_VERSION: " << (gl_version ? gl_version : "(null)") << endl;
  //cout << "GL_RENDERER: " << (gl_renderer ? gl_renderer : "(null)") << endl;

  //cout << endl;
  //ui.TopoView->GetRenderWindow()->Print(cout);
  //cout << "LoadSupportedExtension..." << endl;
  //int supported=extensions->ExtensionSupported("GL_VERSION_1_2");
  //int loaded=0;
  //if(supported)
  //{
  //	cout << "Driver claims to support OpenGL 1.2" <<endl;
  //	loaded=extensions->LoadSupportedExtension("GL_VERSION_1_2");
  //	if(loaded)
  //	{
  //		cout << "OpenGL 1.2 features loaded." <<endl;
  //	}
  //	else
  //	{
  //		cout << "Failed to load OpenGL 1.2 features!" <<endl;
  //	}
  //}
  //supported=extensions->ExtensionSupported("GL_VERSION_1_3");
  //if(supported)
  //{
  //	cout << "Driver claims to support OpenGL 1.3" <<endl;
  //	loaded=extensions->LoadSupportedExtension("GL_VERSION_1_3");
  //	if(loaded)
  //	{
  //		cout << "OpenGL 1.3 features loaded." <<endl;
  //	}
  //	else
  //	{
  //		cout << "Failed to load OpenGL 1.3 features!" <<endl;
  //	}
  //}
  //supported=extensions->ExtensionSupported("GL_VERSION_1_4");
  //if(supported)
  //{
  //	cout << "Driver claims to support OpenGL 1.4" <<endl;
  //	loaded=extensions->LoadSupportedExtension("GL_VERSION_1_4");
  //	if(loaded)
  //	{
  //		cout << "OpenGL 1.4 features loaded." <<endl;
  //	}
  //	else
  //	{
  //		cout << "Failed to load OpenGL 1.4 features!" <<endl;
  //	}
  //}
  //supported=extensions->ExtensionSupported("GL_VERSION_1_5");
  //if(supported)
  //{
  //	cout << "Driver claims to support OpenGL 1.5" <<endl;
  //	loaded=extensions->LoadSupportedExtension("GL_VERSION_1_5");
  //	if(loaded)
  //	{
  //		cout << "OpenGL 1.5 features loaded." <<endl;
  //	}
  //	else
  //	{
  //		cout << "Failed to load OpenGL 1.5 features!" <<endl;
  //	}
  //}
  //supported=extensions->ExtensionSupported("GL_VERSION_2_0");
  //if(supported)
  //{
  //	cout << "Driver claims to support OpenGL 2.0" <<endl;
  //	loaded=extensions->LoadSupportedExtension("GL_VERSION_2_0");
  //	if(loaded)
  //	{
  //		cout << "OpenGL 2.0 features loaded." <<endl;
  //	}
  //	else
  //	{
  //		cout << "Failed to load OpenGL 2.0 features!" <<endl;
  //	}
  //}
  //supported=extensions->ExtensionSupported("GL_VERSION_2_1");
  //if(supported)
  //{
  //	cout << "Driver claims to support OpenGL 2.1" <<endl;
  //	loaded=extensions->LoadSupportedExtension("GL_VERSION_2_1");
  //	if(loaded)
  //	{
  //		cout << "OpenGL 2.1 features loaded." <<endl;
  //	}
  //	else
  //	{
  //		cout << "Failed to load OpenGL 2.1 features!" <<endl;
  //	}
  //}
  //cout << "GetExtensionsString..." << endl;
  //cout << extensions->GetExtensionsString() << endl;

  //cout << "Set up pipeline." << endl;

  //ui.TopoView->GetRenderWindow()->AddRenderer(renderer->getRenderer());

  //ui.AvailableAlgosList->setModel(createAlgoList(this));
ui.cbx_ColorPalette->clear();
ui.cbx_ColorPalette->addItem(QString("NFTopoToColor<NFColorFunctors::NFTopoHeightPixelToNFRainbowFunctor>"));
ui.cbx_ColorPalette->addItem(QString("NFTopoToColor<NFColorFunctors::NFTopoIntensityToGrayscaleFunctor>"));
ui.cbx_ColorPalette->addItem(QString("NFTopoToColor<NFColorFunctors::NFTopoMaskToBinaryFunctor>"));
ui.cbx_ColorPalette->addItem(QString("NFTopoToColor<NFColorFunctors::NFTopoHeightToGrayscaleFunctor>"));
ui.cbx_ColorPalette->addItem(QString("NFTopoToColor<NFColorFunctors::NFTopoHeightToColorMapFunctor>"));
ui.cbx_ColorPalette->addItem(QString("NFTopoToColor<NFColorFunctors::NFTopoIntensityToColorMapFunctor>"));
ui.cbx_ColorPalette->addItem(QString("NFTopoToColor<NFColorFunctors::NFTopoHeightToNFSpectralFunctor>"));
ui.cbx_ColorPalette->addItem(QString("NFTopoToColor<NFColorFunctors::NFTopoHeightPixelToHSVRainbowFunctor>"));


ui.m_Colorbar->connectViewer(v->getColorBarClient());
ui.m_Colorbar->setVisible(true);
 QObject::connect(this, SIGNAL(appendLogMsg(QString)), this , SLOT(addLogEntry(QString)));
 NFCLogger::setCallbackHandler(this);
}

void NFEvalGUIMainWindow::closeEvent(QCloseEvent *event)
{
  writeSettings();
}

NFEvalGUIMainWindow::~NFEvalGUIMainWindow() {

	 
	delete thread;
  //	delete renderer;
  //	delete ui;
}

void NFEvalGUIMainWindow::SelectTopo(QListWidgetItem *p)
{
  AlgoListItem* itm= reinterpret_cast<AlgoListItem*>(p);
  if(NULL!=itm)
  {
    if(NULL!=itm->Algo.get())
    {
      int cnt = itm->Algo->getNumberOfOutputTopos();
      int cnw = 2; // Number of windows
      ui.OutputParams->setParameterSet(itm->Algo->getOutputParameterSet());
      int idx    = 0 ;
      int idxView = 0 ;
      if(0==cnt)
      {
        //setCurrentTopo(itm->Algo->getOutputTopo());
      }
      else
      {
         bool okWin = false;       
         idxView = QInputDialog::getInt(this, tr("Enter Window Number"),tr("No:"), 0, 0, cnw-1, 1, &okWin);
        if (cnt>1)
        {

          bool ok = false;
           
          idx = QInputDialog::getInt(this, tr("Enter Output Topo Index"),
            tr("Index:"), 0, 0, cnt-1, 1, &ok);

          if(false == ok) 
          {
            idx = 0 ;
          }
          if(false == okWin) 
          {
            idxView = 0 ;
          }
        }
        if (0==itm->Algo->getOutputTopo(idx).get())
        {
          QMessageBox::warning(this,tr("Cannot set NULL Topo"), tr("Ignoring Selection"));
        }
        else
        {
          setCurrentTopo(itm->Algo->getOutputTopo(idx),idxView);
        }
      }

      SelectAlgo(itm->Algo,itm->ParamSet);

      ui.TopoView->repaint(-1);

      //histChart->setTopo(m_CurrentTopo);
      //ui.HistogramChart->update();
    }
  }
}

void NFEvalGUIMainWindow::SelectAlgo(QModelIndex  idx)
{

  if(idx.data().canConvert<QString>())
  {
    QString data = idx.data().toString();

    SelectAlgo(data);
  }
}

void NFEvalGUIMainWindow::SelectAlgo(NF::NFEvaluation::Pointer algo,NF::NFParameterSet::Pointer pset)
{
  //while(m_AlgoParmGuiWidgets.size()>0)
  //{
  //	QLayout * t= m_AlgoParmGuiWidgets.front().first;
  //	QWidget * w = m_AlgoParmGuiWidgets.front().second;
  //	m_AlgoParmGuiWidgets.pop_front();
  //	t->removeWidget(w);
  //	//delete t;
  //	delete w;
  //}

  //while(m_AlgoParmGuiItems.size()>0)
  //{
  //	QLayoutItem * t= m_AlgoParmGuiItems.front();
  //	m_AlgoParmGuiItems.pop_front();
  //	ui.AlgoParamLayout->removeItem(t);
  //	delete t;
  //}
  //m_AlgoParamsEdit.clear();

  /*ui.verticalLayout_12->removeItem(ui.AlgoParamLayout);
  delete ui.AlgoParamLayout;

  ui.AlgoParamLayout = new QFormLayout();
  ui.AlgoParamLayout->setObjectName(QString::fromUtf8("AlgoParamLayout"));

  ui.verticalLayout_12->addLayout(ui.AlgoParamLayout);fselect

  QFormLayout *layout = ui.AlgoParamLayout;
  if (layout) {
  for (int i = 0; i < layout->count(); ++i)
  {		
  QLayoutItem* w=layout->itemAt(i, QFormLayout::LabelRole);

  layout->removeItem(w);
  delete w;
  w=layout->itemAt(i,QFormLayout::FieldRole);

  layout->removeItem(w);
  delete w;
  }

  }
  */

  m_CurrentAlgo = algo;
  NF::NFParameterSet::Pointer params;

  if(NULL==pset.get())
  {
    m_CurrentAlgo->setInputTopo(m_CurrentTopo);
    params= m_CurrentAlgo->getDefaultParameterSet();
  }
  else
  {
    params=pset;
  }
  ui.tabsAlgoParams->setCurrentIndex(0);
  ui.tabsAlgoParams->setTabText(0,QString(m_CurrentAlgo->getName().c_str()));

  ui.m_AlgoParamsEdit->setParameterSet(params);
  ui.AlgoParamGrpBox->update();
  //ui.HistogramChart->update();
}

void NFEvalGUIMainWindow::SelectAlgo(QString str,NF::NFParameterSet::Pointer pset)
{
  NF::NFEvaluationFactory::Pointer fac = NF::NFEvaluationFactory::New();
  NF::NFEvaluation::Pointer Algo = NF::NFEvaluation::Pointer(fac->getObjectByName(str.toStdString()).get());
  if(NULL!=Algo.get())
  {
    SelectAlgo(Algo,pset);
  }
}

void NFEvalGUIMainWindow::SetScaleFactor(double f)
{
  ui.TopoView->setZScaleFactor(f,-1);
  ui.TopoView->repaint(-1);
}

void NFEvalGUIMainWindow::setTopoLayerMode(int idxView)
{
  /* if(ui.radioButton_intens->isChecked())
  {
  ui.TopoView->setTopoLayerMode(NF::NF3DEnums::IntensityImage,idxView); 
  }
  else if(ui.radioButton_height->isChecked())
  {
  ui.TopoView->setTopoLayerMode(NF::NF3DEnums::HeightImage,idxView);
  }
  else if(ui.radioButton_ModeColor->isChecked())
  {
  ui.TopoView->setTopoLayerMode(NF::NF3DEnums::ColorImage, idxView);
  }
  else if(ui.radioButton_mask->isChecked())
  {
  ui.TopoView->setTopoLayerMode(NF::NF3DEnums::MaskImage,idxView);
  }*/
  ui.TopoView->repaint(-1);   
}

void NFEvalGUIMainWindow::setDisplayMode(int idxView)
{
  if(ui.radioButton_3d->isChecked())
  {
    ui.TopoView->setDisplayMode(NF::NF3DEnums::image3D,idxView);
 
  }
  else
  {
    ui.TopoView->setDisplayMode(NF::NF3DEnums::image2D,idxView);
    
  }  
  ui.TopoView->repaint(-1);   
}

/*! set the loaded topo in the current topo widget
@param topo ###
@param idxView ###
@return void
*/
void NFEvalGUIMainWindow::setCurrentTopo(NF::NFTopography::Pointer topo,int idxView)
{
  static bool fristRun=true;

  if(topo.get() != NULL)
  {
    m_CurrentTopo= topo;
    
    if(m_CurrentTopo->hasColors())
    {
      ui.cbx_ColorPalette->addItem("NFIdentiyFilter");
      
    }
    else
    {
      
      // TODO fix this
      //ui.cbx_ColorPalette->removeItem("NFIdentityFilter");
      
    }
    if (ui.tab_NF3DView->isVisible())
    {
      int numViews = ui.TopoView->getNumberOfViews();
      if(idxView>=numViews)
      {
        ui.TopoView->setNumberOfViews(idxView+1);
      }
      double factorZ = 1.0;
      {
        NFEvaluationFactory::Pointer factory = NFEvaluationFactory::New();
        NFEvaluation::Pointer minMaxEval = NFEvaluation::Pointer(factory->getObjectByName("NFAlgoTopoMinMax").get());
        minMaxEval->setInput(m_CurrentTopo);
        minMaxEval->evaluate();
        NFVariant minH = minMaxEval->getOutputParameter("Minimum Height");
        NFVariant maxH = minMaxEval->getOutputParameter("Maximum Height");
        const double heightRange=maxH.getDouble()-minH.getDouble();
        const double lateralRange = std::min(m_CurrentTopo->getDeltaXInMicrons() * m_CurrentTopo->getCountX()
          , m_CurrentTopo->getDeltaYInMicrons() * m_CurrentTopo->getCountY());
        const double heightPercentOfWidth = 8.0;
        

        if (fabs(heightRange) > 1e-6)
        {
          // höhe soll 8% der breite betragen 
          const double heightRangeSoll = heightPercentOfWidth * 0.01 * lateralRange;
          factorZ = heightRangeSoll / heightRange ;
        }

        

      }
      ui.TopoView->setZScaleFactor(factorZ,0);
      ui.TopoView->setTopoPointer(m_CurrentTopo,idxView);      
      ui.TopoView->setZScaleFactor(factorZ,0);
    }
    else if (ui.tab_openGL->isVisible())
    {
#ifdef NF_OGL_RENDERER
      ui.glwidget->setTopo(topo);
#endif
    }   
   
    setProfile(0);
    setHistogram(0);

    ui.TopoView->update();
    ui.TopoView->pick(ui.TopoView->geometry().center().rx(),ui.TopoView->geometry().center().ry(),true,-1);
    ui.TopoView->repaint(-1);  
    long nrPoints = m_CurrentTopo->getCountY() * m_CurrentTopo->getCountX();
    

  /*  NF::NFTopography::MaskContainerType::Iterator itM = m_CurrentTopo->getMask()->begin();
    for(;itM!= m_CurrentTopo->getMask()->end();++itM)
    {
      if(0!=*itM)
      {
        ++nrPointsValid;
      }
    }

    NF::NFTopography::IntensityContainerType::Iterator itI = m_CurrentTopo->getIntensities()->begin();
    for(;itI!= m_CurrentTopo->getIntensities()->end();++itI)
    {
      if(0!=*itI)
      {
        ++nrPointsValidIntens;
      }
    }*/

    QString str;
    str.append("Topo Info \n");
    str.append("Height [Points]: ");
    str.append(QString::number(m_CurrentTopo->getCountY()));
    str.append(" = ");
    str.append(QString::number(m_CurrentTopo->getCountY()*m_CurrentTopo->getDeltaYInMillimeter() ));
    str.append(" mm\nWidth  [Points]: ");
    str.append(QString::number(m_CurrentTopo->getCountX()));
    str.append(" = ");
    str.append(QString::number(m_CurrentTopo->getCountX()*m_CurrentTopo->getDeltaXInMillimeter() ));
    str.append(" mm\ndy: ");
    str.append(QString::number(m_CurrentTopo->getDeltaY()));
    QString s;//= QString::fromStdWString(NF::convertMultiplicatorToString(m_CurrentTopo->getUnitMultiplicatorDeltas()));
    s.append("");
    s.append(NF::convertUnitToString(m_CurrentTopo->getUnitY()));
    str.append(" " + s);
    str.append("   dx: ");
    str.append(QString::number(m_CurrentTopo->getDeltaX()));
    str.append(" " + s);
    str.append("\n   spx: ");
    str.append(QString::number(m_CurrentTopo->getOffsetX()*m_CurrentTopo->getUnitConversionFactor(m_CurrentTopo->getUnitMultiplicatorOffsetX(),NF::NFMultiplicatorMilli ) ));
    str.append(" mm");
    str.append("\n   spy: ");
    str.append(QString::number(m_CurrentTopo->getOffsetY()*m_CurrentTopo->getUnitConversionFactor(m_CurrentTopo->getUnitMultiplicatorOffsetY(),NF::NFMultiplicatorMilli ) ));
    str.append(" mm");
    str.append("\n   spz: ");
    str.append(QString::number(m_CurrentTopo->getOffsetZ()*m_CurrentTopo->getUnitConversionFactor(m_CurrentTopo->getUnitMultiplicatorOffsetZ(),NF::NFMultiplicatorMilli ) ));
    str.append(" mm");


    str.append("\nTotal Number of Points: ");
    str.append(QString::number(nrPoints));
    str.append("\nNumber of Points Valid (Mask): ");


    
    NF::NFEvaluationFactory::Pointer fac = NF::NFEvaluationFactory::New();
    NF::NFAlgoTopoMinMax::Pointer eval = fac->getObjectByName("NFAlgoTopoMinMax");
    if(eval.get()!=0)
    {
      eval->setInputTopo(m_CurrentTopo);
      eval->evaluate();
      NF::NFParameterSet::Pointer outParams = eval->getOutputParameterSet();
      NF::NF_INT64 nrPointsValid = outParams->getParameter(NF::NFAlgoTopoMinMax_PointsUsed).getInt64();
      str.append(QString::number(nrPointsValid));
      str.append("\n");
      str.append("Min Height: ");
      str.append(QString::number(eval->getMinHeight()*NF::NFTopography::getUnitConversionFactor(m_CurrentTopo->getUnitMultiplicatorHeights(),NF::NFMultiplicatorMikro)));
      str.append(u8" µm\n");
      str.append("Max Height: ");
      str.append(QString::number(eval->getMaxHeight()*NF::NFTopography::getUnitConversionFactor(m_CurrentTopo->getUnitMultiplicatorHeights(),NF::NFMultiplicatorMikro)));
      str.append(u8" µm\n");
      str.append("Min Intenstiy: ");
      str.append(QString::number(eval->getMinIntensity()));
      str.append("\n");
      str.append("Max Intenstiy: ");
      str.append(QString::number(eval->getMaxIntensity()));
      str.append("\n");

    }

    ui.LogOutput->setText(str);
  }
}

void NFEvalGUIMainWindow::threadFinished()
{

  if(m_CurrentAlgo->getNumberOfOutputTopos()>0)
  {
    setCurrentTopo( m_CurrentAlgo->getOutputTopo());
  }
  else
  {

  }
  if (thread->getResultStatus() != 0)
  {
    QMessageBox::warning(this, tr("Warning"),tr("Evaluation Error"));
  }


  AlgoListItem *itm=new AlgoListItem;
  QString cap(m_CurrentAlgo->getName().c_str());
  cap.append(" (");
  cap.append(QString::number(m_algoCount));
  cap.append(")");
  itm->setText(cap);
  m_algoCount++;

  itm->ParamSet=m_CurrentAlgo->getParameterSet();
  itm->Algo=m_CurrentAlgo;
  ui.AlgoHistroy->addItem(itm);
  ui.OutputParams->setParameterSet(m_CurrentAlgo->getOutputParameterSet());
  this->ui.EvalBtn->setEnabled(true);
  this->ui.btn_AbortEval->setEnabled(false);
}

void NFEvalGUIMainWindow::Eval()
{
  NF::NFParameterSet::Pointer params = ui.m_AlgoParamsEdit->getParameterSet();//  m_CurrentAlgo->getDefaultParameterSet();
  m_CurrentAlgo=NF::NFEvaluation::Pointer(m_CurrentAlgo->createAnother().get());
  if(m_CurrentAlgo->getNumberOfInputTopos()>=1)
  {
    m_CurrentAlgo->setInputTopo(m_CurrentTopo);
  }

  m_CurrentAlgo->setDebugEnabled(ui.chk_Debug->isChecked());
  thread->paramSet = params;
  thread->eval =m_CurrentAlgo;
  thread->topo = m_CurrentTopo;

  if(m_CurrentAlgo->getNumberOfInputTopos()>1)
  {
    return;
  }

  /*NF::NFParameterSet::iterator it= params->begin();
  for(;it!= params->end();++it)
  {
  QLineEdit* led=m_AlgoParamsEdit[it->first];
  QString qstr=led->text();
  std::string str=qstr.toStdString();
  it->second.valueFromString(str);

  }*/
  /* m_CurrentAlgo->setParameterSet(params);
  m_CurrentAlgo->evaluate();*/
  ui.progressBar->setRange ( 0,100 );
  ui.progressBar->setValue ( 0 );

  thread->start();
  this->ui.EvalBtn->setEnabled(false);
  this->ui.btn_AbortEval->setEnabled(true);

  //
  //while(thread.isRunning())
  //{
  //   ui.progressBar->setValue ( thread.cmd.progress * 100);
  //  Sleep(10);
  //}
}

void  NFEvalGUIMainWindow::rereadPlugins()
{
  QString fn = QFileDialog::getExistingDirectory(this,QString("Select Dir"),QString("C:\\NanoFocus\\NFMSoft"));
  NFLoadPlugins(fn.toStdString().c_str());

  ui.AvailableAlgosList->refreshAlgoList();
}

void  NFEvalGUIMainWindow::openFile(const std::string &stdFn)
{
  if(stdFn.size() > 4)
  {
    ui.TopoView->setNumberOfViews(1);
    NF::NFFileReader::Pointer reader=NF::NFFileReader::New();

    reader->setFileName(stdFn);
    if(0==reader->evaluate())
    {
      if(reader->getOutputTopo().isValid())
      {
        if(reader->getOutputTopo()->getCountX() >0 && reader->getOutputTopo()->getCountY()>0)
        {
          //ui.AlgoHistroy->clear();

          setCurrentTopo(reader->getOutputTopo());
          AlgoListItem *itm=new AlgoListItem;
          itm->setText(QString(reader->getName().c_str()));
          itm->ParamSet=reader->getParameterSet();
          itm->Algo=reader;
          ui.AlgoHistroy->addItem(itm);          
        }
      }
    }
  }
}

void NFEvalGUIMainWindow::showMetaInfo()
{
  NF::NFQParamsetEditDialog dia(this,Qt::Dialog);
  if(m_CurrentTopo.get() != NULL)
  {
    dia.setParameterSet(m_CurrentTopo->getMetaData());
    if(dia.exec() == QDialog::Accepted)
    {
    }
  }
}

void NFEvalGUIMainWindow::OpenFile()
{
  QString fn = QFileDialog::getOpenFileName(this,QString("Open Topo"),m_LastOpenDirTopo, tr("NanoFocus (*.nms *.oms *.sip *.png *.fits);;All Files (*.*)"));
  //QString fn = QFileDialog::getOpenFileName(this,QString("Open Topo"),QString(), tr("NanoFocus (*.nms *.oms *.sip);;All Files (*.*)"));
  if (fn.size()>0)
  {
    QFileInfo fi(fn);
    if (fi.exists())
    {
      m_LastOpenDirTopo = fi.absolutePath();
    }
  }

  std::string stdFn=fn.toStdString();

  openFile(stdFn);
}

void NFEvalGUIMainWindow::SaveFile()
{
  QString fn = QFileDialog::getSaveFileName(this,QString("Save Topo"),QString("D:\\"), tr("NanoFocus (*.nms);;NanoFocus (*.sip);;All Files (*.*)"));

  std::string stdFn=fn.toStdString();

  if(stdFn.size() > 4)
  {
    NF::NFFileWriter::Pointer writer=NF::NFFileWriter::New();
    NF::NFTopography::Pointer topo;
    topo=ui.TopoView->getTopoPointer(0);

    writer->setInputTopo(topo);

    writer->setFileName(stdFn);

    if(true == writer->canWrite(stdFn))
    {
      writer->setFileName(stdFn);
      writer->evaluate();
    }
  }
}

void NFEvalGUIMainWindow::IntenstyCalibration()
{
  /*NFmSprintIntensityCalibrationDialog * dia = new NFmSprintIntensityCalibrationDialog(this);
  dia->show();
  dia->OpenFiles();
  dia->exec();*/
}

void  NFEvalGUIMainWindow::SaveHistory()
{
  //QString fn = QFileDialog::getSaveFileName(this,QString("Save History"),QString("D:\\"), tr("NanoFocus Evaluation Description (*.ned);;All Files (*.*)"));
  //if(fn.length() != 0)
  //{
  //  NF::NFComplexEvaluation::Pointer complexEval = NF::NFComplexEvaluation::New();
  //  NF::NFTopography::Pointer  CurrentTopo=m_CurrentTopo;
  //  int lastIdx=0;
  //  int firstIdx=0;
  //  NF::NFEvaluation::ConstPointer p(reinterpret_cast<const NF::NFEvaluation *>(CurrentTopo->getCreator()));
  //  //NF::NFEvaluation::Pointer p(pc->createAnother());
  //  lastIdx=complexEval->addEval(p);
  //  complexEval->addOutputTopoConnection(lastIdx);
  //  CurrentTopo =p->getInputTopo();
  //  bool cnt = CurrentTopo.get() != NULL;
  //  true == cnt ? cnt = CurrentTopo->getCreator()!=NULL : cnt  ;
  //  while(true ==cnt)
  //  {
  //    p=reinterpret_cast<const NF::NFEvaluation *>(CurrentTopo->getCreator());
  //    CurrentTopo =p->getInputTopo();

  //    firstIdx=complexEval->addEval(p);
  //    complexEval->addTopoConnection(firstIdx,lastIdx);
  //    lastIdx = firstIdx;             

  //    cnt = CurrentTopo.get() != NULL;
  //    true == cnt ? cnt = CurrentTopo->getCreator()!=NULL : cnt  ;
  //  }
  //  if(p->getNumberOfInputTopos()==1)
  //  {
  //    complexEval->addInputTopoConnection(firstIdx);
  //  }
  //  complexEval->setName(fn.toStdString());


  //  NF::NFAlgoIOFactory::Pointer IOfac =  NF::NFAlgoIOFactory::New();
  //  NF::NFEvaluationIoTpl::Pointer xmlIo=IOfac->getObjectByName("NFAlgoXMLIO");
  //  xmlIo->setEval(complexEval.get());
  //  xmlIo->setDestination(fn.toStdString());
  //  xmlIo->write();
 // }

  //if(fn.length() != 0)
  //{
  //  int cnt =this->ui.AlgoHistroy->count();

  //  AlgoListItem* itm= reinterpret_cast<AlgoListItem*>(this->ui.AlgoHistroy->item(cnt-1));

  //  typedef std::pair< std::string, NF::NFParameterSet::Pointer > pairType;
  //  typedef std::list<pairType>  listType;
  //  if(NULL!=itm)
  //  {
  //    if(NULL!=itm->Algo.get())
  //    {

  //      listType algoList;

  //      NF::NFTopography::Pointer  CurrentTopo=itm->Algo->getOutputTopo();
  //      while(CurrentTopo->getCreator()!=NULL)
  //      {

  //        //ss<<CurrentTopo->getCreator()->getName()<<std::endl;

  //        NF::NFEvaluation::Pointer p(CurrentTopo->getCreator());
  //        CurrentTopo =p->getInputTopo();
  //        algoList.push_front(pairType(p->getName(),p->getParameterSet()));
  //      }
  //      std::ostringstream ss;
  //      listType::iterator it = algoList.begin();
  //      NF::NFParameterSetFileIO::Pointer paramSetIO = NF::NFParameterSetFileIO::New();

  //      for(int i=0;it!= algoList.end();++it,++i)
  //      {
  //        std::string ParamFileName=fn.toStdString();
  //        ParamFileName.append(".");
  //        ParamFileName.append(it->first);
  //        QString num;
  //        num.setNum(i);
  //        ParamFileName.append(num.toStdString());
  //        ParamFileName.append(".pset");
  //        ss<<'"'<<it->first<<'"'<< ' '<<'['<<ParamFileName<<']'<<std::endl;
  //        paramSetIO->setDestination(ParamFileName);
  //        paramSetIO->setParameterSet(it->second);
  //        paramSetIO->write();
  //      }

  //      std::ofstream out (fn.toStdString().c_str());
  //      out<<ss.str();
  //      out.close();

  //    }
  //  }
  //}
}

void  NFEvalGUIMainWindow::LoadHistory()
{
  QString fn = QFileDialog::getOpenFileName(this,QString("Load History"),QString("D:\\"), tr("NanoFocus Eval (*.eval);;All Files (*.*)"));
  if(fn.length() != 0)
  {
    ui.AlgoHistroy->clear();
    std::ifstream in(fn.toStdString().c_str());
    const size_t maxLineLength=5000;
    char * name = new char [maxLineLength];
    char * dummy = new char [maxLineLength];
    char * ParamFile = new char [maxLineLength];
    NF::NFEvaluationFactory::Pointer fac = NF::NFEvaluationFactory::New();
    NF::NFEvaluation::Pointer lastAlgo;
    while(in)
    {
      in.getline(dummy,maxLineLength,'"');
      in.getline(name,maxLineLength,'"');
      in.getline(dummy,maxLineLength,'[');
      in.getline(ParamFile,maxLineLength,']');
      if(in)
      {
        NF::NFEvaluation::Pointer algo(fac->getObjectByName(name));
        if(algo.get()!=NULL)
        {
          NF::NFParameterSetFileIO::Pointer paramSetIO = NF::NFParameterSetFileIO::New();
          paramSetIO->setSource(ParamFile);
          paramSetIO->read();
          NF::NFParameterSet::Pointer params = paramSetIO->getReadParameterSet();
          algo->setParameterSet(params);
          if(lastAlgo.get() != 0)
          {
            algo->setInputTopo(lastAlgo->getOutputTopo());
          }
          algo->evaluate();
          AlgoListItem *itm=new AlgoListItem;
          itm->setText(QString(algo->getName().c_str()));
          itm->ParamSet=algo->getParameterSet();
          itm->Algo=algo;
          ui.AlgoHistroy->addItem(itm);
          lastAlgo=algo;
        }
      }
    }
    delete [] name;
    delete [] dummy;
    delete [] ParamFile;
  }
}

void NFEvalGUIMainWindow::showAxis(bool val)
{
  //ui.TopoView->setShowAxis(val);
}
void NFEvalGUIMainWindow::getRenderdImage()
{
  bool ok;
  int magnification = QInputDialog::getInt(this, tr("Enter Magnification"),
    tr("Magnification Factor:"),1,1 , 50, 1, &ok);

  if (ok)
  {

#if 0

    QRect size= ui.TopoView->geometry();
    int len  = size.width()*size.height() * 3 *magnification*magnification ;
    int h = -1;
    int w = -1;
    unsigned char * buf = new unsigned char[len];
    ui.TopoView->getRenderedImage(magnification,buf,len,&w,&h);
    if (w>0 && h>0)
    {

      typedef ::itk::RGBPixel<unsigned char> RGBPixelType;
      typedef itk::Image< RGBPixelType, 2 > ImageType;
      typedef ::itk::ImportImageFilter<RGBPixelType,2> ColorImageImportType;
      typedef itk::ImageFileWriter< ImageType > WriterType;

      ColorImageImportType::Pointer m_ColorImage;
      m_ColorImage = ColorImageImportType::New();
      ColorImageImportType::RegionType::SizeType si;
      si[0]=w;
      si[1]=h;
      ColorImageImportType::RegionType reg(si);

      m_ColorImage->SetRegion(reg);

      m_ColorImage->SetImportPointer(reinterpret_cast<RGBPixelType*>(buf),w*h,true);

      m_ColorImage->Update();

      WriterType::Pointer writer = WriterType::New();
      QString fileName = QFileDialog::getSaveFileName(this,tr("File Name"),tr(""),tr("Image Files (*.png *.jpg *.bmp *.tif)") );
      if (fileName.length() > 4)
      {
        writer->SetInput(m_ColorImage->GetOutput());
        writer->SetFileName(fileName.toStdString());
        writer->Update();
      }
    }
#else
    QString fileName = QFileDialog::getSaveFileName(this,tr("File Name"),tr(""),tr("Image Files (*.png *.jpg *.bmp *.tif)") );
    if (fileName.length() > 4)
    {
     ui.TopoView->saveImage(fileName,magnification);
    }
#endif
  }
}

void NFEvalGUIMainWindow::addLogEntry( QString str )
{
  ui.logViewer->appendPlainText(str);
}

void NFEvalGUIMainWindow::saveParams()
{
  NF::NFParameterSet::Pointer params = ui.m_AlgoParamsEdit->getParameterSet();
  saveParameterSet(params);
}

void NFEvalGUIMainWindow::loadParams()
{
  QString fn = QFileDialog::getOpenFileName(this,QString("Load ParameterSet"),QString("D:\\"), tr("xml (*.npsx);;text (*.npst);;All Files (*.*)"));

  std::string stdFn=fn.toStdString();

  if(stdFn.size() > 4)
  {
    NF::NFParameterSetIOFactory::Pointer IOFactory =  NF::NFParameterSetIOFactory::New();
    const NF::NFParameterSetIOFactory::LightObjectListType &li= IOFactory->getLightObjectList();
    NF::NFParameterSetIOFactory::LightObjectListType::const_iterator it = li.begin();

    bool ok=false;
    for(;false==ok && li.end()!=it; ++it)
    {
      NF::NFParameterSetIOFactory::ObjectType temp(*it);
      ok=temp->canRead(stdFn);
      if(true==ok)
      {
       // temp->setParameterSet(params);
        temp->setSource(stdFn);
        temp->read();
        NF::NFParameterSet::Pointer readPset = temp->getReadParameterSet();
        NF::NFParameterSet::Pointer curPset=  ui.m_AlgoParamsEdit->getParameterSet();
        NF::NFParameterSet::Pointer newPset = NF::NFParameterSet::New();
        if (curPset.get() != NULL)
        {
          NF::NFParameterSet::const_iterator it = curPset->begin();
          NF::NFParameterSet::const_iterator itEnd = curPset->end();
          while(it!=itEnd)
          {
            if (readPset->containsParameter(it->first))
            {
              newPset->setParameter(it->first,readPset->getParameter(it->first));
            }
            else
            {
              newPset->setParameter(it->first,it->second);
            }
            ++it;
          }
        }
        
        ui.m_AlgoParamsEdit->setParameterSet(newPset);
      }
    }
  }
}

void NFEvalGUIMainWindow::writeSettings()
{
  QSettings settings(AppName, QSettings::IniFormat);

  settings.beginGroup("paths");
  settings.setValue(LAST_OPEN_DIR_TOPO, m_LastOpenDirTopo);
  settings.endGroup();
}

void NFEvalGUIMainWindow::readSettings()
{
  QSettings settings(AppName, QSettings::IniFormat);

  settings.beginGroup("paths");
  m_LastOpenDirTopo = settings.value(LAST_OPEN_DIR_TOPO,"D:/messungen/").toString();
  settings.endGroup();
}

void NFEvalGUIMainWindow::mousePositionSlot(double xPos, double yPos, double zPos, unsigned int colX, unsigned int rowY, double heightVal, int intensityVal, unsigned int maskVal, int colorVal )
{
  QString s;

  s.append(" X: ");
  s.append(QString::number(xPos));
  s.append(" Y: ");
  s.append(QString::number(yPos));


  s.append(" X: ");
  s.append(QString::number(colX));
  s.append(" Y: ");
  s.append(QString::number(rowY));

  s.append(" Z: ");
  s.append(QString::number(heightVal));
  s.append(" I: ");
  s.append(QString::number(intensityVal));
  s.append(" M: ");
  s.append(QString::number(maskVal));
  s.append(" C: ");
  s.append(QString::number(colorVal));
  ui.mousePosInfo->setText(s);
}

void NFEvalGUIMainWindow::mouseLeftButtonDblClickInSceneSlot(double xPos, double yPos)
{
  qDebug() << "mouseLeftDoubleClickEvent";
  NFTopoViewer *topoViewer = ui.TopoView->getTopoViewer(0);
  NF::NF_SYSTEM_SIZE_TYPE id = 0;

  if (topoViewer)
  {
    NF2DTopoViewer *topo2DViewer = topoViewer->getTopoViewer2D();
    if (topo2DViewer)
    {
      NFObjectID *objID = topo2DViewer->getItemAtPos(xPos, yPos);
      if (objID)
      {
        id = objID->getID();
      }

      const bool ret = topo2DViewer->setCurrentTopoItemForProfileLine(objID);
      NFObjectID *topoItemID = topo2DViewer->getCurrentTopoItemForProfileLine();

      NFObjectID * topoObjId1 = topo2DViewer->getTopo(0);
      NFObjectID * topoObjId2 = topo2DViewer->getTopoByID(objID);

      if (topoObjId1)
      {
        NFTopography::Pointer topo1 = reinterpret_cast<NFTopography*>(topoObjId1->mID);
      }

      if (topoObjId2)
      {
        NFTopography::Pointer topo2 = reinterpret_cast<NFTopography*>(topoObjId2->mID);
      }

      QVariant var = topo2DViewer->getItemsAtPos(xPos, yPos, true);
      QVariant::Type type = var.type();
      QList<QVariant> list = var.toList();
      for (int i = 0; i < list.size(); ++i)
      {
        bool b = list.at(i).canConvert<NFObjectID*>();
        NFObjectID *objId = list.at(i).value<NFObjectID*>();
        int k = 0;
      }
    }
  }
}

void NFEvalGUIMainWindow::mouseButtonPressInSceneSlot(double xPos, double yPos, NF2DTopoViewer::NFUserMouseButton btn)
{
  qDebug() << "mousePressEvent: " << btn;
}

void NFEvalGUIMainWindow::mouseButtonReleaseInSceneSlot(double xPos, double yPos, NF2DTopoViewer::NFUserMouseButton btn)
{
  qDebug() << "mouseReleaseEvent: " << btn;
}

void NFEvalGUIMainWindow::lineChangedSlot(NFObjectID* topoId, NFObjectID* lineId, double startX, double startY, double endX, double endY)
{
  NF::NF_SYSTEM_SIZE_TYPE id1 = 0;
  NF::NF_SYSTEM_SIZE_TYPE id2 = 0;

  if (topoId)
  {
    id1 = topoId->getID();
  }

  if (lineId)
  {
    id2 = lineId->getID();
  }
}

void NFEvalGUIMainWindow::clearHistory()
{
  try
 {
   int i =0;
	 while(true)
   {
     ui.AlgoHistroy->setCurrentRow(i++);
     
     AlgoListItem* itm= dynamic_cast<AlgoListItem*>(ui.AlgoHistroy->currentItem());
     if (NULL == itm)
     {
       throw(2);
     }
     if (NULL == itm->Algo)
     {
         throw(2);
     }
     itm->Algo = 0;
     itm->ParamSet = 0;
     ui.AlgoHistroy->removeItemWidget(ui.AlgoHistroy->currentItem());
   }
 }
 catch (...)
 {
 	int i=0;
  ++i;
 }
 ui.AlgoHistroy->clear();
}

void NFEvalGUIMainWindow::saveOutputParams()
{
  NF::NFParameterSet::Pointer params = ui.OutputParams->getParameterSet();
  saveParameterSet(params);
}

void NFEvalGUIMainWindow::saveParameterSet( NF::NFParameterSet::Pointer pset )
{
  QString fn = QFileDialog::getSaveFileName(this,QString("Save ParameterSet"),QString("D:\\"), tr("xml (*.npsx);;text (*.npst);;All Files (*.*)"));

  std::string stdFn=fn.toStdString();

  if(stdFn.size() > 4)
  {
    NF::NFParameterSetIOFactory::Pointer IOFactory =  NF::NFParameterSetIOFactory::New();
    const NF::NFParameterSetIOFactory::LightObjectListType &li= IOFactory->getLightObjectList();
    NF::NFParameterSetIOFactory::LightObjectListType::const_iterator it = li.begin();

    bool ok=false;
    for(;false==ok && li.end()!=it; ++it)
    {
      NF::NFParameterSetIOFactory::ObjectType temp((*it)->createAnother());
      ok=temp->canWrite(stdFn);
      if(true==ok && pset.get() != NULL)
      {
        temp->setParameterSetToBeWritten(pset);
        temp->setDestination(stdFn);
        temp->write();
      }
    }
  }
}

void NFEvalGUIMainWindow::reinit3DView()
{
  bool useTdx = ui.chk_EnableTdx->isChecked();
  /*ui.TopoView->setUseTdx(useTdx);
  ui.TopoView->reinit();*/
}

void NFEvalGUIMainWindow::abortEval()
{
//#if NFEVAL_VERSION_MAJOR > 7 || (NFEVAL_VERSION_MAJOR == 7 && NFEVAL_VERSION_MINOR >= 3)
  thread->eval->abortEvaluation();
//#endif
}

void NFEvalGUIMainWindow::setView( int currentView )
{
  if (currentView<2)
  {
    ui.TopoView->setNumberOfViews(currentView+1);
  }else
  {
    ui.TopoView->setNumberOfViews(2);
  }
}

/*! setProfile() is activated by the checkBox profileOn in the GUI and 
activates the profile when checked and disactivates when unchecked.
@param idxView ###
@return void
*/
void NFEvalGUIMainWindow::setProfile( int idxView /*= -1*/ )
{
  if (ui.profileOn->isChecked())
  {
    if (ui.histogramOn->isChecked())
    {
      ui.histogramOn->setCheckState(Qt::Unchecked);
    }
    if (ui.xProf->isChecked())
    {
      ui.TopoView->setLowerWindow(0, 0);
    }
    else
    {
      ui.TopoView->setLowerWindow(0,1);
    }    
  } 
  else
  {
    ui.TopoView->setLowerWindow(2,0);
  }
}

void NFEvalGUIMainWindow::setHistogram( int idxView /*= -1*/ )
{
  if (ui.histogramOn->isChecked())
  {
    if (ui.profileOn->isChecked())
    {
      ui.profileOn->setCheckState(Qt::Unchecked);
    }
    ui.TopoView->setLowerWindow(1,0);
  }
  else
  {
    ui.TopoView->setLowerWindow(2,0);
  }
}

void NFEvalGUIMainWindow::setProfileAxis( )
{
  if (ui.profileOn->isChecked())
  {
    setProfile(0);
  }
}

void NFEvalGUIMainWindow::displayColorChanged( QString s )
{
  ui.TopoView->setTopoToColorEvalByName(s,QString(),-1);
}

void NFEvalGUIMainWindow::Mode2Dchanged()
{
  NFTopoViewer * topoViewer = ui.TopoView->getTopoViewer(0);
  NF2DTopoViewer * topo2DViewer = 0;

  if (topoViewer)
  {
    topo2DViewer = topoViewer->getTopoViewer2D();
  }

  if (ui.rbt_moveOrScale->isChecked())
  {
   if (topo2DViewer)
   {
     topo2DViewer->setMode(NF2DTopoViewer::moveOrScaleItems);
   }
  }
  else if (ui.rbt_drawRect->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawRect);
    }
  }
  else if (ui.rbt_drawTopoRect->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawTopoRect);
    }
  }
  else if (ui.rbt_drawOrEditSingleRect->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawOrEditSingleRect);
    }
  }
  else if (ui.rbt_drawOrEditSingleTopoRect->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawOrEditSingleTopoRect);
    }
  }
  else if (ui.rbt_drawLine->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawLine);
    }
  }
  else if (ui.rbt_drawTopoLine->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawTopoLine);
    }
  }
  else if (ui.rbt_drawOrEditSingleLine->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawOrEditSingleLine);
    }
  }
  else if (ui.rbt_drawOrEditSingleTopoLine->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawOrEditSingleTopoLine);
    }
  }
  else if (ui.rbt_drawCircle->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawCircle);
    }
  }
  else if (ui.rbt_drawTopoCircle->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawTopoCircle);
    }
  }
  else if (ui.rbt_drawOrEditSingleCircle->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawOrEditSingleCircle);
    }
  }
  else if (ui.rbt_drawOrEditSingleTopoCircle->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawOrEditSingleTopoCircle);
    }
  }
  else if (ui.rbt_drawPoint->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawPoint);
    }
  }
  else if (ui.rbt_drawTopoPoint->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawTopoPoint);
    }
  }
  else if (ui.rbt_drawOrEditSinglePoint->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawOrEditSinglePoint);
    }
  }
  else if (ui.rbt_drawOrEditSingleTopoPoint->isChecked())
  {
    if (topo2DViewer)
    {
      topo2DViewer->setMode(NF2DTopoViewer::drawOrEditSingleTopoPoint);
    }
  }
}

void NFEvalGUIMainWindow::newLogMessage( int LogLevel, const char* context, const char *message, const NFLogLocationInfo & loc, long long timeStamp, long long timeAfterStart )
{
  QString msg(message);
  emit appendLogMsg(msg);
}

void NFEvalGUIMainWindow::evalInit()
{
  NFEvalInit();
  ui.AvailableAlgosList->refreshAlgoList();
}

void NFEvalGUIMainWindow::evalDestroy()
{
  NFEvalDestroy();
  ui.AvailableAlgosList->refreshAlgoList();
}
