// NumberFormat.cpp: 数字格式化工具函数
//

#include "stdafx.h"
#include "../[CC]Header/CommonStruct.h"

// AddComma - 在数字字符串中添加逗号分隔符
void AddComma(char* str)
{
    if(!str || !*str) return;

    char temp[256];
    ZeroMemory(temp, sizeof(temp));

    int len = strlen(str);
    int commaPos = 0;
    int destPos = 0;

    // 从右向左处理，每3位添加一个逗号
    for(int i = len - 1; i >= 0; i--)
    {
        if(str[i] >= '0' && str[i] <= '9')
        {
            temp[destPos++] = str[i];
            commaPos++;
            if(commaPos == 3 && i > 0)
            {
                temp[destPos++] = ',';
                commaPos = 0;
            }
        }
        else
        {
            temp[destPos++] = str[i];
        }
    }

    // 反转字符串
    for(int i = 0; i < destPos; i++)
    {
        str[i] = temp[destPos - 1 - i];
    }
    str[destPos] = '\0';
}

// AddComma - 将数字转换为带逗号的字符串
char* AddComma(unsigned long num)
{
    static char buffer[256];
    sprintf(buffer, "%lu", num);
    AddComma(buffer);
    return buffer;
}

// RemoveComma - 从数字字符串中移除逗号
char* RemoveComma(char* str)
{
    if(!str || !*str) return str;

    static char buffer[256];
    int destPos = 0;

    for(int i = 0; str[i]; i++)
    {
        if(str[i] != ',')
        {
            buffer[destPos++] = str[i];
        }
    }
    buffer[destPos] = '\0';

    // 复制回原字符串
    strcpy(str, buffer);

    return str;
}

// GetCurTimeToString - 将 stTIME 结构转换为字符串
char* GetCurTimeToString(stTIME* pTime)
{
    if(!pTime) return "";

    static char szTime[32] = "";
    _snprintf_s(szTime, sizeof(szTime), "%04d-%02d-%02d %02d:%02d:%02d",
            pTime->GetYear(), pTime->GetMonth(), pTime->GetDay(),
            pTime->GetHour(), pTime->GetMinute(), pTime->GetSecond());
    return szTime;
}
