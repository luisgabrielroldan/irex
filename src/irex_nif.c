#include "irex_nif.h"
#include "receiver.h"

ERL_NIF_TERM make_ok_tuple(ErlNifEnv *env, ERL_NIF_TERM value)
{
    struct irex_priv *priv = enif_priv_data(env);

    return enif_make_tuple2(env, priv->atom_ok, value);
}

ERL_NIF_TERM make_error_tuple(ErlNifEnv *env, const char *reason)
{
    struct irex_priv *priv = enif_priv_data(env);
    ERL_NIF_TERM reason_atom = enif_make_atom(env, reason);

    return enif_make_tuple2(env, priv->atom_error, reason_atom);
}

static void receiver_info_dtor(ErlNifEnv *env, void *obj)
{
    struct receiver_info *info = (struct receiver_info*) obj;
    receiver_close(info);
}

static void receiver_info_stop(ErlNifEnv *env, void *obj, int fd, int is_direct_call)
{
    (void) env;
    (void) obj;
    (void) fd;
    (void) is_direct_call;
}

static void receiver_info_down(ErlNifEnv *env, void *obj, ErlNifPid *pid, ErlNifMonitor *monitor)
{
    (void) env;
    (void) obj;
    (void) pid;
    (void) monitor;
}

static ErlNifResourceTypeInit receiver_info_init = {receiver_info_dtor, receiver_info_stop, receiver_info_down};

static int load(ErlNifEnv *env, void **priv_data, ERL_NIF_TERM info) {

    struct irex_priv *priv = enif_alloc(sizeof(struct irex_priv));

    if (!priv) {
        error("Can't allocate irex_priv");
        return 1;
    }

    priv->atom_ok = enif_make_atom(env, "ok");
    priv->atom_error = enif_make_atom(env, "error");
    priv->receiver_info_rt = enif_open_resource_type_x(env, "receiver_info", &receiver_info_init, ERL_NIF_RT_CREATE, NULL);

    *priv_data = (void *) priv;
    return 0;
}

static void unload(ErlNifEnv *env, void *priv_data)
{
    (void) env;

    debug("unload");
    enif_free(priv_data);
}

static ERL_NIF_TERM start_receiver(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    int gpio_line;
    char gpio_dev[32];
    int active_low;
    ErlNifPid pid;

    struct irex_priv *priv = enif_priv_data(env);

    if (argc != 4 ||
            !enif_get_string(env, argv[0], gpio_dev, sizeof(gpio_dev), ERL_NIF_LATIN1) ||
            !enif_get_int(env, argv[1], &gpio_line) ||
            !enif_get_int(env, argv[2], &active_low) ||
            !enif_get_local_pid(env, argv[3], &pid)) {
        return enif_make_badarg(env);
    }


    debug(gpio_dev);

    struct receiver_info *info = enif_alloc_resource(priv->receiver_info_rt, sizeof(struct receiver_info));

    info->closed = 1;
    info->fd = -1;
    info->gpio_line = gpio_line;
    info->active_low = active_low;
    info->pid = pid;

    if (receiver_init(gpio_dev, gpio_line, info) < 0) {
        return make_error_tuple(env, "start_failed");
    }

    ERL_NIF_TERM info_resource = enif_make_resource(env, info);
    enif_release_resource(info);

    return make_ok_tuple(env, info_resource);
}

static ERL_NIF_TERM stop_receiver(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    struct irex_priv *priv = enif_priv_data(env);
    struct receiver_info *info;

    if (argc != 1 ||
            !enif_get_resource(env, argv[0], priv->receiver_info_rt, (void**) &info))
        return enif_make_badarg(env);

    receiver_close(info);

    return priv->atom_ok;

}

static ErlNifFunc nif_funcs[] = {
    {"start_receiver", 4, start_receiver, ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"stop_receiver", 1, stop_receiver, 0}
};

ERL_NIF_INIT(Elixir.IRex.Nif, nif_funcs, load, NULL, NULL, unload)
