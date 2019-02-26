/*
 * Copyright (c) 2002-2009 Fenomen Games.  All Rights Reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * You may freely use or modify this code
 *
 * Support: pyro-support@fenomen-games.com
 *
 */

#ifndef _PYRO_SAMPLES_COMMON_OPENGLES_H_INCLUDED
#define _PYRO_SAMPLES_COMMON_OPENGLES_H_INCLUDED

#pragma once

#include <assert.h>

#ifdef __APPLE__
#include <OpenGLES/ES1/gl.h>
#else
#include <GLES/gl.h>
#endif /* __APPLE__ */

#include "Pyro.h"

namespace PyroParticles
{

class CGraphics_OGLES;

class CTexture_OGLES : public PyroParticles::PyroGraphics::ITexture
{
	int m_glTexture;

public:

	CTexture_OGLES(PyroParticles::PyroGraphics::CBitmap *pBitmap, PyroDWord Flags);

	int GetOGLTexture() const { return m_glTexture; }

};

class CIndexBuffer_OGLES : public PyroParticles::PyroGraphics::IIndexBuffer
{
	uint16_t *m_pBuffer;

public:

	CIndexBuffer_OGLES(CGraphics_OGLES *pGraphics, PyroDWord IndexType, PyroDWord nIndexes, PyroDWord Flags)
	{
		m_pBuffer = new uint16_t[nIndexes];
	}

	~CIndexBuffer_OGLES()
	{
		delete[] m_pBuffer;
	}

	virtual PyroResult Lock() { return PyroOK; }

	virtual PyroResult Unlock() { return PyroOK; }

	virtual void *GetBuffer() { return m_pBuffer; }

	virtual PyroResult Restore() { return PyroOK; }

	virtual PyroResult Invalidate() { return PyroOK; }

};

class CVertexBuffer_OGLES : public PyroParticles::PyroGraphics::IVertexBuffer2
{
	PyroByte *m_pBuffer;
	PyroDWord m_VertexFormat;
	PyroDWord m_VertexSize;

public:

	CVertexBuffer_OGLES(CGraphics_OGLES *pGraphics, PyroDWord nVertices, PyroDWord VertexSize, PyroDWord VertexFormat, PyroDWord Flags)
	{
		m_VertexSize = VertexSize;
		m_VertexFormat = VertexFormat;

		m_pBuffer = new PyroByte[nVertices * VertexSize];
	}

	~CVertexBuffer_OGLES()
	{
		delete[] m_pBuffer;
	}

	PyroDWord GetVertexSize() const { return m_VertexSize; }
	PyroDWord GetVertexFormat() const { return m_VertexFormat; }

	virtual PyroResult CopyBufferSubData(void *pSrc, PyroDWord Offset, PyroDWord Size, PyroBool bDiscard)
	{
		memcpy(m_pBuffer + Offset, pSrc, Size);
		return PyroOK;
	}

	void *GetBuffer() {	return m_pBuffer; }

	virtual PyroResult Restore() { return PyroOK; }

	virtual PyroResult Invalidate() { return PyroOK; }

};

class CGraphics_OGLES : public PyroParticles::PyroGraphics::IDevice
{
	int m_glSrcBlendFunc;
	int m_glDestBlendFunc;

	bool m_bNullTexture[8];

	PyroParticles::PyroGraphics::IIndexBuffer *m_pQuadIndexBuffer;
	
	void CreateQuadIndexBuffer(int nQuads);
	
public:

	virtual PyroResult SetBlendFunc(
		PyroParticles::PyroGraphics::IDevice::EBlend SrcBlend,
		PyroParticles::PyroGraphics::IDevice::EBlend DestBlend);

	virtual PyroResult SetRenderState(PyroDWord State, PyroDWord Value);

	virtual PyroResult SetWorldMatrix(float WorldMatrix[3][4]);

	virtual PyroResult SetTexture(PyroDWord Stage, PyroParticles::PyroGraphics::ITexture *pTexture);

	virtual PyroResult CreateVertexBuffer2(
		PyroParticles::PyroGraphics::IVertexBuffer2 **ppVertexBuffer,
		PyroDWord Vertices,
		PyroDWord VertexSize,
		PyroDWord VertexFormat,
		PyroDWord Flags);

	virtual PyroResult CreateIndexBuffer(
		PyroParticles::PyroGraphics::IIndexBuffer **ppIndexBuffer,
		PyroDWord IndexType,
		PyroDWord nIndexes,
		PyroDWord Flags);

	virtual PyroResult CreateTexture(
		PyroParticles::PyroGraphics::ITexture **ppTexture,
		PyroParticles::PyroGraphics::CBitmap *pBitmap,
		PyroDWord Flags);
    
	virtual PyroResult CreateBitmap(
		PyroParticles::PyroGraphics::CBitmap **ppBitmap,
		PyroByte *pBuffer,
		PyroDWord Width,
		PyroDWord Height,
		PyroDWord Flags);

public:

	CGraphics_OGLES(int nMaxQuads)
	{
		m_glSrcBlendFunc = GL_ONE;
		m_glDestBlendFunc = GL_ONE;
		
		CreateQuadIndexBuffer(nMaxQuads);
	}

	virtual PyroResult RenderQuads2(
		PyroParticles::PyroGraphics::IVertexBuffer2 *_pVertexBuffer,
		PyroDWord nFirst,
		PyroDWord nQuads,
		PyroDWord Flags);

	virtual PyroBool IsRGBA() const { return PyroTrue; }

};

}

#endif /* _PYRO_SAMPLES_COMMON_OPENGLES_H_INCLUDED */
