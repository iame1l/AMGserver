#pragma once

#include "boost/serialization/serialization.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"    
#include "boost/serialization/vector.hpp" 
#include "boost/serialization/map.hpp"
#define ObjMem ar &
#define ObjSerialBegin                                                            \
	private:                                                                      \
	friend class boost::serialization::access;                                \
	template<class Archive>                                                   \
	void serialize(Archive& ar, const unsigned int version)                   \
		{
#define ObjSerialEnd                                                              \
		}                                                                         \
	public: