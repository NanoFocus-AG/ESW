#pragma once
#include "NFComplexEvaluation.h"
#include <string>

void SaveCompexEval(NF::NFComplexEvaluation::Pointer& , std::string);
NF::NFLightObject::Pointer ReadCompexEval(std::string stdFn);
int createNFComplexEval(int, char**);

