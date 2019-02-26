/*
 * Copyright (c) 2002-2008 Fenomen Games.  All Rights Reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * Support: pyro-support@fenomen-games.com
 *
 */

#ifndef _PYRO_TYPES_H_INCLUDED
#define _PYRO_TYPES_H_INCLUDED

#pragma once

#include <exception>

typedef unsigned int PyroDWord;
typedef signed int PyroResult;
typedef int PyroBool;
typedef unsigned char PyroByte;

#define PyroFalse	0
#define PyroTrue	1

#define PyroOK		0
#define PyroFailed	-1

#define PyroIsFailed(r)		(((PyroResult) (r)) < 0)

namespace PyroParticles
{

class CPyroException : public std::exception
{
	char *m_pMessage;
            
public:

	PYROAPI CPyroException();

	PYROAPI CPyroException(const CPyroException &Exception);

	PYROAPI CPyroException(const char *pError, ...);

	~CPyroException() throw()
	{
		if (m_pMessage)
		{
			free(m_pMessage);
			m_pMessage = NULL;
		}
	}

    
	PYROAPI const char *GetExceptionMessage() const;

	virtual const char *what() const throw()
	{
		return m_pMessage;
	}

};

}

#endif /* _PYRO_TYPES_H_INCLUDED */
