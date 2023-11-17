#include "NFContour.h"
#include "NFEvaluationFactory.h"
#include "NFParameterSetIOFactory.h"
#include "NFParameterSetIO.h"
#include "NFParameterSetWriter.h"
#include "NFParameterSetReader.h"
#include "NFFileWriter.h"
#include <iostream>
#include <filesystem>
#include <regex>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "ini.h"

namespace NF
{
using Topo = NF::NFTopography::Pointer;

namespace fs = std::filesystem;


bool parseForErrors(const std::string &scriptStdOut)
{
 
 const static std::regex pattern = std::regex(R"((.*error.*))");

 std::smatch match;

 auto result = std::regex_search(scriptStdOut, match, pattern);

 return result;

}


NF::NFContour::NFContour()
{
  setNumberOfInputTopos(1);
  setNumberOfOutputTopos(2);
  //
  setParameterSet(this->getDefaultParameterSet());
}

NFContour::~NFContour()
{
}

NFParameterSet::Pointer NF::NFContour::getDefaultParameterSet() const
{
  NFParameterSet::Pointer params = NFParameterSet::New();

  params->setParameter("ScriptName", NFVariant("C:\\Mahr\\Users\\markus\\Scripts\\Test_FITS_with_Transfer_3.mpr",
                                               NF::NFUnitCls::NFUnitInputFileName));


  params->setParameter("MarwinRemoteEngine", NFVariant("C:\\Program Files (x86)\\Mahr\\MarWin\\RemoteMarScript.exe", NF::NFUnitCls::NFUnitInputFileName));

    

  return (params);
}
int NF::NFContour::generateOutput()
{

  using Factory_t = NF::NFEvaluationFactory::Pointer;
  using Algo_t = NF::NFEvaluation::Pointer;

  Factory_t factory = NF::NFEvaluationFactory::New();

  int rc = 0;

  // check marwin installation

  std::string marwinRemoteEngine = getParameter("MarwinRemoteEngine").getStdString();

  fs::path p = fs::path(marwinRemoteEngine);
  if (false == fs::exists(p))
  {
    return -1;
  }

  // read ini file
  // ini  file has  same stem as script file
  // located in same folder as script file
  fs::path scriptRoot = fs::path(getParameter("ScriptName").getStdString()).parent_path();
  fs::path scriptStem = fs::path(getParameter("ScriptName").getStdString()).stem();
  mINI::INIFile iniFile((scriptRoot / scriptStem).string() + ".ini");
  mINI::INIStructure iniData;
  iniFile.read(iniData);

  // save  input topo as intermediate file for  import in script
  NF::NFFileWriter::Pointer fwriter = NF::NFFileWriter::New();

  std::string fname = "m:\\test_data\\contour.fits";
  fwriter->setFileName(fname);
  fwriter->setInputTopo(this->getInputTopo());
  rc = fwriter->evaluate();

  if (0 != rc)
    return -1;

  // write ini file

  iniData["profiles"]["0"] = fname;
  iniData["export"]["csv"] = "m:\\test_data\\contour.csv";
  iniData["export"]["protocol"] = "C:\\Mahr\\Users\\markus\\Export\\protImage.png";

  iniFile.write(iniData);

  //

  // Start script

  Algo_t startMe = factory->getObjectByName("NFCreateMe").get();
  startMe->setParameter("Filename", marwinRemoteEngine);
  startMe->setParameter("Argument1", "\"" + getParameter("ScriptName").getStdString() + "\"");
  startMe->setParameter("Argument2", "-id 31");
  // startMe->setParameter("Argument3","-err2out 1");
  startMe->setInputTopo(this->getInputTopo());
  rc = startMe->evaluate();

  // check for completion of script

  if (0 != rc)
  {
    NFLOG_WARN(NF::getNFLogger("NFContour")," script did not completerd successfully");
    return -1;
  }

 
 
  setOutputParameter("stdout",startMe->getOutputParameter("stdout"));
  
  std::string stdOut = startMe->getOutputParameter("stdout").getString();
  std::cout << stdOut;
  bool hasErrors = parseForErrors(stdOut);
 
  if(true == hasErrors) 
  {
    NFLOG_WARN(NF::getNFLogger("NFContour"),"script has compile time ot runtime errors");
    NFLOG_WARN(NF::getNFLogger("NFContour"), stdOut);
    return -1;
  }

  // read results back from  result file

  NF::NFParameterSetReader::Pointer preader = NF::NFParameterSetReader::New();

  auto resultFile = iniData["export"]["csv"];
  preader->setSource(resultFile);
  bool success = preader->read();

  if (true == success)
  {
    auto pset = preader->getParameterSet();

    auto element = pset->begin();

    while (element != pset->end())
    {

      if (element->second.getType() == NFVariant::NFPARAMETERSET_POINTER)
      {

        NF::NFParameterSet::Pointer p = element->second.getParameterSet();

        auto e = p->begin();

        while (e != p->end())
        {

          setOutputParameter(e->first, e->second);

          ++e;
        }
      }

      ++element;
    }
  }

  // set output topo =  input topo

  setOutputTopo(getInputTopo(), 0);

  rc = createColorTopo(iniData["export"]["protocol"]);


  return 0;
}

/// creates color channel  of topo object from given filename.
/// special case: the color channel is half the size in y direction than source image. 
/// so the image gets cropped in y direction
int NFContour::createColorTopo(const std::string &fileName)
{

  int x = 0, y = 0, n = 3;

  stbi_set_flip_vertically_on_load(true);
  unsigned char *image = stbi_load(fileName.c_str(), &x, &y, &n, 0);

  if (image == NULL)
    return -1;


  int colorSizeY = y/2; //  only half of input image

  NF::NFTopography::Pointer outTopo = NF::NFTopography::New();

  outTopo->create(x, colorSizeY, 1, 1, 1);
  outTopo->createColors();

  auto colorContainer = outTopo->getColors();
  auto itTopoColor = colorContainer->begin();
  auto colorEnd = colorContainer->end();

  RGBA_UNION_TYPE tempInput;
 

  for (; itTopoColor != colorEnd; ++itTopoColor)
  {
    tempInput.val = *itTopoColor;

    tempInput.rgba.Red = (*image);
    ++image;
    tempInput.rgba.Green = (*image);
    ++image;
    tempInput.rgba.Blue = (*image);
    ++image;
   
    *itTopoColor = tempInput.val;
 
  }
  outTopo->setColors(colorContainer);

  setOutputTopo(outTopo, 1);

  return 0;
}




} // namespace NF
