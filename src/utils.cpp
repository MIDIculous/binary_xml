#include "utils.h"
#include "macros.h"
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>


bool WriteToFile(const char *filename,const char *fmt,...)
{
    FILE *f = fopen(filename,"w");
    if (f == nullptr)
    {
        ERRNO_SHOW(1007,"WriteToFile-fopen",filename);
        return false;
    }
    va_list ap;
    va_start(ap,fmt);
    
    int ok = vfprintf(f,fmt,ap);
    if (ok < 0)
    {
        ERRNO_SHOW(1008,"WriteToFile-fprintf",fmt);
        return false;
    }

    va_end(ap);

    if (fclose(f) == EOF)
    {
        ERRNO_SHOW(1009,"WriteToFile-fclose",filename);
        return false;
    }
    return true;
}

char *ReadFile(const char *filename,bool MustExist,int *size)
// will read file and allocate enough memory for file store in memory
{
    int fd = open(filename,O_RDONLY);
    if (fd == -1)
    {
//      if (!MustExist && errno == ) return nullptr;
        ERRNO_SHOW(1010,"ReadFile-open",filename);
        return nullptr; 
    }
    off_t file_size = FileGetSizeByFd(fd);
    char *data = nullptr;
    if (file_size > 0)
    {
        data = new char[file_size+1];
        ssize_t bytes_read = read(fd,data,file_size);
        if (bytes_read < 0)
        {
            ERRNO_SHOW(1011,"ReadFile-read",filename);
            delete [] data;
            data = nullptr;
        }
        else if (bytes_read < file_size)
        {
            std::cerr << ANSI_RED_BRIGHT << "ReadFile-read(" << filename << ") cannot read whole file (" 
                << bytes_read << " < " << file_size << ")" ANSI_RESET_LF;
            delete [] data;
            data = nullptr;
        }
        else
            data[file_size] = '\0';

    }

    int ok_close = close(fd);   
    if (ok_close == -1)
    {
        ERRNO_SHOW(1012,"ReadFile-close",filename);
    }

    if (size != nullptr)
        *size = file_size;
    return data;    
}

off_t FileGetSizeByFd(int fd)
{
    struct stat file_info;
    int err = fstat(fd,&file_info);
    if (err != 0)
    {
        if (errno == ENOENT)
            return -1; // file not exists - no message

        ERRNO_SHOW(1013,"fstat","FileGetSizeByFd"); 
        return -1; // failed
    }
    if (S_ISDIR(file_info.st_mode)) 
    {
        std::cerr << "\n" ANSI_RED_BRIGHT "Error " ANSI_RED_DARK " while " 
            ANSI_RED_BRIGHT "FileGetSizeByFd" ANSI_RED_DARK " 'file' is directory!" ANSI_RESET;
        return -1; // directory
    }
    return file_info.st_size;
}

void SkipSpaces(const char *&p)
{
    while (isspace(*p)) p++;
}

bool SkipLine(const char *&p)
{
    while (*p != '\0' && *p != '\n') p++;
    if (*p == '\0') return false; // end of file
    p++;    // skip \n
    return true;
}

bool Scan(const char *&p,const char *beg)
{
    while (isspace(*p)) p++;

    const char *p1 = p;
    while (*p1 == *beg && *beg != 0)
    {
        p1++;
        beg++;
    }
    if (*beg != 0) return false; // not equal
    p = p1;
    return true;
}

bool ScanInt(const char *&p,int &value)
{
    while (isspace(*p)) p++;

    const char *p1 = p;
    bool neg = (*p1 == '-');
    if (neg) p1++;
    if (!isdigit(*p1)) return false;

    int x = 0;
    while (isdigit(*p1))
        x = x *10 + (*(p1++) - '0');
    
    p = p1;
    value = (neg ? -x : x);
    return true;
}

int  GetInt(const char *p)
{
    while (isspace(*p)) p++;
    
    bool neg = (*p == '-');
    if (neg) p++;
    if (!isdigit(*p)) return INT_NULL_VALUE;

    int x = 0;
    while (isdigit(*p))
        x = x *10 + (*(p++) - '0');
    
    return neg ? -x : x;
    
}



bool ScanStr(const char *&p,char separator,char *value,unsigned value_size)
{
    while (isspace(*p)) p++;

    char *d = value;
    if (separator == ' ') // bere cokoliv, co je isspace
    {
        while (*p != '\0' && !isspace(*p))
        {
            if (value_size > 1)
            {
                *(d++) = *p;
                value_size--;
            }
            p++;
        }
        if (*p != '\0') p++; // sezere oddelovac!
    }
    else
    {
        while (*p != '\0' && *p != separator)
        {
            if (value_size > 1)
            {
                *(d++) = *p;
                value_size--;
            }
            p++;
        }
        if (*p == separator) p++; // sezere oddelovac!
    }
    *d = '\0';
    return *value != '\0'; // string is not empty
}

bool StrWrite(char *&dest,const char *dest_limit,const char *&src,const char *src_limit)
{
    int dst_space = dest_limit - dest;
    int src_size = (src_limit != nullptr ? src_limit - src : strlen(src));
    if (src_size > dst_space) return false; // nic neudělám a končím
    strncpy(dest,src,src_size);
    dest += src_size;
    src = src_limit;
    return true;
}


const char *MakeIdent(const char *Source,const char *style)
// non reetrant!!!
{
    static char _MakeIdent_Ident[64];
    char *d = _MakeIdent_Ident;
    int len = 0;
// number at the beginning
    if (*Source >= '0' && *Source <= '9')
    {
        *(d++) = '_';
        *(d++) = *(Source++);
        len += 2;
    }

    bool space_to_underscore = strchr(style,'_');
    bool space_to_uppercase = strchr(style,'U');
    bool all_uppercase = strstr(style,"U!") != nullptr;
    bool need_uppercase = space_to_uppercase;

    while (len < sizeof(_MakeIdent_Ident)-1 && *Source != '\0')
    {
        if (*Source >= 'a' && *Source <= 'z' ||
            *Source >= 'A' && *Source <= 'Z' ||
            len > 0 && *Source >= '0' && *Source <= '9' ||
            *Source == '_')
        {
            if (need_uppercase || all_uppercase)
            {
                *(d++) = toupper(*(Source++));
                need_uppercase = false;
            }
            else
                *(d++) = *(Source++);
            len++;
        }
        else if (isspace(*Source))
        {
            if (space_to_underscore)
            {
                *(d++) = '_';
                len++;
            }
            else if (space_to_uppercase)
            {
                need_uppercase = true;
            }
            while (isspace(*Source)) Source++;
        }
        else
            Source++; // bad char for id
    }
    *d = '\0';
    return _MakeIdent_Ident;
}

int GetIdentLen(const char *p)
{
    int len = 0;
    if (!isalpha(*p) && *p != '_') return 0;
    len++;p++;
    while(isalnum(*p) || *p == '_')
    {
        len++;p++;
    }
    return len; 
}