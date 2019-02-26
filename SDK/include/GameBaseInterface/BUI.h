#pragma once

#include "..\NewGameFrame\Client\UI_ForLogic\IUserInterface.h"

class BaseUI
{
private://以下为私有成员，不允许改为公有成员
	IUserInterface *m_pIUI;
	
	int m_iGameID;		//游戏ID

public:
	//构造函数
	BaseUI();
	virtual ~BaseUI();

public://以下是公有成员方法
	//【公共部分】（所有控件都用得上）------------------------------------------------------

	//设置指针m_pIUI 并获得游戏ID
	void SetpIUI(IUserInterface *pIUI, int iGameID);
	//获得指针m_pIUI
	IUserInterface* GetpIUI();
	//加载UI.data
	int Initial();
	//加载UI.data
	int Initial(TCHAR *szDataFileName);
	//显示/隐藏 控件
	BOOL ShowControl(const int iID, bool bFlag);
	//查询控件是否显示/隐藏
	BOOL GetControlShow(const int iID);
	//设置控件所在位置
	BOOL SetControlGameXY(const int iID, int ix, int iy);
	//获得控件所在位置
	BOOL GetControlGameXY(const int iID, int &ix, int &iy);
	//设置控件大小
	BOOL SetControlGameWH(const int iID, int iW, int iH);
	//获得控件大小
	BOOL GetControlGameWH(const int iID, int &iW, int &iH);

	//【按钮部分】---------------------------------------------------------------------------

	//设置按钮上的字
	void SetTextForBut(const int iID, int num);
	//设置按钮上的字
	void SetTextForBut(const int iID, __int64 num);
	//设置按钮上的字2
	void SetTextForBut(const int iID, TCHAR Text[]);
	//设置可用/禁用 按钮
	BOOL EnableControl(const int iID, bool bFlag);
	//查询按钮可用/禁用
	BOOL GetControlEnable(const int iID);
	//是否是按钮
	bool IsBut(const int iID);

	//【图片部分】---------------------------------------------------------------------------

	//设置图片控件加载制定的图片
	void SetImagesLoadPic(const int iID, TCHAR TcNickName[], bool bFlag);
	//设置图片是否穿透
	void SetImagesPenetrate(const int iID, bool bTouFlag);
	//设置仅显示图片部分区域
	void SetShowImageXY(const int iID, bool bFlag = true, int ix = 0, int iy = 0);
	//克隆图片
	void CloneImage(const int iID, int iGoalID);

	//【静态文本部分】-----------------------------------------------------------------------

	//设置文本控件显示内容(1)
	void SetTextInfo(const int iID, TCHAR TcNickName[], bool bFlag, FONTSHOWSTYLE showType = AlignmentLEFT);
	//设置文本控件显示内容(2)
	void SetTextInfo(const int iID, int iNum, bool bFlag, FONTSHOWSTYLE showType = AlignmentLEFT);
	//设置文本控件显示内容(3)
	void SetTextInfo(const int iID, __int64 i64Num, bool bFlag, FONTSHOWSTYLE showType = AlignmentLEFT);

	//得到文本控件的文字
	wchar_t* GetTextString(const int iID);
	//得到文本空间的数字
	int GetTextNum(const int iID);

	//【数字控件部分】-----------------------------------------------------------------------

	//设置各种数字控件显示
	void SetNumVisible(const int iID, __int64 byNum, bool bFlag);

	//【计时器控件部分】---------------------------------------------------------------------

	//显示一个计时器控件
	void SetClock(const int iID, int iTime, bool bFlag = true);
	//获得当前时间
	int GetCurrentTimeNum(const int iID);
	//【文本输入部分】-----------------------------------------------------------------------

	//获取文本输入框内容
	void GetTextEditInfo(const int iID, wchar_t wText[], int iCount);
	//设置文本输入框内容
	void SetTextEditInfo(const int iID, wchar_t wText[]);
	//设置文本框只接受数字
	void SetTextEditOnlyNum(const int iID);
	//设置文本框最小最大值
	void SetTextMin_Max(const int iID, int iMin, int iMax);

	//【牌控件部分】-------------------------------------------------------------------------
	
	//设置牌控件显示
	void SetCardInfo(const int iID, BYTE iCard[], int iNum, bool bFlag);
	//获取升起的扑克
	BYTE GetUpCard(const int iID, BYTE iCard[]);
	//设置升起的扑克
	void SetUpCard(const int iID, BYTE iCard[], int inum);

	//【筹码部分】---------------------------------------------------------------------------

	//显示筹码控件
	void SetCouMa(const int iID, int iNum, bool bFlag);

	//【麻将部分】---------------------------------------------------------------------------
	//[0]牌墙==================================================
    /**
     * @brief 以牌背填充牌墙
     * @param[in]        nCount  填几张    
     */
    void FillMJWallBack(const int iID, int nCount);  
    /**
     * @brief 设置牌墙上某张显示为什么牌
     * @param[in]       nIndex  从该座位的玩家看起，0表右边第一墩下面一张 1表右边第一墩上面一张 2表右边第二墩下面一张 。。。。。
     * @param[in]       nTile   -1为不显示
     */
    void SetMJWallTile(const int iID, int nTile, int nIndex);
    /**
    * @brief 设置视图方位
    */
    void SetMJViewDirection(const int iID, BYTE nViewStation);
    /**
    * @brief 得到方位
    */
    BYTE GetMJViewDirection(const int iID);
    /**
     * @brief 得到某张麻将子的绘制矩形
     * @param[in] nIndex  从该座位的玩家看起，0表右边第一墩下面一张 1表右边第一墩上面一张 2表右边第二墩下面一张 。。。。。
     */
    RECT GetMJTileRectByIndex(const int iID, int nIndex);
    /** @brief 设置牌墙最大长度
     *  @param[in]      nMaxLength
     */
    void SetMJMaxLength(const int iID, int nMaxLength);

	//[1]出牌======================================================
	/**
    * @brief 设置出牌
    */
    void SetMJGiveInfo(const int iID, int nHandTiles[], int nCount);
    /**
    * @brief 设置万能牌
    */
    void SetMJGodTile_Out(const int iID, int nGodTiles[], int nCount);    
    /**
    * @brief 设置视图方位
    */
    void SetMJViewDirection_Out(const int iID, BYTE nViewStation);
	/**
	* @brief 设置每行显示张数, 超过行数范围换到下一行显示
	*/
	void SetMJGiveLineCnt(const int iID, int nCount);

    /**
     * @brief 得到某张麻将子的绘制矩形
     */
    RECT GetMJTileRectByIndex_Out(const int iID, int nIndex);

    /**
    * @brief 得到手牌数据
    */
    void GetMJGiveInfo(const int iID, int nHandTiles[], int nCount);
    /**
    * @brief 得到方位
    */
    BYTE GetMJViewDirection_Out(const int iID);
	/**
	* @brief 获取每行显示张数
	*/
	int GetMJGiveLineCnt(const int iID);
	//[2]手牌==========================================================
	/**
    * @brief 设置手牌
    */
    void SetMJHandInfo(const int iID, int nHandTiles[], int nCount);
    /**
    * @brief 设置万能牌
    */
    void SetMJGodTile_Hand(const int iID, int nGodTiles[], int nCount);    
    /**
    * @brief 设置视图方位
    */
    void SetMJViewDirection_Hand(const int iID, BYTE nViewStation);
    /**
    * @brief 得到弹起的牌
    */
    int GetMJUpTile(const int iID);    
    /**
    * @brief 设置哪几张牌不能选
    */
    void SetMJInvalidTile(const int iID, int nInvalidTiles[], int nCount);
    /**
    * @brief 设置是否最右边一张分开
    */
    void SetMJGiveTileMode(const int iID, bool bIsGiving = false);
    /**
     * @brief 设置是否躺下
     */
    void SetMJIsLaydown(const int iID, bool bIsLaydown = false);
    /**
    * @brief 增加一组拦牌信息
    */
    void AddMJOneBlock(const int iID, MJ_LOGIC_TYPE type, BYTE ucTiles[4], BYTE ucChair);
    /**
    * @brief 增加一组特殊杠
    */
    void AddMJOneSpecQuad(const int iID, BYTE ucTiles[4], BYTE ucCount[4]);
    /**
    * @brief 清除所有拦牌显示
    */
    void ClearMJAllBlock(const int iID);

    /**
    * @brief 得到手牌数据
    */
    void GetMJHandInfo(const int iID, int nHandTiles[], int nCount);
    /**
    * @brief 得到方位
    */
    BYTE GetMJViewDirection_Hand(const int iID);
    /**
     * @brief 得到某张手牌的绘制矩形
     */
    RECT GetMJTileRectByIndex_Hand(const int iID, int nIndex);
    /** @brief 设置某张牌变成万能牌的颜色
     *  如果该索引上的牌是万能牌，则此接口对其无效
     */
    void SetMJGodColor(const int iID, int nIndex, bool bIs=true);

    /** @brief 设置锁定大小
    */
    void SetMJLockSize(const int iID, bool bLock);

    /** 
     * @brief 设置手动使用鼠标选择牌弹起, 此时鼠标移动效果无效
     * @param [in] bSelect 是否手动选择
     */
    void SetMJHandSelect(const int iID, bool bSelect);

    /**  
     * @brief 设置手动使用鼠标选择牌弹起, 此时鼠标移动效果无效
     * @param [in] nUpTiles 控件中弹起的牌数据
     * @param [in] nUpCount 弹起的牌数组长度
     * @return 返回实际的弹起牌的数量
     */
    int GetMJAllUpTile(const int iID, int nUpTiles[], int nUpCount);

	//【单选复选框控件】---------------------------------------------------------------------
	
	//设置是否被选中
	void SetIsSelect(const int iID, bool bFlag);
	//获取是否被选中
	bool GetIsSelect(const int iID);

	//【动画控件部分】(外部动画)-------------------------------------------------------------

	//设置指定动画显示指定的帧数
	void SetAnimateShowFram(const int iID, int iFramNum, bool bFlag);
	//设置外部动画的类型
	void SetAnimateCurveInstance(const int iID, CURVE_TYPE curType = CT_NONE);
	//设置播放外部动画 bLop-是否循环 iPlayCount-播放次数(可加载资源)
	void SetAnimatePlay(const int iID, TCHAR szAnimaPath[], bool bFlag, bool bLop = false, int iPlayCount = 1);
	//播放外部动画(单次播放)
	void PlayAnimation(const int iID, bool bFlag = true, bool bIsLoopFlag = false);

	//【动画控件部分】(MoveAction)-----------------------------------------------------------

	//梭哈控件移动动画
	void PlayChipInAnima(const int iMoveActionID, const  int StarPointID, const  int EndPointID, const  int iBeMoveID, int bView, int iNum, int iTime, bool bFlag);
	//梭哈控件移动动画(赢家获得输家筹码用)
	void PlayChipInAnimaEx(const int iMoveActionID, const  int StarPointID, const  int EndPointID, const  int iBeMoveID, int bView, int iNum, int iTime, bool bFlag);
	//发牌动画
	void PlayCardInAnima(const int iMoveActionID, const  int StarPointID, const  int EndPointID, const  int iBeMoveID, int bView, BYTE iCard[], int iNum, int iTime, bool bFlag);

	//【光标控件部分】-----------------------------------------------------------------------

	//设置光标
	void SetICursor(const wchar_t* pCurImg, bool bHide = true, int PointMode = 0, bool bFlag = true);
};

#include "BUI.inl"