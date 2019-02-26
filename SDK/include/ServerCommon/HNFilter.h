#pragma once
#include <string>
#include "HNTree.h"

class AFX_EXT_CLASS HNFilter
{
private:
	HNTree m_tree;
	
public:
	HNFilter();

	void load(const char* fileName);
	bool m_initialized;
	bool censor(string& source,bool bStop=true);

	void OnUnInit(void);
};

