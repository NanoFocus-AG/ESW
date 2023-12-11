#include "NFEval.h"
#include "NFEvaluation.h"
#include "NFEvaluationFactory.h"
#include "NFFileWriter.h"
#include "NFFileReader.h"

#include <iostream>
#include <string>


#include "NFComplexEval.h"


/* (c) NanoFocus AG, 2023
 leitz@nanofocus.de
 example01
 small demo to show  loading  a measurement file from disk
 the working directory has to be the parent directory of Plugins directory
*/
int example01(int argc, char *argv[])
{

  changeRootLogLevelTemporarily(5); // a log file log.txt will be written

  int rc = NFLoadPlugins("Plugins"); // load all NFEval Plugins  in  subfolder  Plugins

  NF::NFTopography::Pointer topo = NF::NFTopography::New();

  NF::NFFileReader::Pointer reader = NF::NFFileReader::New();

  std::string fname = "TestData01.fits";

  reader->setFileName(fname);

  rc = reader->evaluate();

  if (0 == rc)
  {
    topo = reader->getOutputTopo();

    std::cout << "NX  = " << topo->getCountX() << " px \n";
    std::cout << "NY  = " << topo->getCountY() << " px \n";

    std::cout << "lx  = " << topo->getCountX() * topo->getDeltaXInMillimeter() << " mm \n";
    std::cout << "ly  = " << topo->getCountY() * topo->getDeltaYInMillimeter() << " mm \n";

    for (NF::NF_INT64 y = 0; y < topo->getCountY(); ++y)
    {
      for (NF::NF_INT64 x = 0; x < topo->getCountX(); ++x)
      {
        NF::NFHeightType h = topo->getHeightAt(x, y);
        NF::NFIntensityType i = topo->getIntensityAt(x, y);
        NF::NFMaskType m = topo->getMaskAt(x, y);
      }
    }

    NF::NFParameterSet::Pointer metaInfos = topo->getMetaData();

    auto element = metaInfos->begin();
    while (element != metaInfos->end())
    {
      std::string key = element->first;
      NF::NFVariant value = element->second;

      std::cout << "\n" << key << " = " << value.valueToString() << "\n";

      ++element;
    }
  }

  return 0;
}

/* (c) NanoFocus AG, 2023
 leitz@nanofocus.de
  example02
   use of new plugin  NFContour , executing mahr script
 the working directory has to be the parent directory of Plugins directory
*/
int example02(int argc, char *argv[])
{
  changeRootLogLevelTemporarily(5); // a log file log.txt will be written

  int rc = NFLoadPlugins("Plugins"); // load all NFEval Plugins  in  subfolder  Plugins

  NF::NFTopography::Pointer topo = NF::NFTopography::New();

  NF::NFFileReader::Pointer reader = NF::NFFileReader::New();

  std::string fname =
      "C:\\Mahr\\Users\\markus\\Profiles\\Mikrokonturnormal\\Mikrokonturnormal_5040PTB05_Precitec_CLA2_L1.fits";

  reader->setFileName(fname);

  rc = reader->evaluate();

  if (0 == rc)
  {
    topo = reader->getOutputTopo();

    NF::NFEvaluationFactory::Pointer factory = NF::NFEvaluationFactory::New();

    NF::NFEvaluation::Pointer ContourAlgo = factory->getObjectByName("NFContour").get();

    if (ContourAlgo.isValid())
    {

      ContourAlgo->setInputTopo(topo);
      rc = ContourAlgo->evaluate();

      NF::NFParameterSet::Pointer resultParameter = ContourAlgo->getOutputParameterSet();

      auto element = resultParameter->begin();
      while (element != resultParameter->end())
      {
        std::cout << element->first << " " << element->second.toJSON() << "\n";

        ++element;
      }
    }

    NF::NFFileWriter::Pointer writer = NF::NFFileWriter::New();

    writer->setFileName("m:\\test_data\\contimage.fits");
    writer->setInputTopo(ContourAlgo->getOutputTopo(1));
    bool success = writer->evaluate();
  }

  return rc;
}

int main(int argc, char *argv[])
{


   int rc = createNFComplexEval(argc, argv);

   rc = example01(argc, argv);


  return 0;
}
