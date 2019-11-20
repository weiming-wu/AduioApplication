#include "BaseFunction.h"

#ifdef OS_WINDOWS

#include <io.h>
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#include <imagehlp.h>
#pragma comment(lib,"imagehlp.lib")

#else //!OS_WINDOWS
#include <netdb.h>
#include <arpa/inet.h>

#include <string>
#include <vector>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/vfs.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <endian.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#ifdef HAS_HI_MEM_H
#include "hi_mem.h"
#else
#include <string.h>
#endif

#ifdef OS_ARM_LINUX
#include <sys/syscall.h>
#endif //OS_ARM_LINUX

#endif //!OS_WINDOWS

void CharToHex(char * dest, char * buffer , int len)
{
 int i=0;
 int j=0;
 unsigned char temp;
 while(i<len)
 {
  temp=buffer[i];
  if((temp>=0x30)&&(temp<=0x39))
  {
   temp=temp-0x30;
   dest[j]=temp<<4;
  }
  else if((temp>=0x41)&&(temp<=0x46))
  {
   temp=temp-0x41+0x0A;
   dest[j]=temp<<4;
  }
  else if((temp>=0x61)&&(temp<=0x66))
  {
   temp=temp-0x61+0x0A;
   dest[j]=temp<<4;
  }
  else
  {
   dest[j]=0x00;
  }
//  dest[j]=dest[j]<<4;
  temp=buffer[i+1];
  if((temp>=0x30)&&(temp<=0x39))
  {
   temp=temp-0x30;
   dest[j]=dest[j]|temp;
  }
  else if((temp>=0x41)&&(temp<=0x46))
  {
   temp=temp-0x41+0x0A;
   dest[j]=dest[j]|temp;
  }
  else if((temp>=0x61)&&(temp<=0x66))
  {
   temp=temp-0x61+0x0A;
   dest[j]=dest[j]|temp;
  }
  else
  {
   dest[j]=dest[j]|0x00;
  }
  i=i+2;
  j=j+1;
 }
 return;
}

/*执行外部命令，简单版，后续升级*/
int ExecCommand(const char * cmd)
{
    char tmp[512];
	FILE *pp = popen(cmd, "r");

	//printf("---------------->execute cmd: %s\n", cmd);
    if (!pp)
    {
		ERR("popen exec date command\n");
		return -1;
	}
    while (fgets(tmp, sizeof(tmp), pp) != NULL)
    {
		tmp[strlen(tmp) - 1] = '\0';
		printf("%s\n", tmp);
	}
    pclose(pp);
    return 0;
}

int ExecCommand2(const char * cmd, const char * args, ...)
{
#if 0
    int status = 0;
    char * arglist[256]={NULL};
    int argc = 0;
    pid_t child;
    char *arg;

    va_list ap;
    va_start(ap, args);
    for (const char *p = cmd+strlen(cmd)-1; p>cmd; p--)
    {
        if (*p == PATH_SEPARATOR)
        {
            arglist[argc++] = (char*)(p+1);
            break;
        }
    }
    if (0 == argc)
    {
        arglist[argc++] = (char*)cmd;
    }
    arglist[argc++] = (char*)args;
    while (arg=va_arg(ap, char *))
    {
        arglist[argc++] = arg;
        if (argc >= 254)
            break;
    }
    va_end(ap);
    arglist[argc] = NULL;

    child = vfork();
    if (child < 0)
    {
        ERR("vfork error\n");
        return child;
    }
    if (child == 0)
    {
	    int ret = -1;
	    struct rlimit rl;

        if (getrlimit(RLIMIT_NOFILE, &rl) == 0)
        {
            if(rl.rlim_max == RLIM_INFINITY)
            {
                rl.rlim_max = 1024;
            }
            for(int i = 0; i < rl.rlim_max; i++)
            {
                close(i);
            }
        }

        printf("%s", cmd);
        for (int i; arglist[i]; i++)
            printf(" %s", arglist[i]);
        printf("\n");

        ret = execvp(cmd, arglist);

        if (0 > ret)
		{
			ERR("execvp error.\n");
		}

		_exit(ret);
    }
    else if (child > 0)
    {
        while (0 > waitpid(child, &status, 0/* WUNTRACED | WCONTINUED*/))
        {
            if(errno != EINTR)
            {
                status = -1;
                break;
            }
        }
        printf("------>status=%d\n", status);
    }

    return status;
#else
    char cmdbuf[256] = {0};
    char *arg = NULL;
    char *p = cmdbuf;
    int status;
    pid_t child;

    p += sprintf(p, "%s", cmd);
    if (args)
    {
        va_list ap;
        va_start(ap, args);
        p += sprintf(p, " %s", args);
        while (arg = va_arg(ap, char *))
        {
            p+= sprintf(p, " %s", arg);
        }
        va_end(ap);
    }

    child = vfork();
    if (child < 0)
    {
        ERR("vfork error: %m\n");
        return child;
    }
    if (child == 0)
    {
        int ret = -1;
        struct rlimit rl;

        if (getrlimit(RLIMIT_NOFILE, &rl) == 0)
        {
            if(rl.rlim_max == RLIM_INFINITY)
            {
                rl.rlim_max = 1024;
            }
            for(int i = 3; i < (int)rl.rlim_max; i++)
            {
                close(i);
            }
        }

        ret = execl("/bin/sh", "sh", "-c", cmdbuf, NULL);

        if (0 > ret)
        {
            ERR("execl error. %m\n");
        }

        _exit(127);
    }
    else if (child > 0)
    {
        while (0 > waitpid(child, &status, 0/* WUNTRACED | WCONTINUED*/))
        {
            if (errno != EINTR)
            {
                status = -1;
                break;
            }
        }
        printf("--->status=%d\n", status);
    }

    //return status;
    return 0;
#endif
}

pid_t ExecCommandBackground(const char * cmd, const char * args, ...)
{
    //int status = 0;
    char * arglist[256]={NULL};
    int argc = 0;
    pid_t child;
    char *arg;

    va_list ap;
    va_start(ap, args);
    arglist[argc++] = (char*)cmd;
    arglist[argc++] = (char*)cmd;
    arglist[argc++] = (char*)args;
    while (arg=va_arg(ap, char *))
    {
        arglist[argc++] = arg;
        if (argc >= 254)
            break;
    }
    va_end(ap);
    arglist[argc] = NULL;


    child = vfork();

    if (child == 0)
    {
        for(int i=0;i< NOFILE;++i)//关闭打开的文件描述符
            close(i);
        umask(0);//重设文件创建掩模
        execvp(cmd, arglist);
		ERR("execvp error.\n");
		_exit(127);
    }
    else if (child > 0)
    {
        //int pstat;
        //int ret;
        printf("--------->pid = %d\n", child);
	    //do {
		//    ret = waitpid(child, &pstat, 0);
	    //} while (ret == -1 && errno == EINTR);
    }

    return child;
}

int XGetProgressIDByProgramName(const char * program, const char * args)
{
#ifdef OS_WINDOWS
    #error please implement XGetProgressIDByProgramName()
#else
    char *pid=NULL,*name=NULL,*arg=NULL;
    char buff[256]="ps -o pid,comm,args";/*具体命令格式需要根据不同系统进行修改,
                                         本习系统中,使用该命令时,第一列为进程ID,第二列为进程名,如:
                                              960 jffs2_gcd_mtd5   [jffs2_gcd_mtd5]
                                             1150 udhcpc           udhcpc -i eth0
                                             1152 telnetd          telnetd
                                             1162 RTW_CMD_THREAD   [RTW_CMD_THREAD]
                                             1168 sh               -sh
                                             1184 wpa_supplicant   /root/wifi/wpa_supplicant -B -Dwext -iwlan0 -c/root/wifi/wpa_supplicant.conf
                                             1233 sh               -sh
                                       */
    FILE *pp = popen(buff, "r");
    if (!pp)
    {
	    ERR("popen exec date command\n");
	    return FALSE;
    }
    while (fgets(buff, sizeof(buff), pp) != NULL)
    {
        for(pid = buff; *pid && BlankChar(*pid); pid++);
        if (!*pid)
        {
            pclose(pp);
            return -1;
        }
        for(name = pid+1; *name && !BlankChar(*name); name++);
        if (!*name)
        {
            pclose(pp);
            return -1;
        }
        *name='\0';
        name++;
        for(; *name && BlankChar(*name); name++);
        if (!*name)
        {
            pclose(pp);
            return -1;
        }
        for(arg = name+1; *arg && !BlankChar(*arg); arg++);
        if (!*arg)
        {
            pclose(pp);
            return -1;
        }
        *arg='\0';
        arg++;
        for(; *arg && BlankChar(*arg); arg++);
        if (!*arg)
        {
            pclose(pp);
            return -1;
        }
        if (!strcmp(name, program))
        {
            printf("->%s %s %s\n", pid, name, arg);
            if (args)
            {
                if (strstr(arg, args))
                {
                    return atoi(pid);
                }
            }
            else
            {
                return atoi(pid);
            }
        }
    }
    pclose(pp);
    return -1;
#endif
}

BOOL XWaitProgressQuit(int progressid/*进程ID*/, int& status/*进程退出状态*/)
{
#ifdef OS_WINDOWS
    #error please implement function XGetProgressIDByProgramName()
#else
    pid_t w;
	do {
		w = waitpid(progressid, &status, WUNTRACED | WCONTINUED);
		if (w == -1)
		{
			perror("waitpid");
			return FALSE;
		}

		if (WIFEXITED(status))
		{
			TRACE("exited, status=%d\n", WEXITSTATUS(status));
		}
		else if (WIFSIGNALED(status))
		{
			TRACE("killed by signal %d\n", WTERMSIG(status));
		}
		else if (WIFSTOPPED(status))
		{
			TRACE("stopped by signal %d\n", WSTOPSIG(status));
		}
		else if (WIFCONTINUED(status))
		{
			TRACE("continued\n");
		}
	} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	return TRUE;
#endif
}

uint SystemGetMSCount()
{
#ifdef OS_WINDOWS
    return ::GetTickCount();
#else

#if 0
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
#else
    struct timespec ts = {0, 0};
    if (clock_gettime(CLOCK_MONOTONIC, &ts))
        return 0;
    return ts.tv_sec * 1000 + ts.tv_nsec/1000000;
#endif

#endif
}

uint64  SystemGetUSCount()
{
#ifdef OS_WINDOWS
#error please implement SystemGetUSCount()
#else
    //返回相对时间
    struct timespec ts = {0, 0};
    if (clock_gettime(CLOCK_MONOTONIC, &ts))
        return 0ULL;
    return (uint64)ts.tv_sec * 1000000ULL + ts.tv_nsec/1000ULL;
    //struct timeval tv;
    //gettimeofday(&tv,NULL);
    //return (uint64)(tv.tv_sec*1000000ULL + tv.tv_usec);
#endif
}

uint64 SystemGetUSCount(int abs_dummy)
{
#ifdef OS_WINDOWS
#error please implement SystemGetUSCount()
#else
    //返回绝对时间
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (uint64)(tv.tv_sec*1000000ULL + tv.tv_usec);
#endif
}

uint SystemGetSecondCount()
{
#ifdef OS_WINDOWS
    return (uint)time(NULL);
#else
    struct timespec ts = {0, 0};
    if (clock_gettime(CLOCK_REALTIME, &ts))
        return 0;
    return ts.tv_sec;
#endif
}

int XSleep(uint32 ms)
{
#ifdef OS_WINDOWS
	Sleep(ms);
	return 0;
#else
	return usleep(ms*1000);
#endif
}

void SleepS(unsigned seconds)
{
    struct timeval tv;
    tv.tv_sec=seconds;
    tv.tv_usec=0;
    int err;
    do
    {
       err=select(0,NULL,NULL,NULL,&tv);
    }while(err<0 && errno==EINTR);
}

void SleepMs(unsigned long mSec)
{
    struct timeval tv;
    tv.tv_sec=mSec/1000;
    tv.tv_usec=(mSec%1000)*1000;
    int err;
    do
    {
       err=select(0,NULL,NULL,NULL,&tv);
    }while(err<0 && errno==EINTR);
}

BOOL XSetSystemTime(int y, int mon, int d, int h, int m, int s)
{
#ifdef OS_WINDOWS
#error please implement this function for windows
#else
	time_t now;
    struct tm tm;

    tm.tm_sec = s;
    tm.tm_min = m;
    tm.tm_hour = h;
    tm.tm_mday = d;
    tm.tm_mon = mon - 1;
    tm.tm_year = y - 1900;

    TRACE("SET SYS TIME:%04d/%02d/%02d %02d:%02d:%02d \n", y, mon, d, h, m, s);
    if ((now = mktime(&tm)) < 0) {
        ERR("mktime() failed\n");
        return FALSE;
    }
    stime(&now);
    return 0;
#endif
}

BOOL    BlankChar(int ch)
{
    return ch==' '||ch=='\r'||ch=='\t'||ch=='\b'||ch=='\n';
}

#define uswap_32(x) \
	((((x) & 0xff000000) >> 24) | \
	 (((x) & 0x00ff0000) >>  8) | \
	 (((x) & 0x0000ff00) <<  8) | \
	 (((x) & 0x000000ff) << 24))

#if __BYTE_ORDER == __LITTLE_ENDIAN
# define cpu_to_le32(x)		(x)
# define le32_to_cpu(x)		(x)
# define DO_CRC(x) crc = tab[(crc ^ (x)) & 255] ^ (crc >> 8)
#else
# define cpu_to_le32(x)		uswap_32(x)
# define le32_to_cpu(x)		uswap_32(x)
# define DO_CRC(x) crc = tab[((crc >> 24) ^ (x)) & 255] ^ (crc << 8)
#endif

#define tole(x) cpu_to_le32(x)

static const uint32 crc_table[256] = {
	tole(0x00000000L), tole(0x77073096L), tole(0xee0e612cL), tole(0x990951baL), tole(0x076dc419L), tole(0x706af48fL), tole(0xe963a535L), tole(0x9e6495a3L),
	tole(0x0edb8832L), tole(0x79dcb8a4L), tole(0xe0d5e91eL), tole(0x97d2d988L), tole(0x09b64c2bL), tole(0x7eb17cbdL), tole(0xe7b82d07L), tole(0x90bf1d91L),
	tole(0x1db71064L), tole(0x6ab020f2L), tole(0xf3b97148L), tole(0x84be41deL), tole(0x1adad47dL), tole(0x6ddde4ebL), tole(0xf4d4b551L), tole(0x83d385c7L),
	tole(0x136c9856L), tole(0x646ba8c0L), tole(0xfd62f97aL), tole(0x8a65c9ecL), tole(0x14015c4fL), tole(0x63066cd9L), tole(0xfa0f3d63L), tole(0x8d080df5L),
	tole(0x3b6e20c8L), tole(0x4c69105eL), tole(0xd56041e4L), tole(0xa2677172L), tole(0x3c03e4d1L), tole(0x4b04d447L), tole(0xd20d85fdL), tole(0xa50ab56bL),
	tole(0x35b5a8faL), tole(0x42b2986cL), tole(0xdbbbc9d6L), tole(0xacbcf940L), tole(0x32d86ce3L), tole(0x45df5c75L), tole(0xdcd60dcfL), tole(0xabd13d59L),
	tole(0x26d930acL), tole(0x51de003aL), tole(0xc8d75180L), tole(0xbfd06116L), tole(0x21b4f4b5L), tole(0x56b3c423L), tole(0xcfba9599L), tole(0xb8bda50fL),
	tole(0x2802b89eL), tole(0x5f058808L), tole(0xc60cd9b2L), tole(0xb10be924L), tole(0x2f6f7c87L), tole(0x58684c11L), tole(0xc1611dabL), tole(0xb6662d3dL),
	tole(0x76dc4190L), tole(0x01db7106L), tole(0x98d220bcL), tole(0xefd5102aL), tole(0x71b18589L), tole(0x06b6b51fL), tole(0x9fbfe4a5L), tole(0xe8b8d433L),
	tole(0x7807c9a2L), tole(0x0f00f934L), tole(0x9609a88eL), tole(0xe10e9818L), tole(0x7f6a0dbbL), tole(0x086d3d2dL), tole(0x91646c97L), tole(0xe6635c01L),
	tole(0x6b6b51f4L), tole(0x1c6c6162L), tole(0x856530d8L), tole(0xf262004eL), tole(0x6c0695edL), tole(0x1b01a57bL), tole(0x8208f4c1L), tole(0xf50fc457L),
	tole(0x65b0d9c6L), tole(0x12b7e950L), tole(0x8bbeb8eaL), tole(0xfcb9887cL), tole(0x62dd1ddfL), tole(0x15da2d49L), tole(0x8cd37cf3L), tole(0xfbd44c65L),
	tole(0x4db26158L), tole(0x3ab551ceL), tole(0xa3bc0074L), tole(0xd4bb30e2L), tole(0x4adfa541L), tole(0x3dd895d7L), tole(0xa4d1c46dL), tole(0xd3d6f4fbL),
	tole(0x4369e96aL), tole(0x346ed9fcL), tole(0xad678846L), tole(0xda60b8d0L), tole(0x44042d73L), tole(0x33031de5L), tole(0xaa0a4c5fL), tole(0xdd0d7cc9L),
	tole(0x5005713cL), tole(0x270241aaL), tole(0xbe0b1010L), tole(0xc90c2086L), tole(0x5768b525L), tole(0x206f85b3L), tole(0xb966d409L), tole(0xce61e49fL),
	tole(0x5edef90eL), tole(0x29d9c998L), tole(0xb0d09822L), tole(0xc7d7a8b4L), tole(0x59b33d17L), tole(0x2eb40d81L), tole(0xb7bd5c3bL), tole(0xc0ba6cadL),
	tole(0xedb88320L), tole(0x9abfb3b6L), tole(0x03b6e20cL), tole(0x74b1d29aL), tole(0xead54739L), tole(0x9dd277afL), tole(0x04db2615L), tole(0x73dc1683L),
	tole(0xe3630b12L), tole(0x94643b84L), tole(0x0d6d6a3eL), tole(0x7a6a5aa8L), tole(0xe40ecf0bL), tole(0x9309ff9dL), tole(0x0a00ae27L), tole(0x7d079eb1L),
	tole(0xf00f9344L), tole(0x8708a3d2L), tole(0x1e01f268L), tole(0x6906c2feL), tole(0xf762575dL), tole(0x806567cbL), tole(0x196c3671L), tole(0x6e6b06e7L),
	tole(0xfed41b76L), tole(0x89d32be0L), tole(0x10da7a5aL), tole(0x67dd4accL), tole(0xf9b9df6fL), tole(0x8ebeeff9L), tole(0x17b7be43L), tole(0x60b08ed5L),
	tole(0xd6d6a3e8L), tole(0xa1d1937eL), tole(0x38d8c2c4L), tole(0x4fdff252L), tole(0xd1bb67f1L), tole(0xa6bc5767L), tole(0x3fb506ddL), tole(0x48b2364bL),
	tole(0xd80d2bdaL), tole(0xaf0a1b4cL), tole(0x36034af6L), tole(0x41047a60L), tole(0xdf60efc3L), tole(0xa867df55L), tole(0x316e8eefL), tole(0x4669be79L),
	tole(0xcb61b38cL), tole(0xbc66831aL), tole(0x256fd2a0L), tole(0x5268e236L), tole(0xcc0c7795L), tole(0xbb0b4703L), tole(0x220216b9L), tole(0x5505262fL),
	tole(0xc5ba3bbeL), tole(0xb2bd0b28L), tole(0x2bb45a92L), tole(0x5cb36a04L), tole(0xc2d7ffa7L), tole(0xb5d0cf31L), tole(0x2cd99e8bL), tole(0x5bdeae1dL),
	tole(0x9b64c2b0L), tole(0xec63f226L), tole(0x756aa39cL), tole(0x026d930aL), tole(0x9c0906a9L), tole(0xeb0e363fL), tole(0x72076785L), tole(0x05005713L),
	tole(0x95bf4a82L), tole(0xe2b87a14L), tole(0x7bb12baeL), tole(0x0cb61b38L), tole(0x92d28e9bL), tole(0xe5d5be0dL), tole(0x7cdcefb7L), tole(0x0bdbdf21L),
	tole(0x86d3d2d4L), tole(0xf1d4e242L), tole(0x68ddb3f8L), tole(0x1fda836eL), tole(0x81be16cdL), tole(0xf6b9265bL), tole(0x6fb077e1L), tole(0x18b74777L),
	tole(0x88085ae6L), tole(0xff0f6a70L), tole(0x66063bcaL), tole(0x11010b5cL), tole(0x8f659effL), tole(0xf862ae69L), tole(0x616bffd3L), tole(0x166ccf45L),
	tole(0xa00ae278L), tole(0xd70dd2eeL), tole(0x4e048354L), tole(0x3903b3c2L), tole(0xa7672661L), tole(0xd06016f7L), tole(0x4969474dL), tole(0x3e6e77dbL),
	tole(0xaed16a4aL), tole(0xd9d65adcL), tole(0x40df0b66L), tole(0x37d83bf0L), tole(0xa9bcae53L), tole(0xdebb9ec5L), tole(0x47b2cf7fL), tole(0x30b5ffe9L),
	tole(0xbdbdf21cL), tole(0xcabac28aL), tole(0x53b39330L), tole(0x24b4a3a6L), tole(0xbad03605L), tole(0xcdd70693L), tole(0x54de5729L), tole(0x23d967bfL),
	tole(0xb3667a2eL), tole(0xc4614ab8L), tole(0x5d681b02L), tole(0x2a6f2b94L), tole(0xb40bbe37L), tole(0xc30c8ea1L), tole(0x5a05df1bL), tole(0x2d02ef8dL)
};

uint32 crc32_no_comp(uint32 crc, const uchar *buf, uint len)
{
	const uint32 *tab = crc_table;
	const uint32 *b =(const uint32 *)buf;
	size_t rem_len;

	crc = cpu_to_le32(crc);
	/* Align it */
	if (((long)b) & 3 && len)
	{
		uchar *p = (uchar *)b;
		do {
			DO_CRC(*p++);
		} while ((--len) && ((long)p)&3);
		b = (uint32 *)p;
	}

	rem_len = len & 3;
	len = len >> 2;
	for (--b; len; --len)
	{
		/* load data 32 bits wide, xor data 32 bits wide. */
		crc ^= *++b; /* use pre increment for speed */
		DO_CRC(0);
		DO_CRC(0);
		DO_CRC(0);
		DO_CRC(0);
	}
	len = rem_len;
	/* And the last few bytes */
	if (len)
	{
		uchar *p = (uchar *)(b + 1) - 1;
		do {
			DO_CRC(*++p); /* use pre increment for speed */
		} while (--len);
	}

	return le32_to_cpu(crc);
}

uint32 crc32(uint32 crc, const uchar *p, uint len)
{
    //printf("BYTE_ORDER =%d  __LITTLE_ENDIAN=%d\n",BYTE_ORDER , __LITTLE_ENDIAN);
    return crc32_no_comp(crc ^ 0xffffffffL, p, len) ^ 0xffffffffL;
}

uint8 calcrc_1byte(uint8 abyte)
{
    uint8 i, crc_1byte;
    crc_1byte = 0;
    for (i = 0; i < 8; i++)
    {
        if (((crc_1byte^abyte) & 0x01))
        {
            crc_1byte ^= 0x18;
            crc_1byte >>= 1;
            crc_1byte |= 0x80;
        }
        else
        {
            crc_1byte >>= 1;
        }
        abyte >>= 1;
    }
    return crc_1byte;
}

uint8 crc8(const uint8 *data, int len)
{
    uint8 crc = 0;
    while (len--)
    {
        crc = calcrc_1byte(crc^*data++);
    }
    return crc;
}

BOOL GetUUID(char *uuid,int buff_size)
{
#ifdef OS_WINDOWS
#error please implement GetUUID() function
#else
    int fd = open ("/proc/sys/kernel/random/uuid", O_RDONLY);
    if (fd<0)
        return FALSE;
    do {
        char buff[128];
        struct stat stat_buf;
        if (fstat (fd, &stat_buf) < 0)
        {
            break;
        }
        memset(buff,0,sizeof(buff));
        if (read (fd, buff, sizeof(buff)-1) <= 0)
        {
            break;
        }
        if ((int)strlen(buff)>= buff_size)
        {
            break;
        }

        //过滤非可见字符
        char *p1 = buff;
        for (char * p2=buff;*p2;p2++)
        {
            if (!isspace(*p2))
            {
                *p1++ = *p2;
            }
        }
        *p1 = '\0';

        strcpy(uuid,buff);

        close(fd);
        return TRUE;
    } while(0);
    close(fd);
    return FALSE;
#endif
}

void Second2RECTIME(time_t second/*1970/1/1 00:00:00距今秒数*/,RECTIME& rectime)
{
    struct tm  tm_t;
    struct tm  *t = gmtime_r((const time_t*)&second,&tm_t);
    rectime.year    = t->tm_year + 1900 - REC_TIME_BASE_YEAR;
    rectime.month   = t->tm_mon + 1;
    rectime.day     = t->tm_mday;
    rectime.hour    = t->tm_hour;
    rectime.minute  = t->tm_min;
    rectime.second  = t->tm_sec;

    //DBG("TIME(%04d%04d%02d%02d %02d%02d%02d)\n", t->tm_year + 1900, rectime.year+REC_TIME_BASE_YEAR, rectime.month, rectime.day, rectime.hour, rectime.minute, rectime.second);
}

void RECTIME2Second(const RECTIME& rectime,time_t& second)
{
    struct tm t;
    t.tm_year   = rectime.year + REC_TIME_BASE_YEAR - 1900;
    t.tm_mon    = rectime.month - 1;
    t.tm_mday   = rectime.day ;
    t.tm_hour   = rectime.hour;
    t.tm_min    = rectime.minute;
    t.tm_sec    = rectime.second ;
    second = mktime(&t);
}

void SetThreadName(const char *name)
{
#ifdef OS_WINDOWS
#error please implement SetThreadName
#else
    prctl(PR_SET_NAME, name);
#endif
}

unsigned int GetCurTime_mSec(void)
{
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);

	unsigned long temp = (tv.tv_sec * 1000) + (tv.tv_nsec) / (1000 * 1000);

	//temp++;
	//temp = temp % ((1ul << 31) - 1);

	temp = temp & 0x7FFFFFFF; // temp = [0,  2^31 - 1]

	//printf("GetCurTime_mSec temp = %d\n", temp);
    return temp;
}

void PrintTimeInterval(char *name, int count)
{
#define INTERVAL_MIN_TIME_MS  20
#define INTERVAL_MAX_TIME_MS  100

    static int cnt = 0;
    static long start_time = 0;

    static long mst_old = 0;
    static long mst_new = 0;

    long totaltime = 0;
    long mst_interval = 0;

    mst_old = mst_new;
    mst_new = GetCurTime_mSec();
    mst_interval = mst_new - mst_old;
    cnt++;

    if (cnt == 10)
    {
        start_time = mst_new;
    }

    if ( (cnt < count || count == 0) && (cnt > 10) )
    {
        totaltime = mst_new - start_time;
        if (mst_interval < INTERVAL_MIN_TIME_MS || mst_interval > INTERVAL_MAX_TIME_MS)
        {
            TRACE("%s:cnt=%4d,%4ldmS f=%3.2f ####\n", name, cnt, mst_interval, (cnt-10)*1000.0/totaltime);
        }
        else
        {
            //if (cnt % 45 == 0)
                TRACE("%s:cnt=%4d,%4ldmS f=%3.2f\n", name, cnt, mst_interval, (cnt-10)*1000.0/totaltime);
        }
    }

    return;
}

BOOL WaitNetWorkOK(char *svr_domain, int TimeOut_sec)
{
    int cnt = 0;
    std::vector<std::string> svr_ip;

	svr_ip.clear();

    do
    {
        struct hostent* he = gethostbyname(svr_domain);
        if (he)
        {
            for (int i = 0; he!= NULL && he->h_addr_list[i] != NULL; i++)
            {
                struct in_addr* pAddr = (in_addr*)(he->h_addr_list[i]);
                char str_ip[20] = {0};

                inet_ntop(AF_INET, (const void *)(&(pAddr->s_addr)), str_ip, 20);
                svr_ip.push_back(str_ip);
            }
        }

		if (svr_ip.size() > 0)
		{
		    std::vector<std::string>::iterator it;
            for (it=svr_ip.begin(); it!=svr_ip.end(); it++)
            {
				TRACE("####%s ip = %s\n", svr_domain, it->c_str());
            }

			return TRUE;
		}

		SleepMs(1000);
		cnt++;

		TRACE("####Wait NetWork OK... %d Sec\n", cnt);
    }
	while (TimeOut_sec < 0 ? TRUE : (cnt < TimeOut_sec));

	TRACE("Wait NetWork Connect Timeout!\n");
	return FALSE;
}

