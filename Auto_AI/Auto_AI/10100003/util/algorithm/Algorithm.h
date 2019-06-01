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

using namespace std;
namespace HN
{
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
	};

}

#endif	//HN_ALGORITHM_H