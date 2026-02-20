

#ifndef __MSDEFINE_H__
#define __MSDEFINE_H__

// 注意：SERVER_KIND 枚举已在 ServerGameDefine.h 中定义，这里不需要重复定义
// 如果需要在 MonitoringServer 中使用 SERVER_KIND，直接使用 ServerGameDefine.h 中的定义即可

// 物品槽位位置常量定义（非日本版本缺少这些定义）
// 注意：这些常量在 CommonGameDefine.h 的 #ifdef _JAPAN_LOCAL_ 分支中定义
// 但在 #else 分支中被注释掉了，所以这里需要为 MonitoringServer 定义它们
#define TP_MUNPAWAREHOUSE_START		TP_PYOGUK_END
#define TP_MUNPAWAREHOUSE_END		(TP_MUNPAWAREHOUSE_START + 315)
#define TP_MUGONG_START				TP_MUNPAWAREHOUSE_END
#define TP_MUGONG_END				(TP_MUGONG_START + 25)

#endif