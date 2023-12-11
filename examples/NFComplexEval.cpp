#include "NFComplexEval.h"


#include <string>
#include "NFEval.h"
#include "NFEvaluation.h"
#include "NFEvaluationFactory.h"
#include "NFComplexEvaluation.h"
#include "NFFileReader.h"
#include "NFAlgoIOFactory.h"

using namespace NF;

static const char* NFEVAL_TEST_DIR = ".";
static const char* NFEVAL_TEST_TOPO_FILENAME = "TestData01.fits";
 
 
void SaveCompexEval(NFComplexEvaluation::Pointer &p1, std::string stdFn )
{
  NF::NFAlgoIOFactory::Pointer IOFactory = NF::NFAlgoIOFactory::New();
  const NF::NFAlgoIOFactory::LightObjectListType &li = IOFactory->getLightObjectList();
  NF::NFAlgoIOFactory::LightObjectListType::const_iterator it = li.begin();

  bool ok=false;
  for(;false==ok && li.end()!=it; ++it)
  {
    NF::NFIEvaluationIo::Pointer temp(it->get());
    ok=temp->canWrite(stdFn);
    if(true==ok && p1.get() != NULL)
    {
      temp->setEval(p1.get());
      temp->setDestination(stdFn);
     
      temp->write();
    }
  }
}


NFLightObject::Pointer ReadCompexEval(std::string stdFn)
{
  NFLightObject::Pointer ret;
  NF::NFAlgoIOFactory::Pointer IOFactory = NF::NFAlgoIOFactory::New();
  const NF::NFAlgoIOFactory::LightObjectListType & li = IOFactory->getLightObjectList();
  NF::NFAlgoIOFactory::LightObjectListType::const_iterator it = li.begin();

  bool ok=false;
  for(;false==ok && li.end()!=it; ++it)
  {
    NF::NFIEvaluationIo::Pointer temp(it->get());
    ok=temp->canRead(stdFn);
    if(true==ok )
    {
      temp->setSource(stdFn);
      temp->read();
      ret=temp->getEval();
    }
  }
  return(ret);
}


// 
int createNFComplexEval(int argc, char* argv[])
{
    NFEvalInit(); // load all plugins

    // Get filename
    std::string fileName(NFEVAL_TEST_TOPO_FILENAME);
        
    NFComplexEvaluation::Pointer complexEval = NFComplexEvaluation::New();
    
    int evalResult = -1;

       //(1) generate  Input topo node
    NFTopography::Pointer topo = NFTopography::New();
    size_t evalId = 0;
    complexEval->addInputTopoConnection(evalId, 0, 0);

     

    // (2) generate and set filter-evaluation
    const char *FilterName = "NFGaussianFilter";
    NFEvaluationFactory::Pointer fac = NFEvaluationFactory::New();
    NFEvaluation::Pointer eval = NFEvaluation::Pointer(fac->getObjectByName(FilterName));
    NFParameterSet::Pointer para = eval->getParameterSet();
    eval->setParameterSet(para);
    eval->setInputTopo(topo);

    evalId = complexEval->addNextEval(evalId,eval);

    // (3) generate and set 2. filter-evaluation
    FilterName = "NFMedianFilter";
    eval = NFEvaluation::Pointer(fac->getObjectByName(FilterName));
    para = eval->getParameterSet();
    para->setParameter("HeightRadiusPixel", int(7));
    eval->setParameterSet(para);
    eval->setInputTopo(topo);

    evalId = complexEval->addNextEval(evalId,eval);
    complexEval->addOutputTopoConnection(evalId);

    
    std::string nedFile("testIO.ned" );
    
    SaveCompexEval(complexEval,nedFile);

   
     
   
  return(0);
}

 