#ifndef __TYPE_H__
#define __TYPE_H__

typedef unsigned char       uchar;
typedef unsigned char       byte;
typedef unsigned char       uint8;
typedef short               int16;
typedef unsigned short      uint16;
typedef int                 int32;
typedef unsigned int        uint32;
//typedef long                int64;
//typedef unsigned long       uint64;
typedef unsigned long       ulong;
typedef int                 BOOL;
typedef long long int64;
typedef unsigned long long uint64;
#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

typedef enum emRETURN_TYPE
{
    RETURN_OK = 0, //成功
    RETURN_ERR_UNKNOWN = -1, //未知错误
    RETURN_ERR_OOM = -2, // out of memory <内存分配失败>
    RETURN_ERR_OOR = -3, // out of range <参数超出有效范围>
    RETURN_ERR_OOB = -4, // out of buffer <缓冲区不足>
    RETURN_ERR_ARG = -5, // invalid argument
    RETURN_ERR_TIMEOUT = -6, // time out
    RETURN_ERR_OPEN_FILE = -7, //打开文件失败
    RETURN_ERR_CREATE_THREAD = -8, //创建线程失败
    RETURN_ERR_DENY = -9, //操作不允许
    RETURN_ERR_UNSUPPORT = -10, //不支持
    RETURN_ERR_CREATE_MSGQ = -11, //创建消息队列失败
    RETURN_ERR_MSG_SEND    = -12, //消息发送错误
    RETURN_ERR_JSON_FORMAT = -13, //Json格式错误
} RETURN_TYPE_E;

#if 0
//单例模式定义
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
classname * classname::_instance=NULL;          \
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
#endif
#endif