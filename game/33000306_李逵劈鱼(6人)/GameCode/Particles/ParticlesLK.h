#pragma once
#include "hge.h"
#include "Pyro.h"
#include "PyroHGE.h"
#include "EmitterList.h"

class ParticlesLK
{
public:
	ParticlesLK(void);
	~ParticlesLK(void);

	void	Initiate(const char *fileName);
	bool	OnFrame(float delta_time);
	bool	OnRender(float offset_x, float offset_y, float hscale, float vscale);
	void	setShow(bool bshow);
	bool	getShow();
	void	setPosXY(float x,float y);

private:
	PyroParticles::IPyroParticleLibrary *g_pParticleLibrary;
	PyroParticles::IPyroFile *g_pPyroFile;
	PyroParticles::CEmitterList *g_pEmitters;

	float g_StartTime;
	float g_Time;
	float g_Phase;

	float pos_x_;
	float pos_y_;


private:

	HGE* hge_;
	bool bShow_;

};