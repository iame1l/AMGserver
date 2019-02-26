#pragma once
#include "HNTreeNode.h"
using namespace std;

class HNTree
{
public:
	int count;    //当前查找的一个敏感词的字数
	HNTreeNode* insert(string& keyword);
	HNTreeNode* insert(const char* keyword);
	HNTreeNode* find(string& keyword);
	HNTree(){
		count = 0;
	};
private:
	HNTreeNode m_emptyRoot;
	int m_pace;
	HNTreeNode* insert(HNTreeNode* parent, string& keyword);
	HNTreeNode* insertBranch(HNTreeNode* parent, string& keyword);
	HNTreeNode* find(HNTreeNode* parent,string& keyword);
public:
	void OnUnInit(void);
};
