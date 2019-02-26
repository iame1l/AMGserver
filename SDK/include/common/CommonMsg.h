#pragma once

#define WM_HALL_MSG_BEGIN					WM_USER * 2						//大厅消息开始索引

#define WM_HALL_INSTALL_START				WM_HALL_MSG_BEGIN+3			//整个安装任务开始，启动获取进度计时器
#define WM_HALL_INSTALL_RES					WM_HALL_MSG_BEGIN+4			//安装状态
#define WM_HALL_INSTALL_ING					WM_HALL_MSG_BEGIN+5			//正在安装
#define WM_HALL_INSTALL_FINISH				WM_HALL_MSG_BEGIN+6			//整个安装任务结束，关闭获取进度计时器

#define WM_HALL_INSTALL_PARSE_FAILED              WM_HALL_MSG_BEGIN+7         //解析文件出错