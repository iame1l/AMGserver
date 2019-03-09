/****************************************************************************
Copyright (c) 2014-2017 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2017 ShenZhen Redbird Network Polytron Technologies Inc.
 
http://www.hotniao.com

All of the content of the software, including code, pictures, 
resources, are original. For unauthorized users, the company 
reserves the right to pursue its legal liability.
****************************************************************************/
#ifndef HN_ALGORITHM_H
#define HN_ALGORITHM_H

#include <random>
#include <algorithm>
#include <vector>
#include <map>
#include <ShlObj.h>
#include <sstream>
#include <string>
using namespace std;

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

class Algor
{
public:
	static int rand_Mersenne(const int _Min, const int _Max)
	{
		if (_Min > _Max){ return 0; }
		if (_MSC_VER < 1500){ return 0; }
		random_device rd;
		mt19937 mt(rd());
		uniform_int_distribution<> dis(_Min, _Max);
		return dis(mt);
	}
	template<class _T,class _Pred>
	static void vecor2map(const vector<_T>& v,map<_T,size_t>&m,_Pred pFun)
	{
		for (auto i = v.begin();i != v.end();i++)
		{
			if (pFun(*i))
			{
				m[*i]++;
			}	
		}
	}

	template<class _T>
	static void vecor2map(const vector<_T>& v,map<_T,size_t>&m)
	{
		for (auto i = v.begin();i != v.end();i++)
		{
			m[*i]++;
		}
	}

	template<class _T>
	static vector<size_t> getMaxIndex(_T *array,size_t size)
	{
		vector<size_t> maxIndex;
		maxIndex.clear();
		if(nullptr == array) return maxIndex;

		_T _max = array[0];
		for (size_t i = 1;i < size;i++)
		{
			if(_max < array[i])
				_max = array[i];
		}
		for (size_t i = 0;i < size;i++)
		{
			if(_max == array[i])
				maxIndex.push_back(i);
		}
		return maxIndex;
	}

	template<class _T,class TD>
	static void getRankIndex(_T *array,size_t size,TD *index,TD defVal)
	{
		if(nullptr == array || nullptr == index) return ;
		vector<_T> tmp;
		tmp.assign(array,array + size);
		sort(tmp.begin(),tmp.end());
		TD idx = 0;
		_T max = tmp.back();
		for (size_t i = 0;i < size ;i++)
			index[i] = defVal;
		do 
		{	
			for (size_t i = 0;i < size ;i++)
			{
				if(defVal != index[i])continue;
				if(array[i] == tmp.back())
				{
					if(max > tmp.back() )
					{
						idx ++;
					}
					index[i] = idx;
					max = tmp.back();
					tmp.pop_back();
					break;
				}
			}
		} while (!tmp.empty());	
	}

	template<typename T>
	static string strFormat(const T *array, size_t size)
	{
		string str;
		stringstream stream;
		for (size_t i = 0; i < size; i++)
		{
			string tname = typeid(array[i]).name();
			if (tname == "char"
				|| tname == "unsigned char")
			{
				stream <<hex<< (int)array[i] << ",";
			}
			else
			{
				stream << array[i] << ",";
			}

		}
		str += stream.str();
		return str;
	}

	/*
	*@brief:   遍历所有组合的可能性
	*@Returns:   void
	*@Parameter: int iNum
	*@Parameter: vector<T> & v
	*@Parameter: vector<vector<T>> & des
	*@Parameter: const vector<T> & DSES
	*/
	template<typename T>
	static void gen(int iNum , vector<T>& v,vector<vector<T>>& des,const vector<T>& DSES)
	{
		if (iNum == 0)
		{	 
			des.push_back(v);
			return;
		}
		for (auto itr = DSES.begin();itr != DSES.end();itr++)
		{
			v.push_back(*itr);
			gen(iNum -1, v,des, DSES);
			v.pop_back();
		}
	}

	template<typename T>
	static int change(vector<T>& v,const vector<T>& DSES)
	{
		int size = 0;
		for (auto itr = v.begin();itr != v.end();)
		{
			if(find(DSES.begin(),DSES.end(),*itr) != DSES.end())
			{
				itr = v.erase(itr);
				size++;
			}
			else
				itr++;
		}
		return size;
	}
};



#endif	//HN_ALGORITHM_H