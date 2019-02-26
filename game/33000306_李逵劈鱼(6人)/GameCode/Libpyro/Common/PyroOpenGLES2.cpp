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

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif /* __APPLE__ */

#include "PyroOpenGLES2.h"

static GLuint LoadShader(const char *pShaderSrc, GLenum Type)
{
	GLuint Shader = glCreateShader(Type);

	if (Shader == 0)
		return 0;
		
	glShaderSource(Shader, 1, &pShaderSrc, NULL);
	glCompileShader(Shader);

	GLint Compiled;
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &Compiled);

	if (!Compiled)
	{
		GLint InfoLen = 0;
		glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &InfoLen);
	
		if (InfoLen != 0)
		{
			char *pInfoLog = (char *) malloc(sizeof(char) * InfoLen);
			glGetShaderInfoLog(Shader, InfoLen, NULL, pInfoLog);
			fprintf(stderr, "Error compiling shader:\n%s\n", pInfoLog);
			free(pInfoLog);
		}

		glDeleteShader(Shader);
		
		return 0;
	}

	return Shader;
}

PyroParticles::CTexture_OGLES2::CTexture_OGLES2(PyroParticles::PyroGraphics::CBitmap *pBitmap, PyroDWord Flags)
{
	glGenTextures(1, (GLuint *) &m_glTexture);
	glBindTexture(GL_TEXTURE_2D, m_glTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
	             pBitmap->GetWidth(), pBitmap->GetHeight(),  0, GL_RGBA,
		         GL_UNSIGNED_BYTE, pBitmap->GetBuffer());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

enum EStdShaderAttribLocations
{
	STD_SHADER_ATTRIB_POSITION = 0,
	STD_SHADER_ATTRIB_DIFFUSE,
	STD_SHADER_ATTRIB_TEXTURE0,
	STD_SHADER_NUM_ATTRIBS
};

PyroResult PyroParticles::CGraphics_OGLES2::RenderQuads2(
	PyroParticles::PyroGraphics::IVertexBuffer2 *_pVertexBuffer,
	PyroDWord nFirst,
	PyroDWord nQuads,
	PyroDWord Flags)
{
	if (nQuads == 0)
		return PyroOK;
		
	PyroDWord nCount = nQuads;

	CVertexBuffer_OGLES2 *pVertexBuffer = (CVertexBuffer_OGLES2 *) _pVertexBuffer;

	CIndexBuffer_OGLES2 *pIndexBuffer = (CIndexBuffer_OGLES2 *) m_pQuadIndexBuffer;

	int glPrimType = GL_TRIANGLES;

	PyroDWord VertexSize = pVertexBuffer->GetVertexSize();

	PyroDWord Format = pVertexBuffer->GetVertexFormat();
	PyroByte *pVertex = (PyroByte *) pVertexBuffer->GetBuffer();

	if (Format & PyroParticles::PyroGraphics::IVertexBuffer::VF_POSITION)
	{
		glEnableVertexAttribArray(STD_SHADER_ATTRIB_POSITION);
		glVertexAttribPointer(STD_SHADER_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, VertexSize, pVertex);
	 	pVertex += sizeof(float) * 3;
	}

	if (Format & PyroParticles::PyroGraphics::IVertexBuffer::VF_RHW)
		pVertex += sizeof(float);

	/* Not supported yet */
	if (Format & PyroParticles::PyroGraphics::IVertexBuffer::VF_NORMAL)
		pVertex += sizeof(float) * 3;

	if (Format & PyroParticles::PyroGraphics::IVertexBuffer::VF_DIFFUSE)
	{
		glEnableVertexAttribArray(STD_SHADER_ATTRIB_DIFFUSE);
		glVertexAttribPointer(STD_SHADER_ATTRIB_DIFFUSE, 4, GL_UNSIGNED_BYTE, GL_TRUE, VertexSize, pVertex);
		pVertex += 4;
	}

	if (Format & PyroParticles::PyroGraphics::IVertexBuffer::VF_TEXTURE0)
	{	
		if (!m_bNullTexture[0])
		{
			glEnableVertexAttribArray(STD_SHADER_ATTRIB_TEXTURE0);
			glVertexAttribPointer(STD_SHADER_ATTRIB_TEXTURE0, 2, GL_FLOAT, GL_FALSE, VertexSize, pVertex);
		}

		pVertex += sizeof(float) * 2;
	}

	glDrawElements(glPrimType, nCount * 6, GL_UNSIGNED_SHORT, ((PyroByte *) pIndexBuffer->GetBuffer()) + 12 * nFirst);

	if (Format & PyroParticles::PyroGraphics::IVertexBuffer::VF_POSITION)
		glDisableVertexAttribArray(STD_SHADER_ATTRIB_POSITION);

	if (Format & PyroParticles::PyroGraphics::IVertexBuffer::VF_DIFFUSE)
		glDisableVertexAttribArray(STD_SHADER_ATTRIB_DIFFUSE);

	if (Format & PyroParticles::PyroGraphics::IVertexBuffer::VF_TEXTURE0)
		glDisableVertexAttribArray(STD_SHADER_ATTRIB_TEXTURE0);

  	return PyroOK; 
}

void PyroParticles::CGraphics_OGLES2::CreateShaders()
{
	const char *vShaderStr =
		"uniform mat4 u_mvpMatrix;\n"
		"attribute vec4 a_position;\n"
		"attribute vec4 a_diffuse;\n"
		"attribute mediump vec2 a_texture0;\n"
		"varying mediump vec2 v_texture0;\n"
		"varying lowp vec4 v_diffuse;\n"
		"void main()\n"
		"{\n"
		" v_diffuse = a_diffuse;\n"
		" gl_Position = u_mvpMatrix * a_position;\n"
		" v_texture0 = a_texture0;\n"
		"}\n";
 
	const char *fShaderStr =
		"uniform sampler2D s_texture0;\n"
		"varying mediump vec2 v_texture0;\n"
		"varying lowp vec4 v_diffuse;\n"
		"void main()\n"
		"{\n"
		" gl_FragColor = texture2D(s_texture0, v_texture0) * v_diffuse;\n"
		"}\n";
 
	GLuint VertexShader = ::LoadShader(vShaderStr, GL_VERTEX_SHADER);
	GLuint FragmentShader = ::LoadShader(fShaderStr, GL_FRAGMENT_SHADER);
	
	assert(VertexShader != 0);
	assert(FragmentShader != 0);
 
	GLuint Program = glCreateProgram();
	
	glAttachShader(Program, VertexShader);
	glAttachShader(Program, FragmentShader);

	glBindAttribLocation(Program, STD_SHADER_ATTRIB_POSITION, "a_position");
	glBindAttribLocation(Program, STD_SHADER_ATTRIB_DIFFUSE, "a_diffuse");
	glBindAttribLocation(Program, STD_SHADER_ATTRIB_TEXTURE0, "a_texture0");
	
	glLinkProgram(Program);
	
	GLint Status;
	glGetProgramiv(Program, GL_LINK_STATUS, &Status);
	
	assert(Status != 0);

	glDetachShader(Program, VertexShader);
	glDetachShader(Program, FragmentShader);
	
	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	GLint LogLength;
	glValidateProgram(Program);
    glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &LogLength);

    if (LogLength > 0)
   	{
        char *pLog = (char *)malloc(LogLength);
        glGetProgramInfoLog(Program, LogLength, &LogLength, pLog);
	//	CLog::GetSingleton()->PrintLn("Program validate log : %s", pLog);
        free(pLog);
    }

	m_Program = Program;
	
	m_MVPMatrixLocation = glGetUniformLocation(Program, "u_mvpMatrix");
}

void PyroParticles::CGraphics_OGLES2::CreateQuadIndexBuffer(int nQuads)
{
	PyroParticles::PyroGraphics::IIndexBuffer *pIndexBuffer = NULL;

	CreateIndexBuffer(&pIndexBuffer, PyroParticles::PyroGraphics::IIndexBuffer::IF_WORD, nQuads * 6, 0);

	pIndexBuffer->Lock();
	
	uint16_t *pDest = (uint16_t *) pIndexBuffer->GetBuffer();
	
	uint16_t nVertex = 0;
	
	for (int i = 0; i < nQuads; i ++)
	{
		pDest[0] = nVertex + 0;
		pDest[1] = nVertex + 2;
		pDest[2] = nVertex + 3;

		pDest[3] = nVertex + 0;
		pDest[4] = nVertex + 1;
		pDest[5] = nVertex + 2;
		
		pDest += 6;
		nVertex += 4;
	}
	
	pIndexBuffer->Unlock();
	
	m_pQuadIndexBuffer = pIndexBuffer;
}

PyroResult PyroParticles::CGraphics_OGLES2::SetBlendFunc(
	PyroParticles::PyroGraphics::IDevice::EBlend SrcBlend,
	PyroParticles::PyroGraphics::IDevice::EBlend DestBlend)
{
	int glSrcBlend;

	switch (SrcBlend)
	{
	case BLEND_ZERO:					glSrcBlend = GL_ZERO;	 				break;
	case BLEND_ONE:						glSrcBlend = GL_ONE; 					break;
	case BLEND_DST_COLOR:				glSrcBlend = GL_DST_COLOR; 				break;
	case BLEND_ONE_MINUS_DST_COLOR:		glSrcBlend = GL_ONE_MINUS_DST_COLOR; 	break;
	case BLEND_SRC_ALPHA:				glSrcBlend = GL_SRC_ALPHA; 				break;
	case BLEND_ONE_MINUS_SRC_ALPHA:		glSrcBlend = GL_ONE_MINUS_SRC_ALPHA; 	break;
	case BLEND_DST_ALPHA:				glSrcBlend = GL_DST_ALPHA; 				break;
	case BLEND_ONE_MINUS_DST_ALPHA:		glSrcBlend = GL_ONE_MINUS_DST_ALPHA; 	break;
	case BLEND_SRC_ALPHA_SATURATE:		glSrcBlend = GL_SRC_ALPHA_SATURATE; 	break;
	default:							return PyroOK;
	}

	int glDestBlend;

	switch (DestBlend)
	{
	case BLEND_ZERO:					glDestBlend = GL_ZERO;	 				break;
	case BLEND_ONE:						glDestBlend = GL_ONE; 					break;
	case BLEND_DST_COLOR:				glDestBlend = GL_DST_COLOR; 			break;
	case BLEND_ONE_MINUS_DST_COLOR:		glDestBlend = GL_ONE_MINUS_DST_COLOR; 	break;
	case BLEND_SRC_ALPHA:				glDestBlend = GL_SRC_ALPHA; 			break;
	case BLEND_ONE_MINUS_SRC_ALPHA:		glDestBlend = GL_ONE_MINUS_SRC_ALPHA; 	break;
	case BLEND_DST_ALPHA:				glDestBlend = GL_DST_ALPHA; 			break;
	case BLEND_ONE_MINUS_DST_ALPHA:		glDestBlend = GL_ONE_MINUS_DST_ALPHA; 	break;
	case BLEND_SRC_ALPHA_SATURATE:		glDestBlend = GL_SRC_ALPHA_SATURATE; 	break;
	default:							return PyroOK;
	}

	glBlendFunc(glSrcBlend, glDestBlend);

	return PyroOK;
}

PyroResult PyroParticles::CGraphics_OGLES2::SetRenderState(PyroDWord State, PyroDWord Value)
{
	int glState;

	switch (State)
	{
	case RS_ALPHABLENDENABLE:		glState = GL_BLEND; break;
	default:						return PyroOK;
	}

	if (Value)
			glEnable(glState);
	else	glDisable(glState);

	return PyroOK;
}

PyroResult PyroParticles::CGraphics_OGLES2::SetWorldMatrix(float WorldMatrix[3][4])
{
	return PyroOK;
}

PyroResult PyroParticles::CGraphics_OGLES2::CreateVertexBuffer2(
	PyroParticles::PyroGraphics::IVertexBuffer2 **ppVertexBuffer,
	PyroDWord Vertices,
	PyroDWord VertexSize,
	PyroDWord VertexFormat,
	PyroDWord Flags)
{
	*ppVertexBuffer = new CVertexBuffer_OGLES2(this, Vertices, VertexSize, VertexFormat, Flags);

	return PyroOK;
}

PyroResult PyroParticles::CGraphics_OGLES2::CreateIndexBuffer(PyroParticles::PyroGraphics::IIndexBuffer **ppIndexBuffer, PyroDWord ItemType, PyroDWord nItems, PyroDWord Flags)
{
	*ppIndexBuffer = new CIndexBuffer_OGLES2(this, ItemType, nItems, Flags);

	return PyroOK;
}

PyroResult PyroParticles::CGraphics_OGLES2::CreateTexture(PyroParticles::PyroGraphics::ITexture **ppTexture, PyroParticles::PyroGraphics::CBitmap *pBitmap, PyroDWord Flags)
{
	*ppTexture = new CTexture_OGLES2(pBitmap, Flags);

	return PyroOK;
}

PyroResult PyroParticles::CGraphics_OGLES2::CreateBitmap(PyroParticles::PyroGraphics::CBitmap **ppBitmap, PyroByte *pBuffer, PyroDWord Width, PyroDWord Height, PyroDWord Flags)
{
	*ppBitmap = new PyroParticles::PyroGraphics::CBitmap(pBuffer, Width, Height, Flags);

	return PyroOK;
}

PyroResult PyroParticles::CGraphics_OGLES2::SetTexture(PyroDWord Stage, PyroParticles::PyroGraphics::ITexture *pTexture)
{
	assert(Stage == 0);

	if (pTexture)
	{
		CTexture_OGLES2 *pTexture_OGLES2 = (CTexture_OGLES2 *) pTexture;

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, pTexture_OGLES2->GetOGLTexture());

		m_bNullTexture[Stage] = false;
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
		m_bNullTexture[Stage] = true;
	}

	return PyroOK;
}
