#ifndef __BASE_FUNCTION_H__
#define __BASE_FUNCTION_H__

#ifdef OS_WINDOWS
#include <windows.h>
#endif

#include <time.h>
#include <stdio.h>
#include "Type.h"

#ifndef NULL
#define NULL 0
#endif

#ifndef SIZE_K
#define SIZE_K(x)  ((x) << 10)
#endif

#ifndef SIZE_M
#define SIZE_M(x)  ((SIZE_K(x)) << 10)
#endif

#ifndef SIZE_G
#define SIZE_G(x)  ((SIZE_M(x)) << 10)
#endif

#ifndef dim_of
#define dim_of(array) (sizeof(array) / sizeof(array[0]))
#endif

#ifndef IS_ZERO
#define IS_ZERO(a, max)   ((a) >= (max) ? (0) : (a))
#endif

#ifndef OS_WINDOWS
typedef int                     BOOL;
#define TRUE                    1
#define FALSE                   0

#ifndef MAX
#define MAX(a,b)                (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)                (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX_PATH
#define MAX_PATH                256    //PATH_MAX
#endif
#endif //OS_WINDOWS

#ifndef ARRAY_SIZE
# define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#define     ALIGN(value, align) (align?((( (value) + ( (align) - 1 ) ) / (align) ) * (align)):0 )
//#define     ALIGN2(d,a) (((d)+((a)-1))&~((a)-1)) //返回不小于d的对齐后的数(ALIGN的另一个表达式)
#define     ALIGN2(d,a) (((d)+((a)-1))-(((d)+((a)-1))%(a)))
//#define     ALIGN3(d,a) ((d)&~((a)-1)) //返回不大于d的对齐后的数
#define     ALIGN3(d,a) ((d)-((d)%(a)))

#ifndef WRITE_RAW_H264_TO_FILE
#define WRITE_RAW_H264_TO_FILE 0
#endif

void writereg(unsigned int regaddr,unsigned int regvalue);
unsigned int readreg(unsigned int regaddr);

int     ExecCommand(const char * cmd);
int     ExecCommand2(const char * cmd, const char * args, ...);
pid_t   ExecCommandBackground(const char * cmd, const char * args, ...);
int     XGetProgressIDByProgramName(const char * programName/*程序名*/, const char * args/*启动参数*/);
BOOL    XWaitProgressQuit(int progressid/*进程ID*/, int& status/*进程退出状态*/);
uint32	SystemGetMSCount();//相对时间
uint64  SystemGetUSCount();//相对时间
uint64  SystemGetUSCount(int abs_dummy);//绝对时间
uint32	SystemGetSecondCount();//获取1970-1-1 00:00:00至今经过的秒数
int		XSleep(uint32 ms);
void    SleepS(unsigned seconds);
void    SleepMs(unsigned long mSec);
BOOL    XSetSystemTime(int y, int mon, int d, int h, int m, int s);

BOOL    BlankChar(int ch);

BOOL	XFilePathExists(const char* path);//检测文件/路径是否存在
BOOL	XCreateFullPathDir(const char* dir,uint32 mode);//创建多重路径
BOOL	XGetDiskPartitionSpace(const char * driver,uint64 *totalspace,uint64 * freespace);//获取磁盘空间信息
BOOL    XRemoveDirectory(const char * dir);//移除空目录
BOOL    XRemoveDirectory(const char * dir, int dummy);//可移除非空目录
BOOL    XRemoveFullPathEmptyDir(const char * dir);//移除多级空目录
BOOL    XDeleteFile(const char* file);//删除文件
BOOL    XRenameFile(const char* old_full_path,const char* new_full_path);
BOOL    XGetFileSize(const char* filepath, size_t * size);
BOOL    XGetCurrentPath(char * path, int size);

#ifndef OS_WINDOWS
int     SafeRead(int fd, void *buf, size_t count);
int     SafeWrite(int fd, const uchar *buf, size_t count);
int     FullWrite(int fd, const uchar *buf, size_t len);
#endif

//
void CharToHex(char * dest, char * buffer , int len);

//
void SyncFS();//同步文件系统

//
uint32 crc32(uint32 crc, const uchar *p, uint32 len);
uint8 crc8(const uint8 *data, int len);
BOOL GetUUID(char *uuid,int buff_size);

void PrintTimeInterval(char *name, int count);
BOOL WaitNetWorkOK(char *svr_domain, int TimeOut_sec);

#ifdef USE_IO_REDIRECT
#define DBG(fmt, ...)  TRACE(fmt, ##__VA_ARGS__)
#define ERR(fmt, ...)  TRACE_ERR(fmt, ##__VA_ARGS__)
#else
//#define DBG(fmt, args...)   printf(fmt, ##args)
#define ERR(fmt, args...)   fprintf(stderr, "[%s:%s:%d]" fmt, __FILE__, __FUNCTION__, __LINE__, ##args)
#define TRACE(fmt, args...) printf("[%s:%s:%d]" fmt, __FILE__, __FUNCTION__, __LINE__, ##args)
#define PRINTF      TRACE
#define PRINTF_ERR  ERR
#define TRACE_ERR   ERR
#endif


#ifdef __GNUC__
#    define GCC_VERSION_AT_LEAST(x,y) (__GNUC__ > x || __GNUC__ == x && __GNUC_MINOR__ >= y)
#else
#    define GCC_VERSION_AT_LEAST(x,y) 0
#endif

#if GCC_VERSION_AT_LEAST(3, 1)
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define NO_INLINE __attribute__((noinline))
#else
#define ALWAYS_INLINE inline
#define NO_INLINE
#endif

#ifdef OS_WINDOWS
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

#ifdef OS_WINDOWS
#define I64PRNTFMT "%I64d"
#define U64PRNTFMT "%I64u"
#else
#define I64PRNTFMT "%lld"
#define U64PRNTFMT "%llu"
#endif

#define DBG_ORDER_CONSTRUCT(x)  fprintf(stderr,"[%s:%d] %s()\n" ,__FILE__,__LINE__,#x)
#define DBG_ORDER_DESTRUCT(x)   fprintf(stderr,"[%s:%d] ~%s()\n",__FILE__,__LINE__,#x)

void SetThreadName(const char *name);
#define SetTaskName SetThreadName

//单件模式定义
#define PATTERN_SINGLETON_DECLARE(classname)\
private:                                    \
    static classname * _instance;           \
    class CGarbo                            \
    {                                       \
    public:                                 \
        ~CGarbo()                           \
        {                                   \
            if (classname::_instance)       \
            {                               \
                delete classname::_instance;\
            }                               \
            classname::_instance = NULL;    \
        }                                   \
    };                                      \
public:                                     \
    friend class CGarbo;                    \
    static classname* instance();

#define PATTERN_SINGLETON_IMPLEMENT(classname)  \
classname * classname::_instance=new classname();\
classname * classname::instance()               \
{                                               \
    if(!_instance)                              \
    {                                           \
        _instance = new classname();            \
        if(_instance)                           \
        {                                       \
            static CGarbo _garbo;;              \
        }                                       \
    }                                           \
    return _instance;                           \
}


#define TEMPLATE_PATTERN_SINGLETON_DECLARE1(templatename,T1)\
private:                                                    \
    static templatename<T1> * _instance;                    \
    class CGarbo                                            \
    {                                                       \
    public:                                                 \
        ~CGarbo()                                           \
        {                                                   \
            if (templatename<T1>::_instance)                \
            {                                               \
                delete templatename<T1>::_instance;         \
            }                                               \
            templatename<T1>::_instance = NULL;             \
        }                                                   \
    };                                                      \
public:                                                     \
    static templatename<T1>* instance();

#define TEMPLATE_PATTERN_SINGLETON_IMPLEMENT1(templatename,T1)  \
template <class T1>                                             \
templatename<T1> * templatename<T1>::_instance=NULL;            \
template <class T1>                                             \
templatename<T1> * templatename<T1>::instance()                 \
{                                                               \
    if(!_instance)                                              \
    {                                                           \
        _instance = new templatename<T1>();                     \
        static CGarbo _garbo;                                   \
    }                                                           \
    return _instance;                                           \
}

template <class T>
class COriginalBufferArray
{
public:
    COriginalBufferArray(T * array, COriginalBufferArray* next)
    {
        m_BufferArray = array;
        m_Next = next;
    }
    ~COriginalBufferArray()
    {
        delete[] m_BufferArray;
        delete m_Next;
    }
private:
    T * m_BufferArray;
    COriginalBufferArray * m_Next;
};

template <class T>
class CCallbackInfo
{
public:
    CCallbackInfo():cb_fun(NULL), context(NULL){}
    virtual ~CCallbackInfo(){}
public:
    T    cb_fun;
    void *  context;
};

typedef struct tagMediaDataInfo
{
    unsigned long long  nTimestamp;//绝对时间us
    unsigned long long  nTimestamp2;//相对时间us
    unsigned int        nFrameSerial;//帧编号
    int                 emFrameType;//帧类型
    int                 emAVType;//包类型
    int                 emFrameCodec;//编码类型
    //unsigned int        nPicWidth;//图像宽度
    //unsigned int        nPicHeight;//图像高度
    //int                 nChannel;//编码通道号
    //int                 nFrameRate;//帧率

    //unsigned int        sample;//采样率
    //unsigned char       channel;//通道数
    //unsigned char       bit_width;//位宽
    //unsigned char       res[2];
    unsigned int     	duration;//ms

    unsigned int        nDataSize;//数据大小
    unsigned int        nDataSpace;//实际所占用空间(内存按字节对齐后的大小)
    unsigned char *     lpData;//数据
    struct tagMediaDataInfo *   lpNext;
} MediaDataInfo, *LPMediaDataInfo;

#define REC_TIME_BASE_YEAR (2000) //基础年份定义
#define SYS_TIME_BASE_YEAR REC_TIME_BASE_YEAR

typedef struct tagRECTIME
{
    uint32    second:6;   //0-59
    uint32    minute:6;   //0-59
    uint32    hour:5;     //0-23
    uint32    day:5;      //1-31
    uint32    month:4;    //1-12
    uint32    year:6;     //year-BASE_YRER(表示范围：BASE_YRER ～ BASE_YRER+63）
    ALWAYS_INLINE bool operator >(const tagRECTIME& other) const
    {
        return (*(uint32*)this) > (*(uint32*)&other);
    }
    ALWAYS_INLINE bool operator >(uint32 other) const
    {
        return (*(uint32*)this) > other;
    }
    ALWAYS_INLINE bool operator <(const tagRECTIME& other) const
    {
        return (*(uint32*)this) < (*(uint32*)&other);
    }
    ALWAYS_INLINE bool operator <(uint32 other) const
    {
        return (*(uint32*)this) < other;
    }
    ALWAYS_INLINE bool operator >=(const tagRECTIME& other) const
    {
        return (*(uint32*)this) >= (*(uint32*)&other);
    }
    ALWAYS_INLINE bool operator >=(uint32 other) const
    {
        return (*(uint32*)this) >= other;
    }
    ALWAYS_INLINE bool operator <=(const tagRECTIME& other) const
    {
        return (*(uint32*)this) <= (*(uint32*)&other);
    }
    ALWAYS_INLINE bool operator <=(uint32 other) const
    {
        return (*(uint32*)this) <= other;
    }
    ALWAYS_INLINE bool operator ==(const tagRECTIME& other) const
    {
        return (*(uint32*)this) == (*(uint32*)&other);
    }
    ALWAYS_INLINE bool operator ==(uint32 other) const
    {
        return (*(uint32*)this) == other;
    }
    ALWAYS_INLINE bool operator !=(const tagRECTIME& other) const
    {
        return (*(uint32*)this) != (*(uint32*)&other);
    }
    ALWAYS_INLINE bool operator !=(uint32 other) const
    {
        return (*(uint32*)this) != other;
    }
    ALWAYS_INLINE tagRECTIME& operator=(const uint32 other)
    {
        (*(uint32*)this) = other;
        return *this;
    }
    ALWAYS_INLINE tagRECTIME& operator=(const tagRECTIME& other)
    {
        (*(uint32*)this) = (*(uint32*)&other);
        return *this;
    }
    ALWAYS_INLINE tagRECTIME()
    {
        *(uint32*)this = 0x420000;
    }
    ALWAYS_INLINE tagRECTIME(const tagRECTIME& other)
    {
        *(uint32*)this = (*(uint32*)&other);
    }
    ALWAYS_INLINE tagRECTIME(uint32 other)
    {
        *(uint32*)this = other;
    }
} RECTIME;

void Second2RECTIME(time_t second/*1970/1/1 00:00:00距今秒数*/,RECTIME& rectime);
void RECTIME2Second(const RECTIME& rectime,time_t& second);


#endif //__function_h__

