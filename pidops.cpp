#include "pidops.h"
#include <QMessageBox>

#ifdef PLAT_WIN32
#include <windows.h>
#include <winbase.h>
#endif

bool set_affinity(Q_PID qPid, int cpu_num)
{
#ifdef PLAT_LINUX
    // TODO: Implement in terms of sched_setaffinity
    return true;
    (void)qPid;
    (void)cpu_num;
#elif defined PLAT_WIN32
    DWORD_PTR Mask = 1 << cpu_num;
    if(SetProcessAffinityMask(qPid->hProcess, Mask) == 0)
        return false;
    else
        return true;
#endif
}
