#ifndef _IREX_NIF_H_
#define _IREX_NIF_H_

#include "erl_nif.h"

//#define DEBUG

#ifdef DEBUG
#define debug(...) do { enif_fprintf(stderr, __VA_ARGS__); enif_fprintf(stderr, "\r\n"); fflush(stderr); } while(0)
#define error(...) do { debug(__VA_ARGS__); } while (0)
#else
#define debug(...)
#define error(...) do { enif_fprintf(stderr, __VA_ARGS__); enif_fprintf(stderr, "\n"); } while(0)
#endif

struct irex_priv {
    ERL_NIF_TERM atom_ok;
    ERL_NIF_TERM atom_error;
    ErlNifResourceType *receiver_info_rt;
};

struct receiver_info {
  int fd;
  int req_fd;
  int gpio_pin;
  int poller_pipe[2];
  ErlNifTid poller_tid;
  ErlNifPid pid;
};

ERL_NIF_TERM make_ok_tuple(ErlNifEnv *env, ERL_NIF_TERM value);
ERL_NIF_TERM make_error_tuple(ErlNifEnv *env, const char *reason);

#endif // _IREX_NIF_H_

