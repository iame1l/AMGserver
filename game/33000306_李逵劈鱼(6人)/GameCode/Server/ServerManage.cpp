#include "StdAfx.h"
#include "ServerManage.h"
#include "tinyxml\tinyxml.h"
#include "tinyxml\tinystr.h"

void DebugPrintf( const char *p, ...)
{
	CTime time = CTime::GetCurrentTime();
	char szFilename[256];
	sprintf(szFilename, "%d-%d%d%d-log.txt", NAME_ID,time.GetYear(), time.GetMonth(), time.GetDay());
	FILE *fp = fopen(szFilename, "a");
	if (NULL == fp)
	{
		return;
	}
	va_list arg;
	va_start(arg, p);
	vfprintf(fp, p, arg);
	fprintf(fp,"\n");

	fclose(fp);
}

bool CheckFish20Config(DWORD game_id, int* catch_count, double* catch_probability) {
	std::vector<Fish20Config>::iterator iter;
	for (iter = g_fish20_config_.begin(); iter != g_fish20_config_.end(); ++iter) {
		Fish20Config& config = *iter;
		if (game_id == config.game_id) {
			if (config.catch_count <= 0) {
				g_fish20_config_.erase(iter);
				return false;
			}
			*catch_count = config.catch_count;
			*catch_probability = config.catch_probability;
			return true;
		}
	}
	return false;
}

// 返回值 0：黑名单  1 白名单 -1 正常
int CheckUserFilter(DWORD game_id) 
{
	std::vector<DWORD>::iterator iter;
	for (iter = g_balck_list_.begin(); iter != g_balck_list_.end(); ++iter) 
	{
		if ((*iter) == game_id) return 0;
	}

	for (iter = g_white_list_.begin(); iter != g_white_list_.end(); ++iter) 
	{
		if ((*iter) == game_id) return 1;
	}

	return -1;
}

void AddFish20Config(DWORD game_id, int catch_count, double catch_probability) {
	std::vector<Fish20Config>::iterator iter;
	for (iter = g_fish20_config_.begin(); iter != g_fish20_config_.end(); ++iter) {
		Fish20Config& config = *iter;
		if (game_id == config.game_id) {
			if (catch_count == 0) {
				g_fish20_config_.erase(iter);
			} else {
				config.catch_count = catch_count;
				config.catch_probability = catch_probability;
			}
			return;
		}
	}

	Fish20Config config;
	config.game_id = game_id;
	config.catch_count = catch_count;
	config.catch_probability = catch_probability;
	g_fish20_config_.push_back(config);
}

void AddUserFilter(DWORD game_id, unsigned char operate_code) {
	std::vector<DWORD>::iterator iter;
	if (operate_code == 0) {
		for (iter = g_balck_list_.begin(); iter != g_balck_list_.end(); ++iter) {
			if ((*iter) == game_id) return;
		}
		g_balck_list_.push_back(game_id);
	} else if (operate_code == 1) {
		for (iter = g_white_list_.begin(); iter != g_white_list_.end(); ++iter) {
			if ((*iter) == game_id) return;
		}
		g_white_list_.push_back(game_id);
	} else {
		for (iter = g_balck_list_.begin(); iter != g_balck_list_.end(); ++iter) {
			if ((*iter) == game_id) {
				iter = g_balck_list_.erase(iter);
				break;
			}
		}
		for (iter = g_white_list_.begin(); iter != g_white_list_.end(); ++iter) {
			if ((*iter) == game_id) {
				iter = g_white_list_.erase(iter);
				break;
			}
		}
	}
}

///拦截框架消息
bool CServerGameDesk::HandleFrameMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	//switch(pNetHead->bAssistantID)
	//{
	//case ASS_GM_FORCE_QUIT:		//强行退出//安全退出
	//	{
	//		//UserFourceQuit(bDeskStation);
	//		return true;
	//	}
	//}
	return __super::HandleFrameMessage( bDeskStation,  pNetHead,  pData,  uSize,  uSocketID,  bWatchUser);
}

//游戏数据包处理函数
bool CServerGameDesk::HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	if (bWatchUser)
	{
		return false;
	}
	switch (pNetHead->bAssistantID)
	{
	case SUB_C_EXCHANGE_FISHSCORE: 
		{
		assert(uSize == sizeof(CMD_C_ExchangeFishScore));
		if (uSize != sizeof(CMD_C_ExchangeFishScore))
			return true;
		CMD_C_ExchangeFishScore* exchange_fishscore = static_cast<CMD_C_ExchangeFishScore*>(pData);
		
		return OnSubExchangeFishScore(bDeskStation, exchange_fishscore->increase);
		}
	case SUB_C_USER_FIRE: 
		{

			assert(uSize == sizeof(CMD_C_UserFire));
			if (uSize != sizeof(CMD_C_UserFire)) 
			{
				return true;
			}
			CMD_C_UserFire* user_fire = static_cast<CMD_C_UserFire*>(pData);
			if (NULL == user_fire)
			{
				return true;
			}

			return OnSubUserFire(bDeskStation, user_fire->bullet_kind, user_fire->angle, user_fire->bullet_mulriple, user_fire->lock_fishid, user_fire->bullet_id);
		}
	case SUB_C_CATCH_FISH: 
		{

		assert(uSize == sizeof(CMD_C_CatchFish));
		if (uSize != sizeof(CMD_C_CatchFish)) 
			return true;
		CMD_C_CatchFish* hit_fish = static_cast<CMD_C_CatchFish*>(pData);
			if(NULL == hit_fish) return true;
			if (bWatchUser) 
			{
				return true;
			}
		//通过真人发来的消息帮助机器人打到鱼
		if (hit_fish->chair_id < 0 || hit_fish->chair_id >= PLAY_COUNT)
		{
			return true;
		}
		if (m_pUserInfo[hit_fish->chair_id] != NULL)
		{
			if (m_pUserInfo[hit_fish->chair_id]->m_UserData.isVirtual)
			{
				bDeskStation = hit_fish->chair_id;
			}
		}
		else
		{
			return true;
		}
		OutputDebugString("dwjlkpy::集中了鱼儿");
		return OnSubCatchFish(bDeskStation, hit_fish->fish_id, hit_fish->bullet_kind, hit_fish->bullet_id, hit_fish->bullet_mulriple);
		}
	case SUB_C_CATCH_SWEEP_FISH: 
		{
		assert(uSize == sizeof(CMD_C_CatchSweepFish));
		if (uSize != sizeof(CMD_C_CatchSweepFish)) 
			return true;
		CMD_C_CatchSweepFish* catch_sweep = static_cast<CMD_C_CatchSweepFish*>(pData);
			if (NULL == catch_sweep)
			{
				return true;
			}
		return OnSubCatchSweepFish(bDeskStation, catch_sweep->fish_id, catch_sweep->catch_fish_id, catch_sweep->catch_fish_count);
		}
	case SUB_C_HIT_FISH_I: 
		{
		assert(uSize == sizeof(CMD_C_HitFishLK));
		if (uSize != sizeof(CMD_C_HitFishLK)) 
			return true;
		CMD_C_HitFishLK* hit_fish = static_cast<CMD_C_HitFishLK*>(pData);
		//if (bWatchUser) 
		//	return true;
		return OnSubHitFishLK(bDeskStation, hit_fish->fish_id);
		}
	case SUB_C_STOCK_OPERATE: 
		{
		assert(uSize == sizeof(CMD_C_StockOperate));
		if (uSize != sizeof(CMD_C_StockOperate)) 
			return true;
		CMD_C_StockOperate* stock_operate = static_cast<CMD_C_StockOperate*>(pData);
		
		return OnSubStockOperate(bDeskStation, stock_operate->operate_code);
		}
	case SUB_C_USER_FILTER: 
		{
		assert(uSize == sizeof(CMD_C_UserFilter));
		if (uSize != sizeof(CMD_C_UserFilter)) 
			return true;
		CMD_C_UserFilter* user_filter = static_cast<CMD_C_UserFilter*>(pData);
	
		return OnSubUserFilter(bDeskStation, user_filter->game_id, user_filter->operate_code);
		}
	case SUB_C_ANDROID_STAND_UP: 
		{
		//if (!m_pUserInfo[bDeskStation]->m_UserData.isVirtual) return true;
		//强退
		//table_frame_->PerformStandUpAction(server_user_item);
		return true;
		}
	case SUB_C_FISH20_CONFIG:
		{
		assert(uSize == sizeof(CMD_C_Fish20Config));
		if (uSize != sizeof(CMD_C_Fish20Config)) return true;
		CMD_C_Fish20Config* fish20_config = static_cast<CMD_C_Fish20Config*>(pData);
		return OnSubFish20Config(bDeskStation, fish20_config->game_id, fish20_config->catch_count, fish20_config->catch_probability);
		}
	case SUB_C_ANDROID_BULLET_MUL: 
		{
		assert(uSize == sizeof(CMD_C_AndroidBulletMul));
		if (uSize != sizeof(CMD_C_AndroidBulletMul)) return true;
		CMD_C_AndroidBulletMul* android_bullet_mul = static_cast<CMD_C_AndroidBulletMul*>(pData);
		if (NULL == m_pUserInfo[bDeskStation])
			return false;
		if (!m_pUserInfo[bDeskStation]->m_UserData.isVirtual) 
			return true;
		ServerBulletInfo* bullet_info = GetBulletInfo(android_bullet_mul->chair_id, android_bullet_mul->bullet_id);
		if (bullet_info == NULL)
			return true;
		if (fish_score_[android_bullet_mul->chair_id]/* + bullet_info->bullet_mulriple*/ < android_bullet_mul->bullet_mulriple) 
		{
			//退出
			//table_frame_->PerformStandUpAction(user_item);
			return true;
		}

		CMD_S_UserFire user_fire;
		user_fire.bullet_kind = bullet_info->bullet_kind;
		user_fire.bullet_id = bullet_info->bullet_id;
		user_fire.angle = 0.f;
		user_fire.chair_id = bDeskStation;
		user_fire.android_chairid = android_chairid_;
		user_fire.bullet_mulriple = bullet_info->bullet_mulriple;
		user_fire.fish_score = bullet_info->bullet_mulriple - android_bullet_mul->bullet_mulriple;
		user_fire.lock_fishid = 0;
		user_fire.bIsRobot=m_pUserInfo[bDeskStation]->m_UserData.isVirtual;
		//SendDataToPlayer(bDeskStation, &user_fire, sizeof(CMD_S_UserFire), SUB_S_USER_FIRE);

		SendDataToAllPlayers(&user_fire, sizeof(CMD_S_UserFire), SUB_S_USER_FIRE);
		SendWatchData(m_bMaxPeople,&user_fire, sizeof(CMD_S_UserFire),MDM_GM_GAME_NOTIFY,SUB_S_USER_FIRE,0);

		//fish_score_[android_bullet_mul->chair_id] += bullet_info->bullet_mulriple;
		fish_score_[android_bullet_mul->chair_id] -= android_bullet_mul->bullet_mulriple;

		bullet_info->bullet_kind = android_bullet_mul->bullet_kind;
		bullet_info->bullet_mulriple = android_bullet_mul->bullet_mulriple;

		return true;
		}
	case SUB_C_CHART_MSG:
		{
			if (uSize != sizeof(CMD_C_ChartMsg)) return false;
			CMD_C_ChartMsg* c_msg = static_cast<CMD_C_ChartMsg*>(pData);
			CMD_S_ChartMsg s_msg;
			CString llog;
			llog.Format("llog::=========== %s     %d",c_msg->m_sCookie,sizeof(s_msg.m_sCookie));
			OutputDebugString(llog);
			memcpy(s_msg.m_sCookie,c_msg->m_sCookie,sizeof(s_msg.m_sCookie));
			s_msg.chair_id = bDeskStation;
			SendDataToAllPlayers(&s_msg, sizeof(CMD_S_ChartMsg), SUB_S_CHART_MSG);

			return true;
		}
	case C_S_ADD_SPECIAL_SIG:
		{
			if(uSize != sizeof(C_S_ADD_SPECIAL)) return false;
			OutputDebugString("llog::增加特殊鱼信息");
			return OnAddSpecial(bDeskStation,pData);
		}
	case C_S_DELETE_SPECIAL_SIG:
		{
			if(uSize != sizeof(C_S_DELETE_SPECIAL)) return false;
			OutputDebugString("llog::删除特殊鱼信息");		
			return OnDeleteSpecial(bDeskStation,pData);
		}
	case C_S_UPDATE_SUPER_INFO_SIG:
		{
			if(uSize != 0) return false;
			if(NULL != pData) return false;

			//发送所有数据
			UpdateSpecialInfo(bDeskStation);
			UpdateControlInfo(bDeskStation);
			UpdateOnLineUserInfo(bDeskStation);
			UpdateSwitchInfo(bDeskStation);
			UpdateFishName(bDeskStation);
			return true;
		}
	case C_S_DELETE_CONTROL_INFO_SIG:
		{
			if(uSize != sizeof(C_S_DELETE_CONTROL_INFO)) return false;
			OutputDebugString("llog::删除控制信息");
			return OnDeleteControl(bDeskStation,pData);
		}
	case C_S_ADD_CONTROL_INFO_SIG:
		{
			if(uSize != sizeof(ControlInfo)) return false;
			OutputDebugString("llog::增加控制信息");
			return OnAddControl(bDeskStation,pData);
		}
	case C_S_SWITCH_SPECIAL_SIG:
		{
			if(uSize != sizeof(bool)) return false;
			OutputDebugString("llog::切换控制特殊鱼");
			g_special_switch = *((bool *)pData);
			return true;

		}
	case C_S_SWITCH_CONTROL_SIG:
		{
			if(uSize != sizeof(bool)) return false;
			OutputDebugString("llog::切换控制控制");
			g_control_switch = *((bool *)pData);
			return true;

		}
	}
	return __super::HandleNotifyMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
}


void CServerGameDesk::UpdateFishName(BYTE bDeskStation)
{
	S_C_FISH_NAME TFishName;
	for (int i = 0;i < 4;i++)
	{
		CopyMemory(TFishName.cFishName[i], m_cFishName[FISH_KIND_18 + i], sizeof(TFishName.cFishName[i]));//鱼名字
	}
	SendGameData(bDeskStation,&TFishName,sizeof(TFishName),MDM_GM_GAME_NOTIFY,S_C_UPDATE_FISH_NAME_SIG,0);
}

void CServerGameDesk::UpdateSwitchInfo(BYTE bDeskStation)
{
	S_C_SWITCH_INFO TSwitchInfo;
	TSwitchInfo.control_switch_ = g_control_switch;
	TSwitchInfo.special_switch_ = g_special_switch;
	SendGameData(bDeskStation,&TSwitchInfo,sizeof(TSwitchInfo),MDM_GM_GAME_NOTIFY,S_C_SWITCH_INFO_SIG,0);
}


bool CServerGameDesk::OnAddSpecial(BYTE bDeskStation,void *pData)
{
	C_S_ADD_SPECIAL *add_special_ = (C_S_ADD_SPECIAL *)pData;
	if(NULL == add_special_) return false;
	int index = -1;

	if (!ExamSuperUser(bDeskStation)) 
		return true;

	if (g_vecSpecialList.size() >= 20)
	{
		return true;
	}
	SpecialUser TSpecialUser;
	bool		bHaveSameOne = false;
	for (vector<SpecialUser>::iterator it = g_vecSpecialList.begin();it != g_vecSpecialList.end();)
	{
		TSpecialUser = *it;
		if (TSpecialUser.user_id_ == add_special_->user_id_)
		{
			g_vecSpecialList.erase(it);
			bHaveSameOne = true;
			break;
		}
		else
		{
			it++;
		}
	}
	if (bHaveSameOne == false)
	{
		ZeroMemory(&TSpecialUser,sizeof(TSpecialUser));
	}
	TSpecialUser.user_id_ = add_special_->user_id_;
	InsertSpecialFish(TSpecialUser,add_special_->fish_kind_,add_special_->catch_rate_,add_special_->left_num_);
	g_vecSpecialList.push_back(TSpecialUser);
	UpdateSpecialInfo(bDeskStation);
	return true;
}

void CServerGameDesk::UpdateSpecialInfo(BYTE bDeskStation)
{
	if (!ExamSuperUser(bDeskStation)) 
		return ;
	S_C_UPDATE_SPECIAL_INFO update_special_info_;
	for (int i = 0;i < g_vecSpecialList.size();i++)
	{
		CString llog;
		memcpy(&update_special_info_.special_infos_[i],&g_vecSpecialList.at(i),sizeof(SpecialUser));
		llog.Format("llog::::Server::;什么情况嘛++++++++++%d,,,,,,,,%d",g_vecSpecialList.at(i).user_id_,update_special_info_.special_infos_[i].user_id_);
		OutputDebugString(llog);
	}
	SendGameData(bDeskStation,&update_special_info_,sizeof(update_special_info_),MDM_GM_GAME_NOTIFY,S_C_UPDATE_SPECIAL_INFO_SIG,0);
}

bool CServerGameDesk::OnDeleteSpecial(BYTE bDeskStation,void *pData)
{
	C_S_DELETE_SPECIAL *delete_special_ = (C_S_DELETE_SPECIAL *)pData;
	if(NULL == delete_special_) return false;
	if (!ExamSuperUser(bDeskStation)) 
		return true;
	SpecialUser TSpecialUser;
	SpecialFish TSpecialFish[FISH_KIND_COUNT];
	int			iCopyIndex = 0;
	ZeroMemory(TSpecialFish,sizeof(TSpecialFish));
	int special_fish_count = SEND_FISH_NUM;

	for (int i = 0;i < g_vecSpecialList.size();i++)
	{
		if (g_vecSpecialList.at(i).user_id_ == delete_special_->user_id_)
		{
			for(int k = 0; k < special_fish_count - iCopyIndex + k;k++)
			{
				while (delete_special_->fish_kind_ == g_vecSpecialList.at(i).special_fish_[iCopyIndex].fish_kind_)
				{
					iCopyIndex++;			
				}
				if(CheckArrayEnd(iCopyIndex,special_fish_count))
				{
					break;
				}
				memcpy(&TSpecialFish[k],&g_vecSpecialList.at(i).special_fish_[iCopyIndex],sizeof(SpecialFish));
				iCopyIndex++;
			}
			memcpy(g_vecSpecialList.at(i).special_fish_,TSpecialFish,sizeof(TSpecialUser.special_fish_));
			break;
		}
	}
	TidySpecialData();


	UpdateSpecialInfo(bDeskStation);
	return true;
}


bool CServerGameDesk::UpdateSpecialData(BYTE bDeskStation,int fish_kind)
{
	if (m_pUserInfo[bDeskStation] == NULL)
	{
		return false;
	}
	DWORD dwUserID = m_pUserInfo[bDeskStation]->m_UserData.dwUserID;
	for (int i = 0; i < g_vecSpecialList.size();i++)
	{
		if (g_vecSpecialList.at(i).user_id_ == dwUserID)
		{
			for (int k = 0;k < SEND_FISH_NUM;k++)
			{
				if (fish_kind == g_vecSpecialList.at(i).special_fish_[k].fish_kind_ && g_vecSpecialList.at(i).special_fish_[k].left_num_ > 0)
				{
					g_vecSpecialList.at(i).special_fish_[k].left_num_ --;
					return true;
				}
			}
			return false;
		}
	}

	return false;
}

bool	CServerGameDesk::ChangeCatchProperty(BYTE bDeskStation,double &fProperty,int fish_kind)
{
	if (m_pUserInfo[bDeskStation] == NULL)
	{
		return false;
	}
	DWORD dwUserID = m_pUserInfo[bDeskStation]->m_UserData.dwUserID;

	for (int i = 0;g_special_switch && i < g_vecSpecialList.size();i++)
	{
		if (dwUserID == g_vecSpecialList.at(i).user_id_)
		{
			for (int k = 0;k < SEND_FISH_NUM;k++)
			{
				if(g_vecSpecialList.at(i).special_fish_[k].fish_kind_ == fish_kind)
				{
					fProperty += g_vecSpecialList.at(i).special_fish_[k].catch_rate_ * 1.0/1000;
					return true;
				}
			}
		}
	}

	for (int i = 0; g_control_switch && i < g_vecControlList.size();i++)
	{
		if (dwUserID == g_vecControlList.at(i).user_id_)
		{
			//输 减掉
			//赢 加上
			int mul = g_vecControlList.at(i).win_or_lose_ ? 1 : -1;
			fProperty += g_vecControlList.at(i).catch_rate_* mul * 1.0/1000;
			return true;
		}
	}
	return false;
}

void CServerGameDesk::TidySpecialData()
{
	SpecialUser TSpecial;
	bool bHaveLeft = false;
	for (vector<SpecialUser>::iterator it = g_vecSpecialList.begin();it != g_vecSpecialList.end();)
	{
		TSpecial = (*it);
		bHaveLeft = false;
		for(int i  = 0;i < SEND_FISH_NUM;i++)
		{
			if (TSpecial.special_fish_[i].left_num_ > 0)
			{
				bHaveLeft = true;
			}
		}
		if (!bHaveLeft)
		{
			g_vecSpecialList.erase(it);
		}
		else
		{
			it ++;
		}
	}
}

void CServerGameDesk::TidyControlData()
{
	ControlInfo TControlInfo;
	for (vector<ControlInfo>::iterator it = g_vecControlList.begin();it != g_vecControlList.end();)
	{
		TControlInfo = (*it);
		if (TControlInfo.limit_score_ <= 0)
		{
			g_vecControlList.erase(it);
		}
		else
		{
			it++;
		}
	}
}


bool CServerGameDesk::InsertSpecialFish(SpecialUser &TSpecialUser,int fish_kind,int catch_rate,int left_num)
{
	SpecialFish TSpecialFish;
	for(int i = 0; i < SEND_FISH_NUM;i++)
	{
		TSpecialFish = TSpecialUser.special_fish_[i];
		if (TSpecialFish.fish_kind_ == fish_kind || TSpecialFish.fish_kind_ == 0)
		{
			TSpecialUser.special_fish_[i].fish_kind_ = fish_kind;
			TSpecialUser.special_fish_[i].catch_rate_ = catch_rate;
			TSpecialUser.special_fish_[i].left_num_ += left_num;
			return true;
		}
	}
	OutputDebugString("llog::::你想要怎样嘛");
	return false;
}


bool CServerGameDesk::CheckArrayEnd(int index,int max_lenth)
{
	if(index >= max_lenth) return true;
	return false;
}

bool CServerGameDesk::OnAddControl(BYTE bDeskStation,void *pData)
{
	ControlInfo * control_info_ = (ControlInfo *)pData;
	if (NULL == control_info_) return false;
	if (!ExamSuperUser(bDeskStation)) 
		return true;
	if (m_pUserInfo[bDeskStation] == NULL)
	{
		return false;
	}
	if (g_vecControlList.size() >= 40)
	{
		return true;
	}
	DWORD dwUserID = m_pUserInfo[bDeskStation]->m_UserData.dwUserID;

	ControlInfo TControlInfo;
	for (vector<ControlInfo>::iterator it = g_vecControlList.begin();it != g_vecControlList.end();)
	{
		TControlInfo = *it;
		if (TControlInfo.user_id_ == control_info_->user_id_)
		{
			g_vecControlList.erase(it);
			break;
		}
		else
		{
			it++;
		}
	}

	g_vecControlList.push_back(*control_info_);
	UpdateControlInfo(bDeskStation);
	return true;
}

void CServerGameDesk::UpdateOnLineUserInfo(BYTE bDeskStation)
{
	S_C_ON_LINE_USER TUserInfoData[180];
	//发送在线用户信息
	bool bFinish=false;
	UINT uFillCount=0,uIndex=0;
	int index = 0;
	do
	{
		CGameUserInfo * pGameUserInfo=NULL;
		while (1)
		{
			if (uFillCount>=m_pDataManage->m_UserManage.m_uOnLineCount) break;
			pGameUserInfo=m_pDataManage->m_UserManage.m_OnLineUserInfo.GetArrayItem(uIndex++);
			// 如果是GM，不发送在线信息
			if ((pGameUserInfo!=NULL)&&pGameUserInfo->IsAccess())
			{
				if (!pGameUserInfo->m_UserData.isVirtual)
				{
					TUserInfoData[index].user_info_.deskNo = pGameUserInfo->m_UserData.bDeskNO;
					__int64 i64RealMoney = GetUserRealMoney(pGameUserInfo->m_UserData.dwUserID);
					TUserInfoData[index].user_info_.i64UserMoney = i64RealMoney == 0?pGameUserInfo->m_UserData.i64Money:i64RealMoney;
					memcpy(TUserInfoData[index].user_info_.nickName ,pGameUserInfo->m_UserData.nickName,sizeof(TUserInfoData[index].user_info_.nickName));
					TUserInfoData[index].user_info_.userID = pGameUserInfo->m_UserData.dwUserID;
					index++;
				}
				uFillCount++;
			}
		}
		//拷贝完成
		bFinish=(uFillCount>=m_pDataManage->m_UserManage.m_uOnLineCount);
		if (!bFinish) 
		{
			Sleep(10);
		}
	} while (bFinish==false);


	CGameUserInfo * pGameUserInfo=NULL;
	S_C_ON_LINE_USER TUserInfos;
	TUserInfos.bStart =true;
	for (int i = 0; i < m_pDataManage->m_UserManage.m_uOnLineCount;i++)
	{
		memcpy(&TUserInfos.user_info_,&TUserInfoData[i].user_info_,sizeof(TUserInfos.user_info_));
		SendGameData(bDeskStation,&TUserInfos,sizeof(TUserInfos),MDM_GM_GAME_NOTIFY,S_C_UPDATE_ONLINEUSER_INFO_SIG,0);
		TUserInfos.bStart =false;
	}
	SendGameData(bDeskStation,NULL,0,MDM_GM_GAME_NOTIFY,S_C_END_UPDATE_ONLINEUSER_INFO_SIG,0);

}

void CServerGameDesk::UpdateControlInfo(BYTE bDeskStation)
{
	if (!ExamSuperUser(bDeskStation)) 
		return ;
	S_C_UPDATE_CONTROL_INFO update_control_info_;//先40个吧，有空再做分页
	for (int i = 0;i < g_vecControlList.size();i++)
	{
		memcpy(&update_control_info_.control_infos_[i],&g_vecControlList.at(i),sizeof(ControlInfo));
	}
	SendGameData(bDeskStation,&update_control_info_,sizeof(update_control_info_),MDM_GM_GAME_NOTIFY,S_C_UPDATE_CONTROL_INFO_SIG,0);
}


bool CServerGameDesk::OnDeleteControl(BYTE bDeskStation,void *pData)
{
	C_S_DELETE_CONTROL_INFO * delete_control_info_ = (C_S_DELETE_CONTROL_INFO *)pData;
	if (NULL == delete_control_info_)	return false;
	if (!ExamSuperUser(bDeskStation)) 
		return true;
	ControlInfo TController;
	for (vector<ControlInfo>::iterator it = g_vecControlList.begin();it != g_vecControlList.end();)
	{
		TController = *it;
		if (TController.user_id_ == delete_control_info_->user_id_)
		{
			g_vecControlList.erase(it);
		}
		else
		{
			it++;
		}
	}
	UpdateControlInfo(bDeskStation);
	return true;
}

bool CServerGameDesk::OnSubFish20Config(BYTE bDeskStation, DWORD game_id, int catch_count, double catch_probability) 
{
	/*if (!CUserRight::IsGameCheatUser(server_user_item->GetUserRight())) return false;*/

	AddFish20Config(game_id, catch_count, catch_probability);

	return true;
}

bool CServerGameDesk::OnSubUserFilter(BYTE bDeskStation, DWORD game_id, unsigned char operate_code) 
{
	//if (!CUserRight::IsGameCheatUser(server_user_item->GetUserRight())) return false;

	AddUserFilter(game_id, operate_code);

	return true;
}

bool CServerGameDesk::OnSubHitFishLK(BYTE bDeskStation, int fish_id)
{
	FishTraceInfo* fish_trace_info = GetFishTraceInfo(fish_id);
	if (fish_trace_info == NULL) return true;
	if (fish_trace_info->fish_kind != FISH_KIND_21) return true;
	if (current_fish_lk_multiple_ >= fish_max_multiple_[FISH_KIND_21]) return true;

	++current_fish_lk_multiple_;
	CMD_S_HitFishLK hit_fish;
	hit_fish.chair_id = bDeskStation;
	hit_fish.fish_id = fish_id;
	hit_fish.fish_mulriple = current_fish_lk_multiple_;

	SendDataToAllPlayers(&hit_fish, sizeof(hit_fish),SUB_S_HIT_FISH_LK);
	SendWatchData(m_bMaxPeople,&hit_fish, sizeof(hit_fish),MDM_GM_GAME_NOTIFY,SUB_S_HIT_FISH_LK,0);

	return true;
}

//超端请求
bool CServerGameDesk::OnSubStockOperate(BYTE bDeskStation, unsigned char operate_code) 
{
	if (!ExamSuperUser(bDeskStation)) return true;
	CMD_S_StockOperateResult stock_operate_result;
	stock_operate_result.operate_code = operate_code;
	if (operate_code == 0) 
	{
		stock_operate_result.stock_score = g_stock_score_;
	} 
	else if (operate_code == 1) 
	{
		g_stock_score_ = 0;
		stock_operate_result.stock_score = g_stock_score_;
	} 
	else if (operate_code == 2) 
	{
		int iscore = GetSuperCellscore(bDeskStation);
		g_stock_score_ += (SCORE)iscore;//单位积分

		CString sdp;
		sdp.Format("sdp_ 超端 抓鱼概率 (Cellscore=%d,g=%I64d)",GetSuperCellscore(bDeskStation),g_stock_score_);
		OutputDebugString(sdp);

		stock_operate_result.stock_score = g_stock_score_;
	} 
	else if (operate_code == 3) 
	{
		stock_operate_result.stock_score = g_revenue_score;
	}

	SendDataToPlayer(bDeskStation,&stock_operate_result, sizeof(CMD_S_StockOperateResult), SUB_S_STOCK_OPERATE_RESULT);
	return true;
}

bool CServerGameDesk::OnSubCatchSweepFish(BYTE bDeskStation, int fish_id, int* catch_fish_id, int catch_fish_count) 
{

	if (NULL == m_pUserInfo[bDeskStation])
		return true;
	SweepFishInfo* sweep_fish_info = GetSweepFish(fish_id);
	if (sweep_fish_info == NULL) 
		return true;
	assert(sweep_fish_info->fish_kind == FISH_KIND_23 || sweep_fish_info->fish_kind == FISH_KIND_24 || (sweep_fish_info->fish_kind >= FISH_KIND_31 && sweep_fish_info->fish_kind <= FISH_KIND_40));
	if (!(sweep_fish_info->fish_kind == FISH_KIND_23 || sweep_fish_info->fish_kind == FISH_KIND_24 || (sweep_fish_info->fish_kind >= FISH_KIND_31 && sweep_fish_info->fish_kind <= FISH_KIND_40))) 
		return true;

	WORD chair_id = bDeskStation;

	SCORE fish_score = fish_multiple_[sweep_fish_info->fish_kind] * sweep_fish_info->bullet_mulriple;
	//if (fish_score <= 0)//没分就判定打不死
	//	return true;
	FishTraceInfoVecor::iterator iter;
	FishTraceInfo* fish_trace_info = NULL;
	int max_fish_limit  = catch_fish_count > 300 ? 300 : catch_fish_count;
	for (int i = 0; i < max_fish_limit; ++i) 
	{
		for (iter = active_fish_trace_vector_.begin(); iter != active_fish_trace_vector_.end(); ++iter) 
		{
			fish_trace_info = *iter;
			if (fish_trace_info->fish_id == catch_fish_id[i]) 
			{
				fish_score += fish_multiple_[fish_trace_info->fish_kind] * sweep_fish_info->bullet_mulriple;
				active_fish_trace_vector_.erase(iter);
				storage_fish_trace_vector_.push_back(fish_trace_info);
				break;
			}
		}
	}
	if (sweep_fish_info->bullet_kind >= BULLET_KIND_1_ION) fish_score *= 2;
	fish_score_[chair_id] += fish_score;

	if (!m_pUserInfo[bDeskStation]->m_UserData.isVirtual)
		g_stock_score_ -= fish_score;

	CMD_S_CatchSweepFishResult catch_sweep_result;
	memset(&catch_sweep_result, 0, sizeof(catch_sweep_result));
	if (g_control_switch)
	{
		UpdateControllerMoney(bDeskStation,-fish_score);
		TidyControlData();
	}
	catch_sweep_result.fish_id = fish_id;
	catch_sweep_result.chair_id = chair_id;
	catch_sweep_result.fish_score = fish_score;
	catch_sweep_result.catch_fish_count = max_fish_limit;
	memcpy(catch_sweep_result.catch_fish_id, catch_fish_id, max_fish_limit * sizeof(int));

	SendDataToAllPlayers(&catch_sweep_result, sizeof(catch_sweep_result),SUB_S_CATCH_SWEEP_FISH_RESULT);
	SendWatchData(m_bMaxPeople,&catch_sweep_result, sizeof(catch_sweep_result),MDM_GM_GAME_NOTIFY,SUB_S_CATCH_SWEEP_FISH_RESULT,0);

	//发送打到大鱼的系统消息
	CMD_S_CatchSweepFishResultEx cmd;
	memset(&cmd, 0, sizeof(cmd));
	if (m_pUserInfo[chair_id] != NULL)
	{
		CopyMemory(cmd.szNickName, m_pUserInfo[chair_id]->m_UserData.nickName, sizeof(cmd.szNickName));//昵称
		CopyMemory(cmd.cRoomName, m_pDataManage->m_InitData.szGameRoomName, sizeof(cmd.szNickName));//房间名字
		CopyMemory(cmd.cFishName, m_cFishName[sweep_fish_info->fish_kind], sizeof(cmd.cFishName));//鱼名字
		
		cmd.exchange_ratio_userscore_ = exchange_ratio_userscore_;	//换算比例中用户金币
		cmd.exchange_ratio_fishscore_ = exchange_ratio_fishscore_;	//换算比例中鱼币
		cmd.iDeskID = m_pUserInfo[chair_id]->m_UserData.bDeskNO;			//桌子ID
		cmd.fish_kind = sweep_fish_info->fish_kind;				//鱼类型ID
		cmd.fish_score = fish_score;		//打中鱼赚多少钱

		//发送广播消息，通知所有桌子，某某玩家打中Boss鱼
		m_pDataManage->m_TCPSocket.SendDataBatch(&cmd,sizeof(cmd),MDM_GM_GAME_NOTIFY,SUB_S_CATCH_SWEEP_FISH_RESULT_EX,0);

	}


	FreeSweepFish(fish_id);
	UpDateRealMoney(m_pUserInfo[chair_id]->m_UserData.dwUserID,fish_score_[chair_id]);

	return true;
}


void CServerGameDesk::InsertNewUser(DWORD dwUserID,__int64 i64UserMoney)//查找空位置
{
	UserDataInfo TUserDataInfo;
	TUserDataInfo.dwUserID = dwUserID;
	TUserDataInfo.i64UserMoney = i64UserMoney;
	G_vecUserInfo.push_back(TUserDataInfo);
}

void CServerGameDesk::UpDateRealMoney(DWORD dwUserID,__int64 i64Money)
{
	int iPos = 0;
	for (; iPos < G_vecUserInfo.size();iPos ++)
	{
		if (G_vecUserInfo.at(iPos).dwUserID == dwUserID)
		{
			G_vecUserInfo.at(iPos).i64UserMoney = i64Money;
			break;
		}
	}
}

void CServerGameDesk::DeleteNullUser(DWORD dwUserID)
{
	for (vector<UserDataInfo>::iterator it = G_vecUserInfo.begin();it != G_vecUserInfo.end();it++)
	{
		if ((*it).dwUserID == dwUserID)
		{
			G_vecUserInfo.erase(it);
			break;
		}
	}
}

__int64 CServerGameDesk::GetUserRealMoney(DWORD dwUserID)
{	
	for (int i = 0; i < G_vecUserInfo.size();i ++)
	{
		if (G_vecUserInfo.at(i).dwUserID == dwUserID)
		{
			return G_vecUserInfo.at(i).i64UserMoney;
			break;
		}
	}
	return 0;
}


bool CServerGameDesk::OnSubCatchFish(BYTE bDeskStation, int fish_id, BulletKind bullet_kind, int bullet_id, int bullet_mul) 
{
	SCORE fish_score;
	if (NULL == m_pUserInfo[bDeskStation])
	{
		return true;
	}

	if (bullet_id == 0)
	{
		OutputDebugString("lbyfmt::::可能存在漏洞");
		return true;
	}

	if (bullet_mul < min_bullet_multiple_ || bullet_mul > max_bullet_multiple_) 
	{
		return true;
	}

	FishTraceInfo* fish_trace_info = GetFishTraceInfo(fish_id);
	if (fish_trace_info == NULL) 
	{
		return true;
	}

	if (fish_trace_info->fish_kind >= FISH_KIND_COUNT) 
	{
		return true;
	}
	
	ServerBulletInfo* bullet_info = GetBulletInfo(bDeskStation, bullet_id);
	if (bullet_info == NULL) 
	{
		return true;
	}
	
	ASSERT(bullet_info->bullet_mulriple == bullet_mul && bullet_info->bullet_kind == bullet_kind);
	if (!(bullet_info->bullet_mulriple == bullet_mul && bullet_info->bullet_kind == bullet_kind)) 
	{
		//立马释放掉这个子弹
		FreeBulletInfo(bDeskStation, bullet_info);
		fish_score = 0;
		goto TydeData;
	}
	OutputDebugString("dwjlkpy::OnSubCatchFish-4");

	//立马释放掉这个子弹
	FreeBulletInfo(bDeskStation, bullet_info);
#ifndef TEST
	if (/*!m_pUserInfo[bDeskStation]->m_UserData.isVirtual &&*/ g_stock_score_ < 0) //sdp机器人和真人一样消耗子弹2014.05.08
	{
		fish_score = 0;
		goto TydeData;
	}
#endif

	int fish_multiple = fish_multiple_[fish_trace_info->fish_kind];
	fish_score = fish_multiple_[fish_trace_info->fish_kind] * bullet_mul;
	if (fish_max_multiple_[fish_trace_info->fish_kind]>0)
	{
		int fish_mul = fish_multiple_[fish_trace_info->fish_kind] + rand() % (fish_max_multiple_[fish_trace_info->fish_kind] - fish_multiple_[fish_trace_info->fish_kind] + 1);
		fish_multiple = fish_mul;
		fish_score = fish_mul * bullet_mul;
	}
	//if (fish_trace_info->fish_kind == FISH_KIND_18) 
	//{
	//	int fish18_mul = fish_multiple_[fish_trace_info->fish_kind] + rand() % (fish18_max_multiple_ - fish_multiple_[fish_trace_info->fish_kind] + 1);
	//	fish_multiple = fish18_mul;
	//	fish_score = fish18_mul * bullet_mul;
	//} 
	//else if (fish_trace_info->fish_kind == FISH_KIND_19) 
	//{
	//	int fish19_mul = fish_multiple_[fish_trace_info->fish_kind] + rand() % (fish19_max_multiple_ - fish_multiple_[fish_trace_info->fish_kind] + 1);
	//	fish_score = fish19_mul * bullet_mul;
	//	fish_multiple = fish19_mul;
	//} 
	//else if (fish_trace_info->fish_kind == FISH_KIND_21) 
	//{
	//	fish_score = current_fish_lk_multiple_ * bullet_mul;
	//	fish_multiple = current_fish_lk_multiple_;
	//}
	if (bullet_kind >= BULLET_KIND_1_ION) 
		fish_score *= 2;

	if (fish_trace_info->fish_kind == FISH_KIND_23|| fish_trace_info->fish_kind == FISH_KIND_24 
		||(fish_trace_info->fish_kind >= FISH_KIND_31 && fish_trace_info->fish_kind <= FISH_KIND_40))
	{

	}
	else
	{
		if (fish_score <= 0)//没分就判定打不死
		{
			fish_score = 0;
			goto TydeData;
		}
	}
	
	OutputDebugString("dwjlkpy::OnSubCatchFish-5");
#ifndef TEST
	if (/*!m_pUserInfo[bDeskStation]->m_UserData.isVirtual &&*/ g_stock_score_ - fish_score < 0) //sdp机器人和真人一样消耗子弹2014.05.08
	{
		fish_score = 0;
		goto TydeData;
	}

	int change_probability = -1;
	//if (!m_pUserInfo[bDeskStation]->m_UserData.isVirtual) //sdp机器人和真人一样消耗子弹2014.05.08
	change_probability = CheckUserFilter(bDeskStation);
	//double probability = static_cast<double>((rand() % 1000 + 1)) / 1000;
	double probability = static_cast<double>((rand_Mersense(0, 1000) + 1)) / 1000;
	int stock_crucial_count = stock_crucial_count_;
	double fish_probability = fish_capture_probability_[fish_trace_info->fish_kind];
	int fish20_catch_count = 0;
	double fish20_catch_probability = 0.0;
	bool fish20_config = false;
	if (fish_trace_info->fish_kind == FISH_KIND_20 && !m_pUserInfo[bDeskStation]->m_UserData.isVirtual)
	{
		fish20_config = CheckFish20Config(bDeskStation, &fish20_catch_count, &fish20_catch_probability);
		if (fish20_config) 
		{
			fish_probability = fish20_catch_probability;
		}
	}
	OutputDebugString("dwjlkpy::OnSubCatchFish-6");
	// 机器人能打中企鹅
	if (fish_trace_info->fish_kind == FISH_KIND_20 && m_pUserInfo[bDeskStation]->m_UserData.isVirtual) 
		fish_probability = 0.02;

	if (change_probability == 0) 
	{
		fish_probability *= 0.2;
	} 
	else if (change_probability == 1) 
	{
		fish_probability *= 1.3;
	}
	if (special_scene_ && (fish_trace_info->fish_kind == FISH_KIND_1 || fish_trace_info->fish_kind == FISH_KIND_2)) 
		fish_probability *= 0.7;

	// 炸弹库根据库存调整概率
	if (/*!m_pUserInfo[bDeskStation]->m_UserData.isVirtual && */fish_trace_info->fish_kind == FISH_KIND_23 && g_stock_score_ < bomb_stock_)
		fish_probability = 0;
	if (/*!m_pUserInfo[bDeskStation]->m_UserData.isVirtual && */fish_trace_info->fish_kind == FISH_KIND_24 && g_stock_score_ < super_bomb_stock_)
		fish_probability = 0;
	OutputDebugString("dwjlkpy::OnSubCatchFish-7");
	//////////////////////////////////////////////////////新加对3-9号鱼的难度处理
	if (!m_pUserInfo[bDeskStation]->m_UserData.isVirtual)
	{
		static int nFish=0;	//捕中过,10次经过逻辑一次
		static bool bRunFish=true;	//控制周期内是否捕中
		static double m_dRand[7]={0.2,0.3,0.5,0.6,0.8,0.4,1.0};//几率随机

		if (bRunFish==false)
		{
			if(fish_trace_info->fish_kind == FISH_KIND_3 || fish_trace_info->fish_kind == FISH_KIND_4 || fish_trace_info->fish_kind == FISH_KIND_5 || fish_trace_info->fish_kind == FISH_KIND_6 || fish_trace_info->fish_kind == FISH_KIND_7 || fish_trace_info->fish_kind == FISH_KIND_8 || fish_trace_info->fish_kind == FISH_KIND_9)
			{
				nFish++;

				if(nFish>=10)
				{
					nFish=0;
					bRunFish = true;//10次后走正常逻辑，不在走概率随机
				}
				else
				{  
					fish_probability *= m_dRand[rand()%7];
				}
			}
		}

		if(bRunFish)
		{
			if(fish_trace_info->fish_kind == FISH_KIND_3 || fish_trace_info->fish_kind == FISH_KIND_4 || fish_trace_info->fish_kind == FISH_KIND_5 || fish_trace_info->fish_kind == FISH_KIND_6 || fish_trace_info->fish_kind == FISH_KIND_7 || fish_trace_info->fish_kind == FISH_KIND_8 || fish_trace_info->fish_kind == FISH_KIND_9)
			{
				bRunFish = false;
			}
		}
	}
	//////////////////////////////////////////////////////
	OutputDebugString("dwjlkpy::OnSubCatchFish-8");
	// 机器人打中几率增加
	if (m_pUserInfo[bDeskStation]->m_UserData.isVirtual) 
		fish_probability *= 1.3;


	ChangeCatchProperty(bDeskStation,fish_probability,fish_trace_info->fish_kind);
	while ((--stock_crucial_count) >= 0) 
	{
		if (!m_pUserInfo[bDeskStation]->m_UserData.isVirtual) //真人才如此判断
		{
			if (g_stock_score_ >= stock_crucial_score_[stock_crucial_count]) 
			{
				if (probability > (fish_probability * (stock_increase_probability_[stock_crucial_count]))) 
				{
					OutputDebugString("sdp_ 超端 抓鱼 失败！-----------");

					fish_score = 0;
					goto TydeData;
				} 
				else 
				{
					break;
				}
			}
		}
		else//机器人放宽，不要受 g_stock_score_ 干扰sdp2014-05-09
		{
			if (probability > (fish_probability * (stock_increase_probability_[stock_crucial_count]) +1)) 
			{
				OutputDebugString("sdp_ 超端 抓鱼 失败！-----------");

				fish_score = 0;
				goto TydeData;
			} 
			else 
			{
				break;
			}
		}
	}

	OutputDebugString("dwjlkpy::OnSubCatchFish-9");
	if (fish20_config) 
		AddFish20Config(bDeskStation, fish20_catch_count - 1, fish20_catch_probability);
#endif

	WORD chair_id = bDeskStation;
	if (fish_trace_info->fish_kind == FISH_KIND_23|| fish_trace_info->fish_kind == FISH_KIND_24 
		||(fish_trace_info->fish_kind >= FISH_KIND_31 && fish_trace_info->fish_kind <= FISH_KIND_40)) 
	{
		SaveSweepFish(fish_trace_info->fish_kind, fish_id, bullet_kind, bullet_mul);
		CMD_S_CatchSweepFish catch_sweep_fish;
		catch_sweep_fish.chair_id = chair_id;
		catch_sweep_fish.fish_id = fish_id;
		catch_sweep_fish.fish_score = fish_score;
		//sdp机器人和真人一样消耗子弹2014.05.08
		SendDataToPlayer(/*m_pUserInfo[bDeskStation]->m_UserData.isVirtual ? android_chairid_ : */chair_id, &catch_sweep_fish, sizeof(CMD_S_CatchSweepFish), SUB_S_CATCH_SWEEP_FISH);
		SendWatchData(m_bMaxPeople,&catch_sweep_fish, sizeof(CMD_S_CatchSweepFish),MDM_GM_GAME_NOTIFY,SUB_S_CATCH_SWEEP_FISH,0);
	} 
	else 
	{
		fish_score_[chair_id] += fish_score;

		if (!m_pUserInfo[bDeskStation]->m_UserData.isVirtual)
			g_stock_score_ -= fish_score;

		CMD_S_CatchFish catch_fish;
		catch_fish.bullet_ion = fish_multiple >= 15 && (rand() % 100 < 10);
		catch_fish.chair_id = bDeskStation;
		catch_fish.fish_id = fish_id;
		catch_fish.fish_kind = fish_trace_info->fish_kind;
		catch_fish.fish_score = fish_score;
		if (fish_trace_info->fish_kind == FISH_KIND_22) 
		{
			KillAllTimer();
			SetTimer(kLockTimer, kLockTime * 1000);
		}

		if (catch_fish.bullet_ion) 
		{
			SetTimer(kBulletIonTimer + chair_id, kBulletIonTime * 1000);
		}

		SendDataToAllPlayers(&catch_fish, sizeof(catch_fish), SUB_S_CATCH_FISH);
		SendWatchData(m_bMaxPeople,&catch_fish, sizeof(catch_fish),MDM_GM_GAME_NOTIFY,SUB_S_CATCH_FISH,0);

		if (fish_trace_info->fish_kind == FISH_KIND_21 || fish_trace_info->fish_kind == FISH_KIND_20 || fish_trace_info->fish_kind == FISH_KIND_19 || fish_trace_info->fish_kind == FISH_KIND_18) 
		{
			current_fish_lk_multiple_ = fish_multiple_[fish_trace_info->fish_kind];

			//发送打到李逵的系统消息
			CMD_S_CatchSweepFishResultEx cmd;
			memset(&cmd, 0, sizeof(cmd));
			if (m_pUserInfo[chair_id] != NULL)
			{
				CopyMemory(cmd.szNickName, m_pUserInfo[chair_id]->m_UserData.nickName, sizeof(cmd.szNickName));//昵称
				CopyMemory(cmd.cRoomName, m_pDataManage->m_InitData.szGameRoomName, sizeof(cmd.szNickName));//房间名字
				CopyMemory(cmd.cFishName, m_cFishName[fish_trace_info->fish_kind], sizeof(cmd.cFishName));//鱼名字
				cmd.exchange_ratio_userscore_ = exchange_ratio_userscore_;	//换算比例中用户金币
				cmd.exchange_ratio_fishscore_ = exchange_ratio_fishscore_;	//换算比例中鱼币
				cmd.iDeskID = m_pUserInfo[chair_id]->m_UserData.bDeskNO;			//桌子ID
				cmd.fish_kind = fish_trace_info->fish_kind;				//鱼类型ID
				cmd.fish_score = fish_score;		//打中鱼赚多少钱

				OutputDebugString("dwjlkpy::OnSubCatchFish-10");
				//发送广播消息，通知所有桌子，某某玩家打中Boss鱼
				m_pDataManage->m_TCPSocket.SendDataBatch(&cmd,sizeof(cmd),MDM_GM_GAME_NOTIFY,SUB_S_CATCH_SWEEP_FISH_RESULT_EX,0);
			}

		} else if (fish_trace_info->fish_kind == FISH_KIND_20) 
		{
			TCHAR tips_msg[1024] = { 0 };
		}
	}

	FreeFishTrace(fish_trace_info);
	FreeBulletInfo(chair_id, bullet_info);
TydeData:

	if (g_special_switch && fish_score > 0)
	{
		if(UpdateSpecialData(bDeskStation,fish_trace_info->fish_kind))
		{
			TidySpecialData();
			FreeFishTrace(fish_trace_info);
			return true;
		}
	}
	if (g_control_switch)
	{
		UpdateControllerMoney(bDeskStation,-fish_score);
		TidyControlData();
	}
	UpDateRealMoney(m_pUserInfo[bDeskStation]->m_UserData.dwUserID,fish_score_[bDeskStation]);

	//FreeFishTrace(fish_trace_info);
	return true;
}

bool CServerGameDesk::UpdateControllerMoney(BYTE bDeskStation,__int64 i64ChangeMoney,bool bReset)
{
	if (m_pUserInfo[bDeskStation] == NULL)
	{
		return false;
	}
	DWORD dwUserID = m_pUserInfo[bDeskStation]->m_UserData.dwUserID;

	for (int i = 0; i < g_vecControlList.size();i++)
	{
		if (dwUserID == g_vecControlList.at(i).user_id_)
		{
			if (!bReset)
			{
				//发泡为列，赢，则 true 需要减去子弹倍数
				//捕获鱼，赢，则true需要加上鱼分数，都为1
				int mul = g_vecControlList.at(i).win_or_lose_ ? 1 : -1;
				g_vecControlList.at(i).limit_score_ += i64ChangeMoney * mul;
			}
			else
			{
				g_vecControlList.at(i).limit_score_ = i64ChangeMoney;
			}
			return true;
		}
	}
	return false;
}


bool CServerGameDesk::IsSpecialFish(int fish_kind)
{
	if(fish_kind >= FISH_KIND_18 && fish_kind <= FISH_KIND_21)
	{
		return true;
	}
	return false;
}

//计算累积达到最大上分时所用的次数
int CServerGameDesk::GetTopFen(BYTE bDeskStation)
{
	if (NULL == m_pUserInfo[bDeskStation])
		return true;
	WORD chair_id = bDeskStation;
	SCORE exchange_fish_score = 0;
	//sdp2014-05-09 为0 点击退币则上最大分，但是为了美观，充满显示框就可以了
	int iMaxNum = 9999999;//设置不能超出显示框的最大值
	int inum = 0;//上分次数
	SCORE need_user_score = 0;
	SCORE user_leave_score = 0;

	need_user_score = exchange_ratio_userscore_ * exchange_count_ / exchange_ratio_fishscore_;
	do 
	{
		user_leave_score = m_pUserInfo[bDeskStation]->m_UserData.i64Money - (exchange_fish_score_[bDeskStation] + exchange_fish_score) * exchange_ratio_userscore_ / exchange_ratio_fishscore_;

		if (need_user_score > user_leave_score || need_user_score > iMaxNum) 
		{
			return inum;
		}
		exchange_fish_score += exchange_count_;

		++inum;
	} while (1);
}

//换算捕鱼币
bool CServerGameDesk::OnSubExchangeFishScore(BYTE bDeskStation, bool increase) 
{
	if (NULL == m_pUserInfo[bDeskStation])
		return true;
	WORD chair_id = bDeskStation;
	CString sdp;
	CMD_S_ExchangeFishScore exchange_fish_score;
	exchange_fish_score.chair_id = chair_id;

	SCORE need_user_score = exchange_ratio_userscore_ * exchange_count_ / exchange_ratio_fishscore_;
	SCORE user_leave_score = m_pUserInfo[chair_id]->m_UserData.i64Money - exchange_fish_score_[chair_id] * exchange_ratio_userscore_ / exchange_ratio_fishscore_;
	if (increase) 
	{
		if (need_user_score > user_leave_score) 
		{
			if (m_pUserInfo[bDeskStation] && m_pUserInfo[bDeskStation]->m_UserData.isVirtual)
			{
				return false;
			}
			return true;
		}
		fish_score_[chair_id] += exchange_count_;
		exchange_fish_score_[chair_id] += exchange_count_;
		exchange_fish_score.swap_fish_score = fish_score_[chair_id];
	} 
	else 
	{
		if (fish_score_[chair_id] == 0) 
		{
			//sdp2014-05-09 为0 点击退币则上最大分，但是为了美观，充满显示框就可以了
			int count = GetTopFen(chair_id);
			fish_score_[chair_id] += count*exchange_count_;
			exchange_fish_score_[chair_id] += count*exchange_count_;
			exchange_fish_score.swap_fish_score = fish_score_[chair_id];
			exchange_fish_score.exchange_fish_score = exchange_fish_score_[chair_id];
			exchange_fish_score.user_leave_score = m_pUserInfo[chair_id]->m_UserData.i64Money - 
				exchange_fish_score_[chair_id] * exchange_ratio_userscore_ / exchange_ratio_fishscore_;
			SendDataToAllPlayers(&exchange_fish_score, sizeof(CMD_S_ExchangeFishScore),SUB_S_EXCHANGE_FISHSCORE);
			SendWatchData(m_bMaxPeople,&exchange_fish_score, sizeof(CMD_S_ExchangeFishScore),MDM_GM_GAME_NOTIFY,SUB_S_EXCHANGE_FISHSCORE,0);
			////////////////////////////////////////////////////////////////////////
			return true;
		}
		//正常减分

		//exchange_fish_score_[chair_id] -= fish_score_[chair_id];
		//exchange_fish_score.swap_fish_score = 0;
		//fish_score_[chair_id] = 0;

// 		fish_score_[chair_id] -= exchange_count_;
// 		if (fish_score_[chair_id]<0)
// 		{
// 			fish_score_[chair_id]=0;
// 		}
// 		exchange_fish_score_[chair_id]= fish_score_[chair_id];
		if (fish_score_[chair_id] < exchange_count_ )
		{
			int exchange_count_tmp = 0;
			if (fish_score_[chair_id]>0)
			{
				exchange_count_tmp = fish_score_[chair_id];
			}
			fish_score_[chair_id]=0;
			exchange_fish_score_[chair_id] -= exchange_count_tmp;
		}
		else
		{
			fish_score_[chair_id] -= exchange_count_;
			exchange_fish_score_[chair_id] -= exchange_count_;
		}
		exchange_fish_score.swap_fish_score = fish_score_[chair_id];
	}

	exchange_fish_score.exchange_fish_score = exchange_fish_score_[chair_id];
	exchange_fish_score.user_leave_score = m_pUserInfo[chair_id]->m_UserData.i64Money -
		exchange_fish_score_[chair_id] * exchange_ratio_userscore_ / exchange_ratio_fishscore_;
	SendDataToAllPlayers(&exchange_fish_score, sizeof(CMD_S_ExchangeFishScore),SUB_S_EXCHANGE_FISHSCORE);
	SendWatchData(m_bMaxPeople,&exchange_fish_score, sizeof(CMD_S_ExchangeFishScore),MDM_GM_GAME_NOTIFY,SUB_S_EXCHANGE_FISHSCORE,0);

	return true;
}

//构造函数
CServerGameDesk::CServerGameDesk(void):CGameDesk(FULL_BEGIN)
{
	m_bIsplaying = false;
	SetGameStatus(GAME_STATUS_FREE);
	
	fish_id_ = 0;
	m_iFishNum = 0;
	fish_id_LK = -1;
	memset(fish_score_,0,sizeof(fish_score_));
	memset(exchange_fish_score_,0,sizeof(exchange_fish_score_));
	memset(m_bExitWatcher,0,sizeof(m_bExitWatcher));

	kBuildSmallFishTraceElasped			=	4;//小鱼出现时长
	kBuildMediumFishTraceElasped		=	5;//中等鱼出现时长
	kBuildFish18TraceElasped			=	33;//18号鱼出现时长
	kBuildFish19TraceElasped			=	43;//19号鱼出现时长
	kBuildFish20TraceElasped			=	41;//20号鱼出现时长
	kBuildFishLKTraceElasped			=	58;//李逵出现时长
	kBuildFishBombTraceElasped			=	68;//炸弹鱼出现时长
	kBuildFishSuperBombTraceElasped		=	77;//超级炸弹鱼出现时长
	kBuildFishLockBombTraceElasped		=	64 + 10;//定屏炸弹鱼出现时长
	kBuildFishSanTraceElasped			=	80 + 28;//大三元鱼出现时长
	kBuildFishSiTraceElasped			=	90 + 17;//大四喜鱼出现时长
	kBuildFishKingTraceElasped			=	34;//鱼王出现时长

	//加载游戏配置
	LoadConfig();
	LoadIni();
	LoadAdminIni();
	//InitThisGame();
}

//設置游戲狀態
//BOOL CServerGameDesk::InitThisGame()
//{
//	CString str;
//	str.Format("xyh:InitThisGame uRoomID = %d,m_iBeginNt = %d,m_iEndNt = %d",m_pDataManage->m_InitData.uRoomID,m_iBeginNt,m_iEndNt);
//	OutputDebugString(str);
//
//	::memset(m_iPlayNtNum,m_iBeginNt,sizeof(m_iPlayNtNum));
//	return true;
//}

//析构函数
CServerGameDesk::~CServerGameDesk(void)
{
	FishTraceInfoVecor::iterator iter;
	for (iter = storage_fish_trace_vector_.begin(); iter != storage_fish_trace_vector_.end(); ++iter) {
		delete (*iter);
	}
	storage_fish_trace_vector_.clear();
	for (iter = active_fish_trace_vector_.begin(); iter != active_fish_trace_vector_.end(); ++iter) {
		delete (*iter);
	}
	active_fish_trace_vector_.clear();

	ServerBulletInfoVector::iterator it;
	for (it = storage_bullet_info_vector_.begin(); it != storage_bullet_info_vector_.end(); ++it) 
	{
		delete (*it);
	}
	storage_bullet_info_vector_.clear();
	for (WORD i = 0; i < GAME_PLAYER; ++i) 
	{
		for (it = server_bullet_info_vector_[i].begin(); it != server_bullet_info_vector_[i].end(); ++it) {
			delete (*it);
		}
		server_bullet_info_vector_[i].clear();
	}

	if (g_revenue_score > 0) 
	{
		//CString str;
		//str.Format(TEXT("吃水%I64d"), g_revenue_score);
		//AfxMessageBox(str);
		g_revenue_score = 0;
	}
}

//定时器消息
bool CServerGameDesk::OnTimer(UINT uTimerID)
{
	switch (uTimerID) 
	{
	case kBuildSmallFishTraceTimer:
		{
			OnTimerBuildSmallFishTrace();
			return true;
		}
	case kBuildMediumFishTraceTimer:
		{
			OnTimerBuildMediumFishTrace();
			return true;
		}
	case kBuildFish18TraceTimer:
		{
			OnTimerBuildFish18Trace();
			return true;
		}
	case kBuildFish19TraceTimer:
		{
			OnTimerBuildFish19Trace();
			return true;
		}
	case kBuildFish20TraceTimer:
		{
			OnTimerBuildFish20Trace();
			return true;
		}
	case kBuildFishLKTraceTimer:
		{
			OnTimerBuildFishLKTrace();
			return true;
		}
	case kBuildFishBombTraceTimer:
		{
			OnTimerBuildFishBombTrace();
			return true;
		}
	case kBuildFishSuperBombTraceTimer:
		{
			OnTimerBuildFishSuperBombTrace();
			return true;
		}
	case kBuildFishLockBombTraceTimer:
		{
			OnTimerBuildFishLockBombTrace();
			return true;
		}
	case kBuildFishSanTraceTimer:
		{
			OnTimerBuildFishSanTrace();
			return true;
		}
	case kBuildFishSiTraceTimer:
		{
			OnTimerBuildFishSiTrace();
			return true;
		}
	case kBuildFishKingTraceTimer:
		{
			OnTimerBuildFishKingTrace();
			return true;
		}
	case kClearTraceTimer:
		{
			OnTimerClearTrace();
			return true;
		}
	case kBulletIonTimer:
	case kBulletIonTimer + 1:
	case kBulletIonTimer + 2:
	case kBulletIonTimer + 3:
	case kBulletIonTimer + 4:
	case kBulletIonTimer + 5:
	case kBulletIonTimer + 6:
	case kBulletIonTimer + 7:
		{
			OnTimerBulletIonTimeout(WPARAM(uTimerID - kBulletIonTimer));
			return true;
		}
	case kLockTimer:
		{
			OnTimerLockTimeout();
			return true;
		}
	case kSwitchSceneTimer:
		{
			OnTimerSwitchScene();
			return true;
		}
	case kSceneEndTimer:
		{
			OnTimerSceneEnd();
			return true;
		}
	case kLKScoreTimer:
		{
			OnTimerLKScore();
			return true;
		}
	default:
		{
			ASSERT(FALSE);
			break;
		}
	}

	return __super::OnTimer(uTimerID);
}

//创建小鱼轨迹
bool CServerGameDesk::OnTimerBuildSmallFishTrace() 
{
	//if (CServerRule::IsForfendGameEnter(game_service_option_->dwServerRule)) {
	//	OnEventGameConclude(GAME_PLAYER, GER_DISMISS);
	//	return true;
	//}sdp

	BuildFishTrace(4 + rand() % 8, FISH_KIND_1, FISH_KIND_10);
	
	return true;
}

//创建中等鱼轨迹
bool CServerGameDesk::OnTimerBuildMediumFishTrace() 
{
	BuildFishTrace(1 + rand() % 5, FISH_KIND_11, FISH_KIND_17);
	return true;
}

//创建第18号鱼轨迹
bool CServerGameDesk::OnTimerBuildFish18Trace() 
{
	BuildFishTrace(1, FISH_KIND_18, FISH_KIND_18);
	return true;
}

//创建第19号鱼轨迹
bool CServerGameDesk::OnTimerBuildFish19Trace() 
{
	BuildFishTrace(1, FISH_KIND_19, FISH_KIND_19);
	return true;
}

//创建第20号鱼轨迹
bool CServerGameDesk::OnTimerBuildFish20Trace() 
{
	BuildFishTrace(1, FISH_KIND_20, FISH_KIND_20);
	return true;
}

//创建李逵轨迹
bool CServerGameDesk::OnTimerBuildFishLKTrace() 
{
	CMD_S_FishTrace fish_trace;
	m_iFishNum += 1;
	DWORD build_tick = GetTickCount();
	FishTraceInfo* fish_trace_info = ActiveFishTrace();
	fish_trace_info->fish_kind = FISH_KIND_21;
	fish_trace_info->build_tick = build_tick;
	fish_trace_info->fish_id = GetNewFishID();

	fish_trace.fish_id = fish_trace_info->fish_id;
	fish_trace.fish_kind = fish_trace_info->fish_kind;
	fish_trace.init_count = 3;
	fish_trace.trace_type = TRACE_BEZIER;
	BuildInitTrace(fish_trace.init_pos, fish_trace.init_count, fish_trace.fish_kind, fish_trace.trace_type);

	SendDataToAllPlayers(&fish_trace, sizeof(fish_trace),SUB_S_FISH_TRACE);
	SendWatchData(m_bMaxPeople,&fish_trace,sizeof(fish_trace),MDM_GM_GAME_NOTIFY,SUB_S_FISH_TRACE,0);

	current_fish_lk_multiple_ = fish_multiple_[FISH_KIND_21];
	fish_id_LK = fish_trace_info->fish_id;
	SetTimer(kLKScoreTimer, kLKScoreElasped * 1000);

	return true;
}

int CServerGameDesk::GetNewFishID() 
{
	++fish_id_;
	if (fish_id_ <= 0) fish_id_ = 1;
	return fish_id_;
}

bool CServerGameDesk::OnTimerBuildFishBombTrace() 
{
	BuildFishTrace(1, FISH_KIND_23, FISH_KIND_23);
	return true;
}

bool CServerGameDesk::OnTimerBuildFishLockBombTrace() 
{
	BuildFishTrace(1, FISH_KIND_22, FISH_KIND_22);
	return true;
}

bool CServerGameDesk::OnTimerBuildFishSuperBombTrace() 
{
	BuildFishTrace(1, FISH_KIND_24, FISH_KIND_24);
	return true;
}

bool CServerGameDesk::OnTimerBuildFishSanTrace() 
{
	BuildFishTrace(2, FISH_KIND_25, FISH_KIND_27);
	return true;
}

bool CServerGameDesk::OnTimerBuildFishSiTrace() 
{
	BuildFishTrace(2, FISH_KIND_28, FISH_KIND_30);
	return true;
}

bool CServerGameDesk::OnTimerBuildFishKingTrace() 
{
	BuildFishTrace(1, FISH_KIND_31, FISH_KIND_40);
	return true;
}

bool CServerGameDesk::OnTimerClearTrace() 
{
	ClearFishTrace();
	return true;
}

bool CServerGameDesk::OnTimerBulletIonTimeout(WPARAM bind_param) 
{
	WORD chair_id = static_cast<WORD>(bind_param);
	CMD_S_BulletIonTimeout bullet_timeout;
	bullet_timeout.chair_id = chair_id;

	SendDataToAllPlayers(&bullet_timeout, sizeof(bullet_timeout),SUB_S_BULLET_ION_TIMEOUT);
	SendWatchData(m_bMaxPeople,&bullet_timeout,sizeof(bullet_timeout),MDM_GM_GAME_NOTIFY,SUB_S_BULLET_ION_TIMEOUT,0);

	return true;
}

bool CServerGameDesk::OnTimerLockTimeout() 
{
	SendDataToAllPlayers(NULL, 0,SUB_S_LOCK_TIMEOUT);
	SendWatchData(m_bMaxPeople,NULL, 0,MDM_GM_GAME_NOTIFY,SUB_S_LOCK_TIMEOUT,0);

	StartAllGameTimer();

	KillTimer(kLockTimer);
	return true;
}

//计时器_转换场景
bool CServerGameDesk::OnTimerSwitchScene()
{
	KillAllTimer();
	ClearFishTrace(true);
	special_scene_ = true;
	SetTimer(kSceneEndTimer, kSceneEndElasped * 1000);
	if (next_scene_kind_ == SCENE_KIND_1) 
	{
		BuildSceneKind1();
	} 
	else if (next_scene_kind_ == SCENE_KIND_2) 
	{
		BuildSceneKind2();
	} 
	else if (next_scene_kind_ == SCENE_KIND_3) 
	{
		BuildSceneKind3();
	} 
	else if (next_scene_kind_ == SCENE_KIND_4) 
	{
		BuildSceneKind4();
	} 
	else if (next_scene_kind_ == SCENE_KIND_5) 
	{
		BuildSceneKind5();
	}

	next_scene_kind_ = static_cast<SceneKind>((next_scene_kind_ + 1) % SCENE_KIND_COUNT);
	return true;
}

bool CServerGameDesk::OnTimerSceneEnd() 
{
	special_scene_ = false;
	StartAllGameTimer();
	KillTimer(kSceneEndTimer);
	SendDataToAllPlayers(NULL, 0,SUB_S_SCENE_END);
	SendWatchData(m_bMaxPeople,NULL, 0,MDM_GM_GAME_NOTIFY,SUB_S_SCENE_END,0);

	SendDataToAllPlayers(NULL, 0,SUB_S_LOCK_TIMEOUT);
	KillTimer(kLockTimer);
	return true;
}

bool CServerGameDesk::OnTimerLKScore() 
{
	if (current_fish_lk_multiple_ >=fish_max_multiple_[FISH_KIND_21]) 
	{
		KillTimer(kLKScoreTimer);
		return true;
	}
	CMD_S_HitFishLK hit_fish;
	hit_fish.chair_id = 3;
	hit_fish.fish_id = fish_id_LK;
	hit_fish.fish_mulriple = ++current_fish_lk_multiple_;

	SendDataToAllPlayers(&hit_fish, sizeof(hit_fish),SUB_S_HIT_FISH_LK);
	SendWatchData(m_bMaxPeople,&hit_fish, sizeof(hit_fish),MDM_GM_GAME_NOTIFY,SUB_S_HIT_FISH_LK,0);

	if (current_fish_lk_multiple_ ==fish_max_multiple_[FISH_KIND_21]) 
	{
		KillTimer(kLKScoreTimer);
	}
	return true;
}


//清除所的计时器
void CServerGameDesk::KillAllTimer()
{
	KillTimer(kBuildSmallFishTraceTimer);
	KillTimer(kBuildMediumFishTraceTimer);
	KillTimer(kBuildFish18TraceTimer);
	KillTimer(kBuildFish19TraceTimer);
	KillTimer(kBuildFish20TraceTimer);
	KillTimer(kBuildFishLKTraceTimer);
	KillTimer(kBuildFishBombTraceTimer);
	KillTimer(kBuildFishLockBombTraceTimer);
	KillTimer(kBuildFishSuperBombTraceTimer);
	KillTimer(kBuildFishSanTraceTimer);
	KillTimer(kBuildFishSiTraceTimer);
	KillTimer(kBuildFishKingTraceTimer);
	KillTimer(kLKScoreTimer);
}

//重置游戏状态
bool CServerGameDesk::ReSetGameState(BYTE bLastStation)
{
	KillAllTimer();
	KillTimer(kBulletIonTimer);
	KillTimer(kLockTimer);
	KillTimer(kClearTraceTimer);
	KillTimer(kSwitchSceneTimer);
	KillTimer(kSceneEndTimer);
	KillTimer(kLKScoreTimer);

	m_bIsplaying = false;
	SetGameStatus(GAME_STATUS_FREE);


	m_iFishNum = 0;
	
	memset(fish_score_,0,sizeof(fish_score_));
	memset(exchange_fish_score_,0,sizeof(exchange_fish_score_));
	
	for (int i = 0; i < FISH_KIND_COUNT; ++i)
	{
		FreeSweepFish(i);
	}

	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		FreeAllBulletInfo(i);
	}

	return TRUE;
}

//游戏开始
bool CServerGameDesk::GameBegin(BYTE bBeginFlag)
{
	if (__super::GameBegin(bBeginFlag)==false) 
		return false;

	SetGameStatus(GAME_STATUS_PLAY);
	StartAllGameTimer();

	SetTimer(kSwitchSceneTimer, kSwitchSceneElasped * 1000);

	SetTimer(kClearTraceTimer, kClearTraceElasped * 1000);

	return TRUE;
}

//发给所有人
void CServerGameDesk::SendDataToAllPlayers(void * pData, UINT uSize, BYTE bAssID)
{
	for (int i=0;i < PLAY_COUNT;i++) 
	{
		SendGameData(i,pData,uSize,MDM_GM_GAME_NOTIFY,bAssID,0);
	}
}
//发给指定玩家
void CServerGameDesk::SendDataToPlayer(BYTE bDeskStation, void * pData, UINT uSize, BYTE bAssID)
{
	SendGameData(bDeskStation,pData,uSize,MDM_GM_GAME_NOTIFY,bAssID,0);
}

//游戏结束
bool CServerGameDesk::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
	switch(bCloseFlag)
	{
	case GFF_SAFE_FINISH:	//安全结束 ---维护关闭房间的时候会调用 
		{
			for(int i=0; i<PLAY_COUNT; i++)
			{
				if(NULL == m_pUserInfo[i])
				{
					continue;
				}
				//只给真人结算
				if (!m_pUserInfo[i]->m_UserData.isVirtual)
				{
					//给离开的玩家结算
					//写入数据库				
					__int64 temp_point[PLAY_COUNT];
					__int64 iChangeMoney[PLAY_COUNT];
					bool temp_cut[PLAY_COUNT];
					memset(temp_point,0,sizeof(temp_point));
					memset(iChangeMoney,0,sizeof(iChangeMoney));
					memset(temp_cut,0,sizeof(temp_cut));

					temp_point[i] = (fish_score_[i] - exchange_fish_score_[i]) * exchange_ratio_userscore_ / exchange_ratio_fishscore_;

					ChangeUserPointint64(temp_point,temp_cut);
					__super::RecoderGameInfo(iChangeMoney);
				}
			}
		}
	}
	//重置数据
	ReSetGameState(bCloseFlag);
	__super::GameFinish(bDeskStation,bCloseFlag);
	return true;
}

////////////////////////////////////////////////////////////////
/////////////////////游戏辅助相关///////////////////////////////
////////////////////////////////////////////////////////////////

//判断是否正在游戏
bool CServerGameDesk::IsPlayGame(BYTE bDeskStation)
{
	return false;
	//__super::IsPlayGame(bDeskStation);
}

/*------------------------------------------------------------------------------*/
///判断此游戏桌是否开始游戏 (因为捕鱼比较特殊 玩家离开都返回false 让玩家离开桌子 而不是断线)
BOOL	CServerGameDesk::IsPlayingByGameStation()
{
	return FALSE;
}


//用户离开游戏桌
BYTE CServerGameDesk::UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo)
{
	if (NULL == m_pUserInfo[bDeskStation])
		return __super::UserLeftDesk(bDeskStation,pUserInfo);
	m_bExitWatcher[bDeskStation] = false;

	int inum = 0;//玩家人数
	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		if (NULL == m_pUserInfo[i] || bDeskStation == i)//排除自己
			continue;
		++inum;
	}

	CalcScore(bDeskStation);//算分

	DeleteNullUser(m_pUserInfo[bDeskStation]->m_UserData.dwUserID);
	if (inum == 0)//房间已经无人
	{
		GameFinish(0,GF_SALE);
	}
	if (ExamSuperUser(bDeskStation))
	{
		try
		{
			SaveSetting();
		}
		catch (...)
		{

		}
	}
	return __super::UserLeftDesk(bDeskStation,pUserInfo);
}

//提前结束
BOOL CServerGameDesk::UserStop(BYTE bDeskStation, BOOL bArgee)
{
	//SendGameData((bDeskStation+2)%4,MDM_GM_GAME_NOTIFY,ASS_STOP_THING,0);
	return true;
}

bool CServerGameDesk::UserNetCut(BYTE bDeskStation, CGameUserInfo *pLostUserInfo)
{
	//断线也做离开处理 
	UserLeftDesk(bDeskStation,pLostUserInfo);
	return true;
	//m_bExitWatcher[bDeskStation] = false;

	//int inum = 0;//玩家人数
	//for (int i = 0; i < PLAY_COUNT; ++i)
	//{
	//	if (NULL == m_pUserInfo[i] || bDeskStation == i)//排除自己
	//		continue;
	//	++inum;
	//}

	//CalcScore(bDeskStation);//算分

	//if (inum == 0)//房间已经无人
	//{
	//	GameFinish(0,GF_SALE);
	//}

	//__super:: UserNetCut(bDeskStation, pLostUserInfo);
	//return true;
}
// bool CServerGameDesk::UserReCome(BYTE bDeskStation, CGameUserInfo *pLostUserInfo)
// {
// 	fish_score_[bDeskStation] = pLostUserInfo->m_UserData.i64Money;
// 	__super::UserReCome(bDeskStation, pLostUserInfo);
// 	return true ;
// }

///用户坐到游戏桌
BYTE CServerGameDesk::UserSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo)
{
	return __super::UserSitDesk(pUserSit,pUserInfo);
}

//统计玩游戏玩家
BYTE CServerGameDesk::CountPlayer()
{
	BYTE count=0;
	for(int i=0;i<m_bMaxPeople;i++)
	{
		if(m_pUserInfo[i])
			count++;
	}
	return count;
}

void CServerGameDesk::StartAllGameTimer() 
{
	SetTimer(kBuildSmallFishTraceTimer, kBuildSmallFishTraceElasped * 1000);
	SetTimer(kBuildMediumFishTraceTimer, kBuildMediumFishTraceElasped * 1000);
	SetTimer(kBuildFish18TraceTimer, kBuildFish18TraceElasped * 1000);
	SetTimer(kBuildFish19TraceTimer, kBuildFish19TraceElasped * 1000);
	SetTimer(kBuildFish20TraceTimer, kBuildFish20TraceElasped * 1000);
	SetTimer(kBuildFishLKTraceTimer, kBuildFishLKTraceElasped * 1000);
	SetTimer(kBuildFishBombTraceTimer, kBuildFishBombTraceElasped * 1000);
	SetTimer(kBuildFishLockBombTraceTimer, kBuildFishLockBombTraceElasped * 1000);
	SetTimer(kBuildFishSuperBombTraceTimer, kBuildFishSuperBombTraceElasped * 1000);
	SetTimer(kBuildFishSanTraceTimer, kBuildFishSanTraceElasped * 1000);
	SetTimer(kBuildFishSiTraceTimer, kBuildFishSiTraceElasped * 1000);
	SetTimer(kBuildFishKingTraceTimer, kBuildFishKingTraceElasped * 1000);
}

void CServerGameDesk::BuildInitTrace(FPoint init_pos[5], int init_count, FishKind fish_kind, TraceType trace_type) {
	assert(init_count >= 2 && init_count <= 3);
	srand(GetTickCount() + rand() % kResolutionWidth);
	WORD chair_id = rand() % GAME_PLAYER;
	int center_x = kResolutionWidth / 2;
	int center_y = kResolutionHeight / 2;
	int factor = rand() % 2 == 0 ? 1 : -1;
	switch (chair_id) {
	case 0:
	case 1:
	case 2:
		init_pos[0].x = static_cast<float>(center_x + factor * (rand() % center_x));
		init_pos[0].y = 0.f - static_cast<float>(fish_bounding_box_height_[fish_kind]) * 2;
		init_pos[1].x = static_cast<float>(center_x + factor * (rand() % center_x));
		init_pos[1].y = static_cast<float>(center_y + (rand() % center_y));
		init_pos[2].x = static_cast<float>(center_x - factor * (rand() % center_x));
		init_pos[2].y = static_cast<float>(kResolutionHeight + fish_bounding_box_height_[fish_kind] * 2);
		break;
	case 3:
		init_pos[0].x = static_cast<float>(kResolutionWidth + fish_bounding_box_width_[fish_kind] * 2);
		init_pos[0].y = static_cast<float>(center_y + factor* (rand() % center_y));
		init_pos[1].x = static_cast<float>(center_x - (rand() % center_x));
		init_pos[1].y = static_cast<float>(center_y + factor* (rand() % center_y));
		init_pos[2].x = -static_cast<float>(fish_bounding_box_width_[fish_kind] * 2);
		init_pos[2].y = static_cast<float>(center_y - factor* (rand() % center_y));
		break;
	case 5:
	case 6:
	case 4:
		init_pos[0].x = static_cast<float>(center_x + factor * (rand() % center_x));
		init_pos[0].y = kResolutionHeight + static_cast<float>(fish_bounding_box_height_[fish_kind] * 2);
		init_pos[1].x = static_cast<float>(center_x + factor * (rand() % center_x));
		init_pos[1].y = static_cast<float>(center_y - (rand() % center_y));
		init_pos[2].x = static_cast<float>(center_x - factor * (rand() % center_x));
		init_pos[2].y = static_cast<float>(-fish_bounding_box_height_[fish_kind] * 2);
		break;
	case 7:
		init_pos[0].x = static_cast<float>(-fish_bounding_box_width_[fish_kind] * 2);
		init_pos[0].y = static_cast<float>(center_y + factor* (rand() % center_y));
		init_pos[1].x = static_cast<float>(center_x + (rand() % center_x));
		init_pos[1].y = static_cast<float>(center_y + factor* (rand() % center_y));
		init_pos[2].x = static_cast<float>(kResolutionWidth + fish_bounding_box_width_[fish_kind] * 2);
		init_pos[2].y = static_cast<float>(center_y - factor* (rand() % center_y));
		break;
	}

	if (trace_type == TRACE_LINEAR && init_count == 2) {
		init_pos[1].x = init_pos[2].x;
		init_pos[1].y = init_pos[2].y;
	}
}

void CServerGameDesk::BuildFishTrace(int fish_count, FishKind fish_kind_start, FishKind fish_kind_end) 
{
	//fish_count = 1;
	//if (m_iFishNum > 5)
	//{
	//	return;
	//}
	if (FISH_KIND_24 == fish_kind_start)
	{
		OutputDebugString("dwjlkpy1::创建炸弹鱼儿0");
	}
	
	m_iFishNum += fish_count;

	BYTE tcp_buffer[SOCKET_TCP_PACKET] = { 0 };
	WORD send_size = 0;
	CMD_S_FishTrace* fish_trace = reinterpret_cast<CMD_S_FishTrace*>(tcp_buffer);

	DWORD build_tick = GetTickCount();
	srand(build_tick + fish_count * 123321);
	for (int i = 0; i < fish_count; ++i) 
	{
		if (send_size + sizeof(CMD_S_FishTrace) > sizeof(tcp_buffer)) 
		{
			SendDataToAllPlayers(tcp_buffer, send_size,SUB_S_FISH_TRACE);
			SendWatchData(m_bMaxPeople,tcp_buffer, send_size,MDM_GM_GAME_NOTIFY,SUB_S_FISH_TRACE,0);

			send_size = 0;
		}

		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = static_cast<FishKind>(fish_kind_start + (rand() + i) % (fish_kind_end - fish_kind_start + 1));
		if (fish_speed_[fish_trace_info->fish_kind]>0)
		{
			fish_trace_info->build_tick = build_tick;
			fish_trace_info->fish_id = GetNewFishID();

			fish_trace->fish_id = fish_trace_info->fish_id;
			fish_trace->fish_kind = fish_trace_info->fish_kind;
			if (fish_trace_info->fish_kind == FISH_KIND_1 || fish_trace_info->fish_kind == FISH_KIND_2) 
			{
				fish_trace->init_count = 2;
				fish_trace->trace_type = TRACE_LINEAR;
			} 
			else 
			{
				fish_trace->init_count = 3;
				fish_trace->trace_type = TRACE_BEZIER;
			}
			BuildInitTrace(fish_trace->init_pos, fish_trace->init_count, fish_trace->fish_kind, fish_trace->trace_type);

			send_size += sizeof(CMD_S_FishTrace);
			++fish_trace;
		}
	}
	if (FISH_KIND_24 == fish_kind_start)
	{
		OutputDebugString("dwjlkpy1::创建炸弹鱼儿1");
	}
	if (send_size > 0) 
	{
		if (FISH_KIND_24 == fish_kind_start)
		{
			OutputDebugString("dwjlkpy1::创建炸弹鱼儿2");
		}
		SendDataToAllPlayers(tcp_buffer, send_size,SUB_S_FISH_TRACE);
		SendWatchData(m_bMaxPeople,tcp_buffer, send_size,MDM_GM_GAME_NOTIFY,SUB_S_FISH_TRACE,0);
	}
}

void CServerGameDesk::BuildSceneKind1() 
{

//	BYTE tcp_buffer[SOCKET_TCP_PACKET] = { 0 };
	CMD_S_SwitchScene TSwitchScene;
	CMD_S_SwitchScene* switch_scene = &TSwitchScene;/*reinterpret_cast<CMD_S_SwitchScene*>(tcp_buffer);*/
	switch_scene->scene_kind = next_scene_kind_;
	DWORD build_tick = GetTickCount();
	switch_scene->fish_count = 0;

	for (int i = 0; i < 100; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_1;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[i] = fish_trace_info->fish_kind;
	}
	OutputDebugString("dwjlkpy2::BuildSceneKind1-2");
	switch_scene->fish_count += 100;
	for (int i = 0; i < 17; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_3;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	OutputDebugString("dwjlkpy2::BuildSceneKind1-3");
	switch_scene->fish_count += 17;
	for (int i = 0; i < 17; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_5;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	OutputDebugString("dwjlkpy2::BuildSceneKind1-4");
	switch_scene->fish_count += 17;
	for (int i = 0; i < 30; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_2;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	OutputDebugString("dwjlkpy2::BuildSceneKind1-5");
	switch_scene->fish_count += 30;
	for (int i = 0; i < 30; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_4;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	OutputDebugString("dwjlkpy2::BuildSceneKind1-6");
	switch_scene->fish_count += 30;
	for (int i = 0; i < 15; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_6;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	OutputDebugString("dwjlkpy2::BuildSceneKind1-7");
	switch_scene->fish_count += 15;

	FishTraceInfo* fish_trace_info = ActiveFishTrace();
	fish_trace_info->fish_kind = FISH_KIND_20;
	fish_trace_info->build_tick = build_tick;
	fish_trace_info->fish_id = GetNewFishID();
	OutputDebugString("dwjlkpy2::BuildSceneKind1-8");
	switch_scene->fish_id[switch_scene->fish_count] = fish_trace_info->fish_id;
	switch_scene->fish_kind[switch_scene->fish_count] = fish_trace_info->fish_kind;
	switch_scene->fish_count += 1;
	OutputDebugString("dwjlkpy2::BuildSceneKind1-9");
	SendDataToAllPlayers(switch_scene, sizeof(CMD_S_SwitchScene),SUB_S_SWITCH_SCENE);
	SendWatchData(m_bMaxPeople,switch_scene, sizeof(CMD_S_SwitchScene),MDM_GM_GAME_NOTIFY,SUB_S_SWITCH_SCENE,0);
	OutputDebugString("dwjlkpy2::BuildSceneKind1-10");
}

void CServerGameDesk::BuildSceneKind2() 
{
	//	BYTE tcp_buffer[SOCKET_TCP_PACKET] = { 0 };
	CMD_S_SwitchScene TSwitchScene;
	CMD_S_SwitchScene* switch_scene = &TSwitchScene;/*reinterpret_cast<CMD_S_SwitchScene*>(tcp_buffer);*/
	switch_scene->scene_kind = next_scene_kind_;
	DWORD build_tick = GetTickCount();
	switch_scene->fish_count = 0;
	for (int i = 0; i < 200; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_1;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 200;
	for (int i = 0; i < 14; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = static_cast<FishKind>(FISH_KIND_12 + i % 7);
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 14;

	SendDataToAllPlayers(switch_scene, sizeof(CMD_S_SwitchScene),SUB_S_SWITCH_SCENE);
	SendWatchData(m_bMaxPeople,switch_scene, sizeof(CMD_S_SwitchScene),MDM_GM_GAME_NOTIFY,SUB_S_SWITCH_SCENE,0);
}

void CServerGameDesk::BuildSceneKind3() 
{
	//	BYTE tcp_buffer[SOCKET_TCP_PACKET] = { 0 };
	CMD_S_SwitchScene TSwitchScene;
	CMD_S_SwitchScene* switch_scene = &TSwitchScene;/*reinterpret_cast<CMD_S_SwitchScene*>(tcp_buffer);*/
	switch_scene->scene_kind = next_scene_kind_;
	DWORD build_tick = GetTickCount();
	switch_scene->fish_count = 0;
	for (int i = 0; i < 50; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_1;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 50;

	for (int i = 0; i < 40; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_3;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 40;

	for (int i = 0; i < 30; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_4;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 30;

	FishTraceInfo* fish_trace_info = ActiveFishTrace();
	fish_trace_info->fish_kind = FISH_KIND_16;
	fish_trace_info->build_tick = build_tick;
	fish_trace_info->fish_id = GetNewFishID();

	switch_scene->fish_id[switch_scene->fish_count] = fish_trace_info->fish_id;
	switch_scene->fish_kind[switch_scene->fish_count] = fish_trace_info->fish_kind;
	switch_scene->fish_count += 1;

	for (int i = 0; i < 50; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_1;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 50;

	for (int i = 0; i < 40; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_2;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 40;

	for (int i = 0; i < 30; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_5;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 30;

	fish_trace_info = ActiveFishTrace();
	fish_trace_info->fish_kind = FISH_KIND_17;
	fish_trace_info->build_tick = build_tick;
	fish_trace_info->fish_id = GetNewFishID();

	switch_scene->fish_id[switch_scene->fish_count] = fish_trace_info->fish_id;
	switch_scene->fish_kind[switch_scene->fish_count] = fish_trace_info->fish_kind;
	switch_scene->fish_count += 1;

	SendDataToAllPlayers(switch_scene, sizeof(CMD_S_SwitchScene),SUB_S_SWITCH_SCENE);
	SendWatchData(m_bMaxPeople,switch_scene, sizeof(CMD_S_SwitchScene),MDM_GM_GAME_NOTIFY,SUB_S_SWITCH_SCENE,0);
}

void CServerGameDesk::BuildSceneKind4() 
{
	//	BYTE tcp_buffer[SOCKET_TCP_PACKET] = { 0 };
	CMD_S_SwitchScene TSwitchScene;
	CMD_S_SwitchScene* switch_scene = &TSwitchScene;/*reinterpret_cast<CMD_S_SwitchScene*>(tcp_buffer);*/
	switch_scene->scene_kind = next_scene_kind_;
	DWORD build_tick = GetTickCount();
	switch_scene->fish_count = 0;
	for (int i = 0; i < 8; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_11;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 8;
	for (int i = 0; i < 8; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_12;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 8;
	for (int i = 0; i < 8; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_13;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 8;
	for (int i = 0; i < 8; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_14;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 8;
	for (int i = 0; i < 8; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_15;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 8;
	for (int i = 0; i < 8; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_16;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 8;
	for (int i = 0; i < 8; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_17;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 8;
	for (int i = 0; i < 8; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_18;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 8;

	SendDataToAllPlayers(switch_scene, sizeof(CMD_S_SwitchScene),SUB_S_SWITCH_SCENE);
	SendWatchData(m_bMaxPeople,switch_scene, sizeof(CMD_S_SwitchScene),MDM_GM_GAME_NOTIFY,SUB_S_SWITCH_SCENE,0);
}

void CServerGameDesk::BuildSceneKind5() 
{
	//	BYTE tcp_buffer[SOCKET_TCP_PACKET] = { 0 };
	CMD_S_SwitchScene TSwitchScene;
	CMD_S_SwitchScene* switch_scene = &TSwitchScene;/*reinterpret_cast<CMD_S_SwitchScene*>(tcp_buffer);*/
	switch_scene->scene_kind = next_scene_kind_;
	DWORD build_tick = GetTickCount();
	switch_scene->fish_count = 0;
	for (int i = 0; i < 40; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_1;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 40;
	for (int i = 0; i < 40; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_2;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 40;
	for (int i = 0; i < 40; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_5;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 40;
	for (int i = 0; i < 40; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_3;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 40;
	for (int i = 0; i < 24; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_4;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 24;
	for (int i = 0; i < 24; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_6;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 24;
	for (int i = 0; i < 13; ++i) {
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_7;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 13;
	for (int i = 0; i < 13; ++i) 
	{
		FishTraceInfo* fish_trace_info = ActiveFishTrace();
		fish_trace_info->fish_kind = FISH_KIND_6;
		fish_trace_info->build_tick = build_tick;
		fish_trace_info->fish_id = GetNewFishID();

		switch_scene->fish_id[switch_scene->fish_count + i] = fish_trace_info->fish_id;
		switch_scene->fish_kind[switch_scene->fish_count + i] = fish_trace_info->fish_kind;
	}
	switch_scene->fish_count += 13;

	FishTraceInfo* fish_trace_info = ActiveFishTrace();
	fish_trace_info->fish_kind = FISH_KIND_18;
	fish_trace_info->build_tick = build_tick;
	fish_trace_info->fish_id = GetNewFishID();
	switch_scene->fish_id[switch_scene->fish_count] = fish_trace_info->fish_id;
	switch_scene->fish_kind[switch_scene->fish_count] = fish_trace_info->fish_kind;
	switch_scene->fish_count += 1;

	fish_trace_info = ActiveFishTrace();
	fish_trace_info->fish_kind = FISH_KIND_17;
	fish_trace_info->build_tick = build_tick;
	fish_trace_info->fish_id = GetNewFishID();
	switch_scene->fish_id[switch_scene->fish_count] = fish_trace_info->fish_id;
	switch_scene->fish_kind[switch_scene->fish_count] = fish_trace_info->fish_kind;
	switch_scene->fish_count += 1;

	SendDataToAllPlayers(switch_scene, sizeof(CMD_S_SwitchScene),SUB_S_SWITCH_SCENE);
	SendWatchData(m_bMaxPeople,switch_scene, sizeof(CMD_S_SwitchScene),MDM_GM_GAME_NOTIFY,SUB_S_SWITCH_SCENE,0);
}

void CServerGameDesk::ClearFishTrace(bool force) 
{
	if (force) 
	{
		std::copy(active_fish_trace_vector_.begin(), active_fish_trace_vector_.end(), std::back_inserter(storage_fish_trace_vector_));
		active_fish_trace_vector_.clear();
	} 
	else 
	{
		FishTraceInfoVecor::iterator iter;
		FishTraceInfo* fish_trace_info = NULL;
		DWORD now_tick = GetTickCount();
		for (iter = active_fish_trace_vector_.begin(); iter != active_fish_trace_vector_.end();) 
		{
			fish_trace_info = *iter;
			if (now_tick >= (fish_trace_info->build_tick + kFishAliveTime)) 
			{
				iter = active_fish_trace_vector_.erase(iter);
				storage_fish_trace_vector_.push_back(fish_trace_info);
			} 
			else 
			{
				++iter;
			}
		}
	}
}

void CServerGameDesk::CalcScore(BYTE bDeskStation) 
{
	if (m_pUserInfo[bDeskStation] == NULL) return;
	//给离开的玩家结算
	//写入数据库				
	__int64 temp_point[PLAY_COUNT];
	__int64 iChangeMoney[PLAY_COUNT];
	bool temp_cut[PLAY_COUNT];
	memset(temp_point,0,sizeof(temp_point));
	memset(iChangeMoney,0,sizeof(iChangeMoney));
	memset(temp_cut,0,sizeof(temp_cut));

	temp_point[bDeskStation] = (fish_score_[bDeskStation] - exchange_fish_score_[bDeskStation]) * exchange_ratio_userscore_ / exchange_ratio_fishscore_;
	temp_cut[bDeskStation] = 1;
	ChangeUserPointint64(temp_point,temp_cut,bDeskStation/*,1*/);
	__super::RecoderGameInfo(iChangeMoney);
	SaveIni();
	//数据清零
	fish_score_[bDeskStation] = 0;
	exchange_fish_score_[bDeskStation] = 0;
}

bool CServerGameDesk::OnEventGameStart() 
{
	return true;
}

//游戏结束事件
bool CServerGameDesk::OnEventGameConclude(WORD chair_id, BYTE reason) 
{
	//if (reason == GER_DISMISS) 
	//{
	//	for (WORD i = 0; i < GAME_PLAYER; ++i) 
	//	{
	//		if (m_pUserInfo[i] == NULL)
	//			continue;
	//		CalcScore(i);
	//	}

	//	GameFinish(0,GF_NORMAL);
	//	KillAllTimer();
	//	KillTimer(kSwitchSceneTimer);
	//	KillTimer(kClearTraceTimer);
	//	ClearFishTrace(true);
	//	next_scene_kind_ = SCENE_KIND_1;
	//	special_scene_ = false;
	//	android_chairid_ = INVALID_CHAIR;
	//} 
	//else if (chair_id < GAME_PLAYER && m_pUserInfo[chair_id] != NULL) 
	//{
	//	CalcScore(chair_id);
	//}
	return true;
}

//获取游戏状态信息
bool CServerGameDesk::OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{
	if (m_pUserInfo[bDeskStation] == NULL)
	{
		return false;
	}
	if (!m_bIsplaying)
	{
		m_bIsplaying = true;
		GameBegin(ALL_ARGEE);
	}
	
	GameStation cmd;
	cmd.game_status = GetGameStatus();

	//不允许旁观
	if (bWatchUser)
	{
		return false;
		m_bExitWatcher[bDeskStation] = true;
	}
	
	if (!OnEventSendGameScene(bDeskStation, cmd.game_status,cmd))
	{
		return true;
	}
	InsertNewUser(m_pUserInfo[bDeskStation]->m_UserData.dwUserID,m_pUserInfo[bDeskStation]->m_UserData.i64Money);
	SendGameStation(bDeskStation,uSocketID,bWatchUser,&cmd,sizeof(GameStation));
	S_C_BulletPerameter TBulletPerameter;
	TBulletPerameter.iFireFrequency = m_iFireFrequency;
	SendGameData(bDeskStation,&TBulletPerameter,sizeof(TBulletPerameter),MDM_GM_GAME_NOTIFY,S_C_BULLET_PERAMETER_SIG,0);
	return true;
}

bool CServerGameDesk::OnEventSendGameScene(WORD chair_id, BYTE game_status, GameStation& cmd) 
{
	switch (game_status) 
	{
	case GAME_STATUS_FREE:
	case GAME_STATUS_PLAY:
		//配置数据
		SendGameConfig(chair_id, cmd);
		SendSuperConfig(chair_id);
		//状态数据
		cmd.GameStatus.game_version = GAME_VERSION;
// 		for(int i=0; i<PLAY_COUNT; i++)
// 		{
// 			if(IsRobot(i))
// 				fish_score_[i] = GetUserMoney(i);
// 		}
		if(IsRobot(chair_id)) //如果是机器人，帮他换好鱼币
		{
			fish_score_[chair_id] = GetRandScore(GetUserMoney(chair_id)* (exchange_ratio_fishscore_/exchange_ratio_userscore_));
			exchange_fish_score_[chair_id] = fish_score_[chair_id];
		}
		memcpy(cmd.GameStatus.fish_score, fish_score_, sizeof(cmd.GameStatus.fish_score));
		memcpy(cmd.GameStatus.exchange_fish_score, exchange_fish_score_, sizeof(cmd.GameStatus.exchange_fish_score));
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if (NULL == m_pUserInfo[i])
			{
				continue;
			}
			cmd.GameStatus.user_leave_score[i] = m_pUserInfo[i]->m_UserData.i64Money - exchange_fish_score_[i] * exchange_ratio_userscore_ / exchange_ratio_fishscore_;
		}
		
		//游戏消息
		BYTE mess[] = TEXT("键盘↑↓键加减炮，→←键上下分，空格键或鼠标左键发射子弹，F4关闭声音!");
		CopyMemory(cmd.GameMessage,mess,sizeof(BYTE)*MAX_PATH);
		return true;
	}
	return false;
}

void CServerGameDesk::SendSuperConfig(BYTE bDeskStation)
{
	S_C_SUPER_CONFIG TSuperConfig;
	TSuperConfig.bSuperUser = ExamSuperUser(bDeskStation);
	SendGameData(bDeskStation,&TSuperConfig,sizeof(TSuperConfig),MDM_GM_GAME_NOTIFY,S_C_SUPER_CONFIG_SIG,0);
}

bool CServerGameDesk::SendGameConfig(BYTE bDeskStation, GameStation& cmd) 
{
	cmd.GameConfig.exchange_ratio_userscore = exchange_ratio_userscore_;
	cmd.GameConfig.exchange_ratio_fishscore = exchange_ratio_fishscore_;
	cmd.GameConfig.exchange_count = exchange_count_;
	cmd.GameConfig.min_bullet_multiple = min_bullet_multiple_;
	cmd.GameConfig.max_bullet_multiple = max_bullet_multiple_;
	cmd.GameConfig.bomb_range_width = bomb_range_width_;
	cmd.GameConfig.bomb_range_height = bomb_range_height_;
	cmd.GameConfig.scene_kind_ = next_scene_kind_;
	for (int i = 0; i < FISH_KIND_COUNT; ++i) 
	{
		cmd.GameConfig.fish_min_multiple[i] = fish_multiple_[i];
		cmd.GameConfig.fish_max_multiple[i]=fish_max_multiple_[i];
		cmd.GameConfig.fish_speed[i] = fish_speed_[i];
		cmd.GameConfig.fish_bounding_box_width[i] = fish_bounding_box_width_[i];
		cmd.GameConfig.fish_bounding_box_height[i] = fish_bounding_box_height_[i];
		cmd.GameConfig.fish_hit_radius[i] = fish_hit_radius_[i];
	}
	//cmd.GameConfig.fish18_max_mutiple = fish18_max_multiple_;
	//cmd.GameConfig.fish19_max_mutiple = fish19_max_multiple_;
	//cmd.GameConfig.fishLK_max_mutiple = fishLK_max_multiple_;


	for (int i = 0; i < BULLET_KIND_COUNT; ++i) 
	{
		cmd.GameConfig.bullet_speed[i] = bullet_speed_[i];
		cmd.GameConfig.net_radius[i] = net_radius_[i];
	}

	return true;
}

FishTraceInfo* CServerGameDesk::GetFishTraceInfo(int fish_id) {
	FishTraceInfoVecor::iterator iter;
	FishTraceInfo* fish_trace_info = NULL;
	for (iter = active_fish_trace_vector_.begin(); iter != active_fish_trace_vector_.end(); ++iter) {
		fish_trace_info = *iter;
		if (fish_trace_info->fish_id == fish_id) return fish_trace_info;
	}

	return NULL;
}

void CServerGameDesk::SaveSweepFish(FishKind fish_kind, int fish_id, BulletKind bullet_kind, int bullet_mulriple) {
	SweepFishInfo sweep_fish;
	sweep_fish.fish_kind = fish_kind;
	sweep_fish.fish_id = fish_id;
	sweep_fish.bullet_kind = bullet_kind;
	sweep_fish.bullet_mulriple = bullet_mulriple;
	sweep_fish_info_vector_.push_back(sweep_fish);
}

bool CServerGameDesk::FreeSweepFish(int fish_id) {
	std::vector<SweepFishInfo>::iterator iter;
	for (iter = sweep_fish_info_vector_.begin(); iter != sweep_fish_info_vector_.end(); ++iter) {
		if ((*iter).fish_id == fish_id) {
			sweep_fish_info_vector_.erase(iter);
			return true;
		}
	}

	assert(!"FreeSweepFish Failed");
	return false;
}

//获得
SweepFishInfo* CServerGameDesk::GetSweepFish(int fish_id) {
	std::vector<SweepFishInfo>::iterator iter;
	for (iter = sweep_fish_info_vector_.begin(); iter != sweep_fish_info_vector_.end(); ++iter) {
		if ((*iter).fish_id == fish_id) {
			return &(*iter);
		}
	}
	return NULL;
}

//从仓库中激活一颗子弹数据给指定玩家
ServerBulletInfo* CServerGameDesk::ActiveBulletInfo(WORD chairid) {
	ServerBulletInfo* bullet_info = NULL;
	if (storage_bullet_info_vector_.size() > 0) {
		bullet_info = storage_bullet_info_vector_.back();
		storage_bullet_info_vector_.pop_back();
		server_bullet_info_vector_[chairid].push_back(bullet_info);
	}

	if (bullet_info == NULL) {
		bullet_info = new ServerBulletInfo();
		server_bullet_info_vector_[chairid].push_back(bullet_info);
	}

	return bullet_info;
}

//清除指定玩家指定子弹信息，并将清除的数据保存到仓库中
bool CServerGameDesk::FreeBulletInfo(WORD chairid, ServerBulletInfo* bullet_info) {
	ServerBulletInfoVector::iterator iter;
	for (iter = server_bullet_info_vector_[chairid].begin(); iter != server_bullet_info_vector_[chairid].end(); ++iter) {
		if (bullet_info == *iter) {
			server_bullet_info_vector_[chairid].erase(iter);
			storage_bullet_info_vector_.push_back(bullet_info);
			return true;
		}
	}

	assert(!"FreeBulletInfo Failed");
	return false;
}

//清理指定玩家所有子弹信息
void CServerGameDesk::FreeAllBulletInfo(WORD chairid) {
	std::copy(server_bullet_info_vector_[chairid].begin(), server_bullet_info_vector_[chairid].end(), std::back_inserter(storage_bullet_info_vector_));
	server_bullet_info_vector_[chairid].clear();
}

//获取子弹信息
ServerBulletInfo* CServerGameDesk::GetBulletInfo(WORD chairid, int bullet_id) {
	ServerBulletInfoVector::iterator iter;
	ServerBulletInfo* bullet_info = NULL;
	for (iter = server_bullet_info_vector_[chairid].begin(); iter != server_bullet_info_vector_[chairid].end(); ++iter) {
		bullet_info = *iter;
		if (bullet_info->bullet_id == bullet_id) return bullet_info;
	}
	//assert(!"GetBulletInfo:not found");
	return NULL;
}
void CServerGameDesk::UTF_8ToUnicode(wchar_t* pOut,char *pText)  
{     
	char* uchar = (char *)pOut;   
	uchar[1] = ((pText[0] & 0x0F) << 4) + ((pText[1] >> 2) & 0x0F);  
	uchar[0] = ((pText[1] & 0x03) << 6) + (pText[2] & 0x3F);   
}   

void CServerGameDesk::UnicodeToGB2312(char* pOut,wchar_t uData)  
{   
	WideCharToMultiByte(CP_ACP,NULL,&uData,1,pOut,sizeof(wchar_t),NULL,NULL);   
} 

void CServerGameDesk::UTF_8ToGB2312(char*pOut, char *pText, int pLen)    
{     
	char Ctemp[4];     
	memset(Ctemp,0,4);   
	int i =0 ,j = 0;   
	while(i < pLen)   
	{    
		if(pText[i] >= 0)    
		{    
			pOut[j++] = pText[i++];   
		}   
		else   
		{    
			WCHAR Wtemp;   
			UTF_8ToUnicode(&Wtemp,pText + i);  
			UnicodeToGB2312(Ctemp,Wtemp);   
			pOut[j] = Ctemp[0];  
			pOut[j + 1] = Ctemp[1];    
			i += 3;     
			j += 2;    
		}     
	}
	
	pOut[j] ='\0';
	return;   
}   

bool CServerGameDesk::LoadConfig() 
{
	TCHAR file_name[MAX_PATH] = { 0 };
	_sntprintf(file_name, CountArray(file_name), TEXT("lkpy_config_%s.xml"), SKIN_FOLDER);

	TiXmlDocument xml_doc;
	if (!xml_doc.LoadFile(CT2A(file_name), TIXML_ENCODING_UTF8)) 
	{
		return false;
	}
	const TiXmlElement* xml_element = xml_doc.FirstChildElement("Config");
	if (xml_element == NULL) 
	{
		return false;
	}
	g_AdminiList.clear();
	const TiXmlElement* xml_child = NULL;
	int fish_count = 0, bullet_kind_count = 0;
	memset(fish_speed_,0,sizeof(fish_speed_));
	for (xml_child = xml_element->FirstChildElement(); xml_child; xml_child = xml_child->NextSiblingElement()) 
	{
		if (!strcmp(xml_child->Value(), "Stock")) 
		{
			//库存(不能超过10个):stockScore库存分数,大于些值执行相应概率,从最大的开始判断,小于0将打不中鱼
			for (const TiXmlElement* xml_stock = xml_child->FirstChildElement(); xml_stock; xml_stock = xml_stock->NextSiblingElement()) 
			{
				xml_stock->Attribute("stockScore", &stock_crucial_score_[stock_crucial_count_]);
				xml_stock->Attribute("increaseProbability", &stock_increase_probability_[stock_crucial_count_]);
				++stock_crucial_count_;
				if (stock_crucial_count_ >= 10) 
					break;
			}
		} 
		else if (!strcmp(xml_child->Value(), "CellScore"))
		{
			xml_child->Attribute("score", &m_iCellScore);
		}
		else if (!strcmp(xml_child->Value(), "SuperMan"))
		{
			int iUserID;
			xml_child->Attribute("UserId", &iUserID);
			g_AdminiList.push_back(iUserID);
		}
		else if (!strcmp(xml_child->Value(), "FireFrequency"))
		{
			xml_child->Attribute("Frequency", &m_iFireFrequency);
		}
		else if (!strcmp(xml_child->Value(), "ScoreExchange")) 
		{
			//金币和渔币的兑换(金币:渔币) exchangeCount:每次兑换数量
			const char* attri = xml_child->Attribute("exchangeRatio");
			char* temp = NULL;
			exchange_ratio_userscore_ = strtol(attri, &temp, 10);
			exchange_ratio_fishscore_ = strtol(temp + 1, &temp, 10);
			xml_child->Attribute("exchangeCount", &exchange_count_);
		} 
		else if (!strcmp(xml_child->Value(), "Cannon")) 
		{
			const char* attri = xml_child->Attribute("cannonMultiple");//大炮倍数
			char* temp = NULL;
			min_bullet_multiple_ = strtol(attri, &temp, 10);
			max_bullet_multiple_ = strtol(temp + 1, &temp, 10);
		} 
		else if (!strcmp(xml_child->Value(), "Bomb"))//炸弹(局部炸弹,超级炸弹) 当库存小于此值炸弹被击中的几率为0
		{
			const char* attri = xml_child->Attribute("BombProbability");
			char* temp = NULL;
			bomb_stock_ = strtol(attri, &temp, 10);
			super_bomb_stock_ = strtol(temp + 1, &temp, 10);
		} 
		else if (!strcmp(xml_child->Value(), "Fish")) 
		{
			//<!-- kind: 对应 CMD_Fish.h文件的FishKind的枚举值 不能更改 
			// name:只作为描述 speed:鱼的速度 multiple:鱼的倍数 BoundingBox:鱼的边框 hitRadius:击中半径 captureProbability:捕获概率(0-1)  -->
			//<!-- 特殊鱼 鱼王和超级炸弹是圆的 用BoundingBox的第1个参数作为直径 -->
			//<!-- 特殊鱼 局部炸弹的BoundingBox的后2个参数表示炸弹的涉及范围 -->
			//<!-- 特殊鱼 FISH_KIND_18(鲨鱼)FISH_KIND_19(龙)和FISH_KIND_21(李逵)的multiple是随机倍数的2个参数表示最小和最大 -->
			int fish_kind;
			
			xml_child->Attribute("kind", &fish_kind);
			if (fish_kind >= FISH_KIND_COUNT || fish_kind < 0) 
				return false;

			const char* cFishName = xml_child->Attribute("name");
			UTF_8ToGB2312(m_cFishName[fish_kind], (char*)cFishName, strlen(cFishName));

			xml_child->Attribute("speed", &fish_speed_[fish_kind]);
			//if (fish_kind == FISH_KIND_18) 
			//{
			//	const char* attri = xml_child->Attribute("multiple");
			//	char* temp = NULL;
			//	fish_multiple_[fish_kind] = strtol(attri, &temp, 10);
			//	fish18_max_multiple_ = strtol(temp + 1, &temp, 10);
			//} 
			//else if (fish_kind == FISH_KIND_19) 
			//{
			//	const char* attri = xml_child->Attribute("multiple");
			//	char* temp = NULL;
			//	fish_multiple_[fish_kind] = strtol(attri, &temp, 10);
			//	fish19_max_multiple_ = strtol(temp + 1, &temp, 10);
			//} 
			//else if (fish_kind == FISH_KIND_21) 
			//{
			//	const char* attri = xml_child->Attribute("multiple");
			//	char* temp = NULL;
			//	fish_multiple_[fish_kind] = strtol(attri, &temp, 10);
			//	fishLK_max_multiple_ = strtol(temp + 1, &temp, 10);
			//} 
			//else 
			{
				const char* attri = xml_child->Attribute("multiple");
				char* temp = NULL;
				fish_multiple_[fish_kind] = strtol(attri, &temp, 10);
				if ((*temp)!='\0')
				{
					fish_max_multiple_[fish_kind] = strtol(temp + 1, &temp,10);
				}
				else
				{
					fish_max_multiple_[fish_kind] =0;
				}
			}
			const char* attri = xml_child->Attribute("BoundingBox");
			char* temp = NULL;
			fish_bounding_box_width_[fish_kind] = strtol(attri, &temp, 10);
			fish_bounding_box_height_[fish_kind] = strtol(temp + 1, &temp, 10);
			if (fish_kind == FISH_KIND_23) 
			{
				bomb_range_width_ = strtol(temp + 1, &temp, 10);
				bomb_range_height_ = strtol(temp + 1, &temp, 10);
			}
			xml_child->Attribute("hitRadius", &fish_hit_radius_[fish_kind]);
			xml_child->Attribute("captureProbability", &fish_capture_probability_[fish_kind]);
			++fish_count;
		} 
		else if (!strcmp(xml_child->Value(), "Bullet")) 
		{
			//kind:对应BulletKind name:描述 speed:子弹速度 netRadius:渔网的半径
			int bullet_kind;
			xml_child->Attribute("kind", &bullet_kind);
			if (bullet_kind >= BULLET_KIND_COUNT || bullet_kind < 0) 
				return false;
			xml_child->Attribute("speed", &bullet_speed_[bullet_kind]);
			xml_child->Attribute("netRadius", &net_radius_[bullet_kind]);
			++bullet_kind_count;
		}
		else if(!strcmp(xml_child->Value(), "BuildFish")) 
		{
			//<!-- BuildFish:各种鱼生成间歇时间 BuildTime 单位秒-->
			//<!-- 鱼依次为0：小鱼出现时长、1：中等鱼出现时长、2：18号鱼出现时长、3：19号鱼出现时长、4：20号鱼出现时长-->
			//<!--        5：李逵出现时长、6：炸弹鱼出现时长、7：超级炸弹鱼出现时长、8：锁定炸弹鱼出现时长-->

			int iBuildID = -1;
			int iBuildTime = 0;
			xml_child->Attribute("BuildID", &iBuildID);
			xml_child->Attribute("BuildTime", &iBuildTime);
			if (iBuildID == 0)
			{
				if (iBuildTime > 0)
				{
					kBuildSmallFishTraceElasped = iBuildTime;
				}
			}
			else if (iBuildID == 1)
			{
				if (iBuildTime > 0)
				{
					kBuildMediumFishTraceElasped = iBuildTime;
				}
			}
			else if (iBuildID == 2)
			{
				if (iBuildTime > 0)
				{
					kBuildFish18TraceElasped = iBuildTime;
				}
			}
			else if (iBuildID == 3)
			{
				if (iBuildTime > 0)
				{
					kBuildFish19TraceElasped = iBuildTime;
				}
			}
			else if (iBuildID == 4)
			{
				if (iBuildTime > 0)
				{
					kBuildFish20TraceElasped = iBuildTime;
				}
			}
			else if (iBuildID == 5)
			{
				if (iBuildTime > 0)
				{
					kBuildFishLKTraceElasped = iBuildTime;
				}
			}
			else if (iBuildID == 6)
			{
				if (iBuildTime > 0)
				{
					kBuildFishBombTraceElasped = iBuildTime;
				}
			}
			else if (iBuildID == 7)
			{
				if (iBuildTime > 0)
				{
					kBuildFishSuperBombTraceElasped = iBuildTime;
				}
			}
			else if (iBuildID == 8)
			{
				if (iBuildTime > 0)
				{
					kBuildFishLockBombTraceElasped = iBuildTime;
				}
			}
			else if (iBuildID == 9)
			{
				if (iBuildTime > 0)
				{
					kBuildFishSanTraceElasped = iBuildTime;
				}
			}
			else if (iBuildID == 10)
			{
				if (iBuildTime > 0)
				{
					kBuildFishSiTraceElasped = iBuildTime;
				}
			}
			else if (iBuildID == 11)
			{
				if (iBuildTime > 0)
				{
					kBuildFishKingTraceElasped = iBuildTime;
				}
			}
		}
	}

	if (fish_count != FISH_KIND_COUNT) 
		return false;
	if (bullet_kind_count != BULLET_KIND_COUNT) 
		return false;

	return true;
}


//获取该玩家的单位分数
int CServerGameDesk::GetSuperCellscore(BYTE bDesk)
{
	return m_iCellScore;
}

//根据房间ID加载ini配置文件
BOOL CServerGameDesk::LoadExtIni(UINT uRoomID)
{
	//CString s = CINIFile::GetAppPath ();/////本地路径
	//CINIFile f( s +SKIN_FOLDER +".ini");

	//CString str;
	//str.Format("11100400_%d",uRoomID);
	//CString key =str;
	//m_iBeginNt = f.GetKeyVal(key,"beginnt",m_iBeginNt);
	//m_iEndNt = f.GetKeyVal(key,"endnt",m_iEndNt);

	//::memset(m_iPlayNtNum,m_iBeginNt,sizeof(m_iPlayNtNum));
	return TRUE;
}

FishTraceInfo* CServerGameDesk::ActiveFishTrace() 
{
	FishTraceInfo* fish_trace_info = NULL;
	if (storage_fish_trace_vector_.size() > 0) 
	{
		fish_trace_info = storage_fish_trace_vector_.back();
		storage_fish_trace_vector_.pop_back();
		active_fish_trace_vector_.push_back(fish_trace_info);
	}

	if (fish_trace_info == NULL) 
	{
		fish_trace_info = new FishTraceInfo;
		active_fish_trace_vector_.push_back(fish_trace_info);
	}

	return fish_trace_info;
}

bool CServerGameDesk::FreeFishTrace(FishTraceInfo* fish_trace_info) {
	FishTraceInfoVecor::iterator iter;
	for (iter = active_fish_trace_vector_.begin(); iter != active_fish_trace_vector_.end(); ++iter) {
		if (fish_trace_info == *iter) {
			active_fish_trace_vector_.erase(iter);
			storage_fish_trace_vector_.push_back(fish_trace_info);

			m_iFishNum--;
			return true;
		}
	}

	assert(!"FreeFishTrace Failed");
	return false;
}

//存在真人
bool CServerGameDesk::ExistUnAI()
{
	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		if(m_pUserInfo[i] == NULL)
			continue;
		if (m_pUserInfo[i]->m_UserData.isVirtual == 0 || m_bExitWatcher[i])//存在真人
		{
			return true;
		}
	}

	return false;
}

bool CServerGameDesk::OnSubUserFire(BYTE bDeskStation, BulletKind bullet_kind, float angle, int bullet_mul, int lock_fishid,int iBullet_id) 
{
	if (NULL == m_pUserInfo[bDeskStation])
		return true;
	//sdp机器人和真人一样消耗子弹2014.05.08
	// 没真实玩家机器人不打炮
	//if (!ExistUnAI() ) 
	//{
	//	return true;
	//}

	if (bullet_mul < min_bullet_multiple_ || bullet_mul > max_bullet_multiple_) 
	{
		return true;
	}

	WORD chair_id = bDeskStation;
	assert(fish_score_[chair_id] >= bullet_mul);
	if (fish_score_[chair_id] < bullet_mul) 
	{
			return true;
	}

	fish_score_[chair_id] -= bullet_mul;

	if (!m_pUserInfo[bDeskStation]->m_UserData.isVirtual)
	{
		int revenue = /*game_service_option_->wRevenueRatio*/0 * bullet_mul / 100;//税收比例
		g_stock_score_ += bullet_mul - revenue;
		g_revenue_score += revenue;
	}
	if (lock_fishid > 0 && GetFishTraceInfo(lock_fishid) == NULL) 
	{
		lock_fishid = 0;
	}

	CMD_S_UserFire user_fire;
	user_fire.bullet_kind = bullet_kind;
	user_fire.bullet_id = iBullet_id;//GetBulletID(chair_id);
	user_fire.angle = angle;
	user_fire.chair_id = bDeskStation;
	user_fire.android_chairid = /*m_pUserInfo[bDeskStation]->m_UserData.isVirtual ? android_chairid_ :*/ INVALID_CHAIR;//sdp机器人和真人一样消耗子弹2014.05.08
	user_fire.bullet_mulriple = bullet_mul;
	user_fire.fish_score = -bullet_mul;
	user_fire.lock_fishid = lock_fishid;
	user_fire.bIsRobot=m_pUserInfo[bDeskStation]->m_UserData.isVirtual;
	SendDataToAllPlayers(&user_fire, sizeof(user_fire), SUB_S_USER_FIRE);
	SendWatchData(m_bMaxPeople,&user_fire, sizeof(user_fire),MDM_GM_GAME_NOTIFY,SUB_S_USER_FIRE,0);
	ServerBulletInfo* bullet_info = ActiveBulletInfo(chair_id);
	bullet_info->bullet_id = user_fire.bullet_id;
	bullet_info->bullet_kind = user_fire.bullet_kind;
	bullet_info->bullet_mulriple = user_fire.bullet_mulriple;
	UpDateRealMoney(m_pUserInfo[bDeskStation]->m_UserData.dwUserID,fish_score_[bDeskStation]);


	UpdateControllerMoney(bDeskStation,bullet_mul);
	TidyControlData();

	return true;
}

int CServerGameDesk::GetBulletID(WORD chairid) 
{
	ASSERT(chairid < GAME_PLAYER);
	++bullet_id_[chairid];
	if (bullet_id_[chairid] <= 0) bullet_id_[chairid] = 1;
	return bullet_id_[chairid];
}
//---------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//修改奖池数据1(平台->游戏)
/// @param bAIWinAndLostAutoCtrl 机器人控制输赢开关
/// @return 是否成功
bool CServerGameDesk::SetRoomPond(bool	bAIWinAndLostAutoCtrl)
{
	return false;
}

//修改奖池数据2(平台->游戏)
/// @param m_iAIWantWinMoney 机器人输赢控制：机器人赢钱区域1,2,3 4所用断点
/// @param m_iAIWinLuckyAt 机器人输赢控制：机器人在区域1,2,3,4赢钱的概率
/// @return 是否成功
bool CServerGameDesk::SetRoomPondEx(__int64	iAIWantWinMoney[], int	iAIWinLuckyAt[])
{
	return false;
}

//修改奖池数据3(平台->游戏)
/// @param iReSetAIHaveWinMoney 机器人输赢控制：重置机器人已经赢钱的数目
/// @return 是否成功
bool CServerGameDesk::SetRoomPondEx(__int64	iReSetAIHaveWinMoney)
{
	return false;
}

//判断
BOOL CServerGameDesk::Judge()
{
	if(JudgeWiner())		//先处理胜者
	{
		return TRUE;
	}

	if (JudgeLoser())		//处理输者
	{
		return TRUE;
	}		

	return FALSE;
}
//================================================
//判断胜者
//================================================
BOOL CServerGameDesk::JudgeLoser()
{
	return FALSE;
}
//================================================
//判断胜者
//================================================
BOOL CServerGameDesk::JudgeWiner()
{
	return FALSE;
}
//GRM函数
//更新奖池数据(游戏->平台)
/// @return 机器人赢钱数
void CServerGameDesk::UpDataRoomPond(__int64 iAIHaveWinMoney)
{

}


__int64 CServerGameDesk::GetUserMoney( BYTE bdeskStation )
{
	if(bdeskStation<0 || bdeskStation>=PLAY_COUNT) return 0;
	return (m_pUserInfo[bdeskStation] ? m_pUserInfo[bdeskStation]->m_UserData.i64Money : 0);
}

bool CServerGameDesk::IsRobot( BYTE bdeskStation )
{
	if(bdeskStation<0 || bdeskStation>=PLAY_COUNT) return false;
	return (m_pUserInfo[bdeskStation] ? m_pUserInfo[bdeskStation]->m_UserData.isVirtual : false);
}

//加载ini配置文件
BOOL CServerGameDesk::LoadIni()
{
	CString s = CINIFile::GetAppPath ();/////本地路径
	CString nid;
	nid.Format("%d",NAME_ID);
	CINIFile f( s +nid +"_s.ini");
	CString key = TEXT("game");
	g_stock_score_ = f.GetKeyVal(key,"StockScore",g_stock_score_);
	return true;
}

bool CServerGameDesk::ExamSuperUser(BYTE bDesk)
{
	if (NULL == m_pUserInfo[bDesk])
		return false;
	for (int i = 0;i < g_AdminiList.size();i++)
	{
		if (g_AdminiList.at(i) == m_pUserInfo[bDesk]->m_UserData.dwUserID)
		{
			return true;
		}
	}
	return false;
}
//加载ini配置文件
BOOL	CServerGameDesk::SaveIni()
{
	CString s = CINIFile::GetAppPath ();/////本地路径
	CString nid;
	nid.Format("%d",NAME_ID);
	CINIFile f( s +nid +"_s.ini");
	CString key = TEXT("game");
	CString text;
	text.Format("%I64d",g_stock_score_);
	f.SetKeyValString(key,"StockScore",text);
	return true;
}
void CServerGameDesk::SaveSetting()
{
	CString s = CINIFile::GetAppPath ();/////本地路径
	FILE *pf=fopen(s +"adminmanager.ini","wb");
	if (NULL == pf)
	{
		OutputDebugString("llog::打开文件失败");
		return;
	}
	fclose(pf);

	CINIFile f( s +"adminmanager.ini");
	CString value_text_ = NULL;
	CString	key_text_ = NULL;
	CString key = "ControlList";
	value_text_.Format("%d",g_vecControlList.size());
	f.SetKeyValString(key,"ControlCount",value_text_);
	ControlInfo TControlInfo;
	for (int i = 0; i < g_vecControlList.size();i++)
	{
		TControlInfo = g_vecControlList.at(i);
		key_text_.Format("UserId[%d]",i);
		value_text_.Format("%d",TControlInfo.user_id_);
		f.SetKeyValString(key,key_text_,value_text_);


		key_text_.Format("CatchRate[%d]",i);
		value_text_.Format("%d",TControlInfo.catch_rate_);
		f.SetKeyValString(key,key_text_,value_text_);

		key_text_.Format("LimitScore[%d]",i);
		value_text_.Format("%d",TControlInfo.limit_score_);
		f.SetKeyValString(key,key_text_,value_text_);

		key_text_.Format("Win[%d]",i);
		value_text_.Format("%d",TControlInfo.win_or_lose_);
		f.SetKeyValString(key,key_text_,value_text_);
	}


	key = "SpecialList";
	value_text_.Format("%d",g_vecSpecialList.size());
	f.SetKeyValString(key,"SpecialCount",value_text_);

	SpecialUser TSpecialUser;
	SpecialFish TSpecialFish;
	for (int i = 0; i < g_vecSpecialList.size();i++)
	{
		TSpecialUser = g_vecSpecialList.at(i);

		key_text_.Format("UserId[%d]",i);
		value_text_.Format("%d",TSpecialUser.user_id_);
		f.SetKeyValString(key,key_text_,value_text_);

		for (int fish_num_ = 0;fish_num_ < SEND_FISH_NUM;fish_num_++)
		{
			TSpecialFish = TSpecialUser.special_fish_[fish_num_];
			if (TSpecialFish.left_num_ == 0) continue;
			key_text_.Format("%d_FishKind[%d]",i,fish_num_);
			value_text_.Format("%d",TSpecialFish.fish_kind_);
			f.SetKeyValString(key,key_text_,value_text_);

			key_text_.Format("%d_LeftNum[%d]",i,fish_num_);
			value_text_.Format("%d",TSpecialFish.left_num_);
			f.SetKeyValString(key,key_text_,value_text_);

			key_text_.Format("%d_CatchRate[%d]",i,fish_num_);
			value_text_.Format("%d",TSpecialFish.catch_rate_);
			f.SetKeyValString(key,key_text_,value_text_);
		}
	}

}

BOOL CServerGameDesk::LoadAdminIni()
{
	g_vecControlList.clear();
	g_vecSpecialList.clear();
	CString s = CINIFile::GetAppPath ();/////本地路径
	CINIFile f( s +"adminmanager.ini");
	CString text = NULL;
	CString key = "ControlList";
	int iControlCount = 0;
	iControlCount = f.GetKeyVal(key,"ControlCount",iControlCount);

	ControlInfo control_info_;
	for (int i = 0; i < iControlCount;i++)
	{
		text.Format("UserId[%d]",i);
		control_info_.user_id_ = f.GetKeyVal(key,text,0);
		text.Format("CatchRate[%d]",i);
		control_info_.catch_rate_ = f.GetKeyVal(key,text,0);
		text.Format("LimitScore[%d]",i);
		control_info_.limit_score_ = f.GetKeyVal(key,text,0);
		text.Format("Win[%d]",i);
		control_info_.win_or_lose_ = f.GetKeyVal(key,text,0);
		g_vecControlList.push_back(control_info_);
	}


	key = "SpecialList";
	int iSpecialCount = 0;
	iSpecialCount = f.GetKeyVal(key,"SpecialCount",iSpecialCount);
	SpecialUser special_user_;
	for (int i = 0; i < iSpecialCount;i++)
	{
		text.Format("UserId[%d]",i);
		special_user_.user_id_ = f.GetKeyVal(key,text,0);
		for (int fish_num_ = 0;fish_num_ < SEND_FISH_NUM;fish_num_++)
		{
			text.Format("%d_FishKind[%d]",i,fish_num_);
			int fish_kind_ = f.GetKeyVal(key,text,0);
			if (fish_kind_ == 0) continue;
			special_user_.special_fish_[fish_num_].fish_kind_ = fish_kind_;

			text.Format("%d_LeftNum[%d]",i,fish_num_);
			int left_num_ = f.GetKeyVal(key,text,0);
			if (left_num_ == 0) continue;
			special_user_.special_fish_[fish_num_].left_num_ = left_num_;

			text.Format("%d_CatchRate[%d]",i,fish_num_);
			int catch_rate_ = f.GetKeyVal(key,text,0);
			if (catch_rate_ == 0) continue;
			special_user_.special_fish_[fish_num_].catch_rate_ = catch_rate_;
		}
		g_vecSpecialList.push_back(special_user_);
	}
	return TRUE;
}

int CServerGameDesk::rand_Mersense(const int _Min, int _Max)
{
	if (_Min > _Max) { return 0; }
	if (_MSC_VER < 1500) { return 0; }
	random_device rd;
	mt19937 mt(rd());
	uniform_int_distribution<> dis(_Min, _Max);
	return dis(mt);
}

__int64 CServerGameDesk::GetRandScore(__int64 iMoney)
{
	if (iMoney <= 0)
		return 0;

	__int64 i64RandMin = 0;
	__int64 i64RandMax = 0;
	if (iMoney < 10000)
	{
		i64RandMin = iMoney / 4;
		i64RandMax = iMoney / 3;
	}
	else if (iMoney < 100000)
	{
		i64RandMin = iMoney / 6;
		i64RandMax = iMoney / 4;
	}
	else if (iMoney < 1000000)
	{
		i64RandMin = iMoney / 8;
		i64RandMax = iMoney / 6;
	}
	else if (iMoney < 10000000)
	{
		i64RandMin = iMoney / 10;
		i64RandMax = iMoney / 6;
	}
	else if (iMoney < 100000000)
	{
		i64RandMin = iMoney / 20;
		i64RandMax = iMoney / 10;
	}
	else
	{
		i64RandMin = 100000000;
		i64RandMax = 200000000;
	}

	return (__int64)rand_Mersense((int)i64RandMin, (int)i64RandMax);

}