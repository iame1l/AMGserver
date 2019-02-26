#pragma once
#include<map>
#include<string>
//#include<unordered_map>  
using namespace std;


class HNTree;
class HNTreeNode
{
	friend class HNTree;
	typedef map<string,HNTreeNode> _TreeMap;
	typedef map<string,HNTreeNode>::iterator _TreeMapIterator;

private:
	string m_character;
	_TreeMap m_map;
	HNTreeNode* m_parent;
public:
	HNTreeNode(string character,int Length);
	HNTreeNode(){
		m_character="";
	};
	//~HNTreeNode(void);

	string getCharacter() const;
	HNTreeNode* findChild(string& nextCharacter);
	HNTreeNode* insertChild(string& nextCharacter,int Length);

	//HNTreeNode* getfirstchild();
	HNTreeNode* ReMove(void);
};

