#ifndef _MONITOR_H_
#define _MONITOR_H_

#include "erl_nif.h"

#include "irex_nif.h"

void receiver_close(struct receiver_info* info);
int receiver_init(struct receiver_info* info);

#endif // _MONITOR_H_
