
#include "Stdafx.h"
#include "ParticlesLK.h"

#define DEF_SCREEN_WIDTH	  1280		// Ä¬ÈÏÆÁ¿í
#define DEF_SCREEN_HEIGHT	  768			// Ä¬ÈÏÆÁ¸ß

ParticlesLK::ParticlesLK(void)
{
	hge_ = hgeCreate(HGE_VERSION);
	g_pParticleLibrary = nullptr;
	g_pPyroFile = nullptr;
	g_pEmitters = nullptr;

	pos_x_ = 640.0f;
	pos_y_ = 384.0f;

	bShow_ = false;
}

ParticlesLK::~ParticlesLK(void)
{
	hge_->Release();
}

bool ParticlesLK::OnFrame(float delta_time)
{
	float screen_width = static_cast<float>(hge_->System_GetState(HGE_SCREENWIDTH));
	float screen_height = static_cast<float>(hge_->System_GetState(HGE_SCREENHEIGHT));

	float hscale = static_cast<float> (screen_width / DEF_SCREEN_WIDTH);  
	float vscale = static_cast<float> (screen_height / DEF_SCREEN_HEIGHT);  

	srand(GetTickCount());

	const PyroParticles::IPyroParticleEmitter *pEmitter = g_pPyroFile->GetFileEmitter(0);
	const char *pEmitterName = pEmitter->GetName();

	
	//Test
	if( hge_->Input_KeyDown(HGEK_LBUTTON))
	{
		hge_->Input_GetMousePos(&pos_x_,&pos_y_);
		bShow_ = true;
	}

	g_pEmitters->RemoveInactive(g_Time, 0.1f);

	if (g_Time - g_Phase > 4.0f)
	{
		//zlogC.Format("zlogC::g_Time1=%f",g_Time);
		//OutputDebugString(zlogC);

		g_Phase = g_Time;

		g_pEmitters->Append(g_pPyroFile, pEmitterName, g_Time, pos_x_, pos_y_);
	}

	float PrevTime = g_Time;
	g_Time = (hge_->Timer_GetTime() - g_StartTime);
	float TimeDelta = g_Time - PrevTime;


	float CurTime = PrevTime;

	while (TimeDelta > 0.0f)
	{
		float SafeTimeDelta = (TimeDelta >= 0.02f) ? 0.02f : TimeDelta;

		g_pEmitters->Move(CurTime, SafeTimeDelta);
		g_pEmitters->Prepare(CurTime, SafeTimeDelta);

		CurTime += SafeTimeDelta;
		TimeDelta -= SafeTimeDelta;
	}

	return false;
	
}

bool ParticlesLK::OnRender(float offset_x, float offset_y, float hscale, float vscale)
{
	if(bShow_)
	{
		g_pEmitters->Render(g_Time);
	}

	return false;
}

void ParticlesLK::Initiate(const char *fileName)
{
	PyroParticles::PyroGraphics::IDevice *pPyroGraphicsDevice = new PyroParticles::CGraphics_HGE(hge_);
	g_pParticleLibrary = CreateParticleLibrary(PYRO_SDK_VERSION);
	g_pParticleLibrary->Init(pPyroGraphicsDevice);

	g_pEmitters = new PyroParticles::CEmitterList();

	g_pPyroFile = g_pParticleLibrary->LoadPyroFile(fileName);
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_bomb.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_bubble.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_Ding.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_doubledragon.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_fish.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_fixbomb.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_JuBuZaDan.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_JuBuZaDan1.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_Knife.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_light.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_Missile.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_Missile1.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_moneybomb.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_salute1.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_smallbomb.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_switch.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\Pak_yinyang.pyro");
// 	g_pPyroFile = g_pParticleLibrary->LoadPyroFile("lkpy\\particle\\particle.pyro");
	g_pPyroFile->CreateTextures();

	g_Time = 0.0f;
	g_Phase = -100.0f;
	g_StartTime = hge_->Timer_GetTime();
}

bool ParticlesLK::getShow()
{
	return bShow_;
}
void ParticlesLK::setShow(bool bShow)
{
	bShow_ = bShow;
}

void ParticlesLK::setPosXY(float x,float y)
{
	pos_x_ = x;
	pos_y_ = y;
}
