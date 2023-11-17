#pragma once
#include "NFEval_Config.h"
#include "NFEvaluation.h"
#include "NFMacros.h"

namespace NF
{

#ifdef SWIG
  %template(NFConturPointer)NF::NFSmartPointer<NF::NFContur>;
#endif


class NFContour : public NFEvaluation
{
  NF_OBJECT(NFContour);

public:
  virtual ::NF::NFParameterSet::Pointer getDefaultParameterSet() const;

protected:
  
 NFContour();
  ~NFContour();

  virtual int generateOutput();

private:

   int  createColorTopo(const std::string &fileName);
};

}