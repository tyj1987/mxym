#ifndef __HELPERFUNCTIONS_H__
#define __HELPERFUNCTIONS_H__

// Windows宏定义
#ifndef RGB
#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)((BYTE)(b)))<<16)))
#endif

// min/max宏（避免与std::min/max冲突）
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

// 项目自定义函数前向声明
extern char* SafeStrCpy(char* dest, const char* src, size_t destSize);
extern DWORD MakeNewKey(void);

// ASSERT宏定义（如果未定义）
#ifndef ASSERT
#ifdef _DEBUG
#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("ASSERT failed: %s at %s:%d\n", #condition, __FILE__, __LINE__); \
            __debugbreak(); \
        } \
    } while (0)
#else
#define ASSERT(condition) ((void)0)
#endif
#endif

#ifndef ASSERTMSG
#ifdef _DEBUG
#define ASSERTMSG(condition, msg) \
    do { \
        if (!(condition)) { \
            printf("ASSERT failed: %s - %s at %s:%d\n", #condition, msg, __FILE__, __LINE__); \
            __debugbreak(); \
        } \
    } while (0)
#else
#define ASSERTMSG(condition, msg) ((void)0)
#endif
#endif

// WRITEDEBUGFILE宏
#ifndef WRITEDEBUGFILE
#define WRITEDEBUGFILE(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif

#endif // __HELPERFUNCTIONS_H__
