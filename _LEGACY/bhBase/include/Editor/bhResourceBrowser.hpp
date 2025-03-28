#ifndef BH_RESOURCE_BROWSER_HPP
#define BH_RESOURCE_BROWSER_HPP

//#include "bhDefines.h"
#include "bhHash.hpp"

class bhResourceBrowser
{
public:
	void Init();
	void Display(bool* show);

protected:
private:
	void AddTableEntry(bhHash_t hash, const char* info);
};

#endif //BH_RESOURCE_BROWSER_HPP
