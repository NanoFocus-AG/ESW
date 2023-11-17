

/*

Ticket #6123
Es ist ein Export konform zu Mahr Profildateien zu schreiben. Dateiendung *.prf

Im Anhang eine Beispieldatei (989439604188006112011190730701035751_Kontur_1_filtered.txt). Nach erfolgreicher Implementierung in den Trunk muss die Datei
auf 8.6 zurück portiert werden. Weitere Informationen folgen.

Nach Reverse-Engineering der Textdatei:

    Die Daten sind im X-/Y-/Z-Format abgelegt
    Spalte 1: Fortlaufende Nummer, Anschließendes Trennzeichen ist '='
    Spalte 2: Tischposition X-Achse zu jedem Messwert in mm, Dezimaltrennzeichen = "."
    Spalte 3: Tischposition Y-Achse zu jedem Messwert
    Spalte 4: Tischposition Z-Achse+Höhenwert des Sensors in mm
    Spalte 5: Wahrscheinlich weitere Achse (Hier immer 0.000000000)
    Spalte 6: 0



*/

#include "NFPrfIO.h"
#include <iomanip>
#include "NFEvaluationFactory.h"

#ifdef COMPILE_WITH_BOOST
#include "boost/date_time/posix_time/posix_time.hpp" //include all types plus i/o
using namespace boost::posix_time;
#endif
namespace NF
{
log4cxx::LoggerPtr prDatLogger = getNFLogger("NFEvaluation.FileIO.NFPrfIO");

std::string Header_Template = R"([PROFILE_HEADER]
NAME=@FILENAME@
PROF_CLASS=[p3dlin]
PROF_TYPE=3002
INTERVAL_LIN=0.001000000
PRETRAV_LIN=0.000000000
POSTTRAV_LIN=0.000000000
CAXIS_ACTIV=0
NO_POINTS=@POINTS@
NO_TOT_POINTS=@POINTS@
PB_RADIUS=0.499978420
MA=0
F=0.010000000
PB_UX=0.000000000
PB_UY=0.000000000
PB_UZ=-1.000000000
START_X=@START_X@
START_Y=@START_Y@
START_Z=@START_Z@
START_C=0.000000000
START_HB=0.000000000
START_TX=@START_X@
START_TY=@START_Y@
STOP_X=48.997336000
STOP_Y=0.000000000
STOP_Z=-0.930153000
STOP_C=0.000000000
STOP_HB=0.000000000
STOP_TX=0.000000000
STOP_TY=84.700000000
RANGE=41.033100000
TEMP=-1001.000000000
[PROFILE_HISTORY]
ProductName=evaluation
ProductVersion=8.00
DateTime=@DATE@ (Mitteleuropäische Sommerzeit)
ModuleName=SCAN REL
MachineName=DriveUnit.PCV
MachineNumber=273605
ProbeName=PCv 350/36/25 ARM-ID. 9048600
TipName=Kugeltaser 1mm unten
ProbeRadius=0.500000000000
ProbesystemName=PCV-Probesystem:1
VLIN=0.500000000000
ALIN=30.000000000000
accuracy.x=0.001000000000
range.x=200.000000000000
sampling correction done on W and Z values.
Probe radius changed to acutal value - former value=0.500000000000
OBJECT=SDI PHEV 2
NUMBER=
INSPECTOR=nn
OPERATION=
TEXT_1=
TEXT_2=
TEXT_3=
TEXT_4=
TEXT_5=
COMMENTCOUNT=1
COMMENT_0=optischer Taster
Profile.Type=Surface
[PROFILE_VALUES]
// X,Y,Z,C,Lock)";

NFPrfIO::NFPrfIO(void)
{
  setNumberOfOutputTopos(1);
}

NFPrfIO::~NFPrfIO(void)
{
}

int NFPrfIO::generateOutput()
{
  return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// file reading
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NFPrfIO::canRead(const std::string &FileName) const
{
  bool ret = false;

  return (ret);
}

//! Reads image- / Topo-information
bool NFPrfIO::readInformation()
{
  return (true);
}

//! reads image / Topo
bool NFPrfIO::read()
{
  try
  {
    // TODO: eigentlich soll nur requested region gelesen werden ???
    NFTopography::Pointer topo = this->getOutputTopo(0);

    this->setOutputTopo(topo, 0);

    NFLOG_INFO(prDatLogger, "Finished reading file '" << getFileName().c_str() << "' as  pr dat file");

    // Fire progress event
    // setProgressAndInvoke(progressEnd);
  }
  catch (NFException &e)
  {
    e.what();
    return (false);
  }
  catch (...)
  {
    return (false);
  }
  return (true);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// file writing
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//! returns true if File can be written
bool NFPrfIO::canWrite(const std::string &FileName) const
{
  bool ret = false;
  std::string extension("prf");

  if (FileName.length() > extension.size())
  {
    std::string str(FileName);
    str = FileName.substr(str.length() - extension.size(), extension.size());
    ret = (Utils::caseInsCompare(extension, str));
  }

  return (ret);
}

void replaceInHeader(std::string &source, const std::string &tag, const std::string &replaceWith)
{

  auto pos = source.find(tag, 0);
  while (pos != std::string::npos)
  {
    source.replace(pos, tag.length(), replaceWith);
    pos += replaceWith.length();
    pos = source.find(tag, pos);
  }
}

//! writes image / Topo
bool NFPrfIO::write()
{
  try
  {

    NFTopography::Pointer topo = this->getInputTopo(0);

    if (0 == topo.get()) return (false);
    if (0 == topo->getCountX() || 0 == topo->getCountY()) return (false);

    bool isXProfile = (1 == topo->getCountY() && topo->getCountX() >= 1); // a single point is defined as x-profile
    bool isYProfile = (1 == topo->getCountX() && topo->getCountY() > 1);


    std::string header(Header_Template);

    replaceInHeader(header, "@FILENAME@", getFileName());
    replaceInHeader(header, "@POINTS@", std::to_string(  isXProfile == true ? topo->getCountX():topo->getCountY()));

#ifdef COMPILE_WITH_BOOST
    replaceInHeader(header, "@DATE@", to_simple_string(second_clock::local_time()));
#endif
    replaceInHeader(header, "@START_X@", std::to_string(topo->getOffsetX()));
    replaceInHeader(header, "@START_Y@", std::to_string(topo->getOffsetY()));
    replaceInHeader(header, "@START_Z@", std::to_string(topo->getOffsetZ()));

    std::ofstream of(getFileNameW().c_str());
    of << header << std::endl;

   
    if(isXProfile)
    {
      for (NF_INT64 ix = 0; ix < topo->getCountX(); ++ix)
      {

        auto x = topo->getOffsetX() + ix * topo->getDeltaXInMillimeter();
        auto y = topo->getOffsetY();
        auto z = topo->getHeightAt(ix, 0) / 1000.0f + topo->getOffsetZ();

        of << ix << "=" << std::setprecision(9) << std::fixed << x << " " << y << " " << z << " " << 0.0 << " " << 0 << "\n";
      }
    }
  
    if(isYProfile)
    {
      for (NF_INT64 iy = 0; iy < topo->getCountY(); ++iy)
      {
        auto x = topo->getOffsetX();
        auto y = topo->getOffsetY() + iy * topo->getDeltaYInMillimeter();
        auto z = topo->getHeightAt(0, iy) / 1000.0f + topo->getOffsetZ();

        of << iy << "=" << std::setprecision(9) << std::fixed << x << " " << y << " " << z << " " << 0.0 << " " << 0 << "\n";
      }
    }
    of.close();

    // Fire progress event
    setProgressAndInvoke(1.0f);
  }
  catch (NFException &e)
  {
    NFLOG_ERROR(prDatLogger,
                _T("Caught Exception: ") << e.what() << _T( " From Function: ") << e.function() << _T(" In File:") << e.file() << _T(" Line: ") << e.line());
    return (false);
  }
  catch (...)
  {
    NFLOG_ERROR(prDatLogger, _T("Unknown error while saving file."));
    return (false);
  }
  return (true);
}

//! write image / Topo information
bool NFPrfIO::writeInformation()
{

  return (true);
}

NFParameterSet::Pointer NFPrfIO::getDefaultParameterSet() const
{
  NFParameterSet::Pointer ret = NFFileIo::getDefaultParameterSet();
  return (ret);
}

void NFPrfIO::setProgressAndInvoke(float progress)
{
  setProgress(progress);
  invokeEvent(NFProgressEvent());
}

} // namespace NF
