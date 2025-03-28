#pragma once
#include <inttypes.h>

class bhRandom
{
public:

  bhRandom();
  int GetNext();

protected:
private:

  int a, b;
};
