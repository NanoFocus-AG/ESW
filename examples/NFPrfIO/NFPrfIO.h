

#pragma once

#include "NFFileIO.h"
#include "NFMacros.h"
#include "NFPrfIO_Config.h"

namespace NF
{

class NFPrfIO : public NFFileIo
{
  NF_OBJECT(NFPrfIO);

public:
  virtual bool canRead(const std::string &FileName) const;
  //! returns true if File can be written
  virtual bool canWrite(const std::string &FileName) const;
  //! reads image / Topo
  virtual bool read();

  //! writes image / Topo
  virtual bool write();
  //! Reads image / Topo information
  virtual bool readInformation();
  //! write image / Topo information
  virtual bool writeInformation();

  virtual NFParameterSet::Pointer getDefaultParameterSet() const;

protected:
  NFPrfIO(void);
  virtual ~NFPrfIO(void);
  void setProgressAndInvoke(float progress);

private:
  virtual int generateOutput();

  
};

void SetHeader();

} // namespace NF
