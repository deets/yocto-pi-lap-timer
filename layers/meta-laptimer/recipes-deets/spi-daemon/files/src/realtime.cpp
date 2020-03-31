// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#include "realtime.h"

#include <sched.h>
#include <sys/mman.h> // Needed for mlockall()
#include <malloc.h>


void prevent_page_locking()
{
  // Now lock all current and future pages from preventing of being paged
  if (mlockall(MCL_CURRENT | MCL_FUTURE ))
  {
    perror("mlockall failed:");
  }
}

void set_priority(int prio, int sched)
{
  struct sched_param param;
  // Set realtime priority for this thread
  param.sched_priority = prio;
  if (sched_setscheduler(0, sched, &param) < 0)
    perror("sched_setscheduler");
}
