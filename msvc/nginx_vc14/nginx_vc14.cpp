extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_time.h>
}

extern "C" void *(*pcre_malloc)(size_t) = nullptr;
extern "C" void(*pcre_free)(void *) = nullptr;

extern "C" void
ngx_localtime(time_t s, ngx_tm_t *tm)
{
#ifdef _WIN32
    struct tm stm;
    localtime_s(&stm, &s);
    tm->wYear = stm.tm_year + 1900;
    tm->wMonth = stm.tm_mon + 1;
    tm->wDay = stm.tm_mday;
    tm->wHour = stm.tm_hour;
    tm->wMinute = stm.tm_min;
    tm->wSecond = stm.tm_sec;
    tm->wDayOfWeek = stm.tm_wday;
#elif defined(NGX_HAVE_LOCALTIME_R)
    localtime_r(&s, tm);
    tm->ngx_tm_mon++;
    tm->ngx_tm_year += 1900;
#else
    ngx_tm_t *t = (ngx_tm_t *)localtime(&s);
    *tm = *t;
    tm->ngx_tm_mon++;
    tm->ngx_tm_year += 1900;
#endif
}

extern "C"
void srandom(unsigned long seed)
{
    srand(seed);
}
