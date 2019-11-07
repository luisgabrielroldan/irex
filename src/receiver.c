#include "receiver.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/gpio.h>

#define READ_EVENT_CONT 1
#define READ_EVENT_ERROR -1
#define READ_EVENT_SUCCESS 0

static int send_message(ErlNifEnv *env,
                        ERL_NIF_TERM atom_event,
                        ErlNifPid *pid,
                        int64_t timestamp,
                        int value)
{
    ERL_NIF_TERM msg = enif_make_tuple3(env,
                                        atom_event,
                                        enif_make_int64(env, timestamp),
                                        enif_make_int(env, value));

    return enif_send(env, pid, NULL, msg);
}

static void chardev_close(struct receiver_info *info) {
    if (info->req_fd > 0) {
        close(info->req_fd);
    }

    if (info->fd > 0) {
        close(info->fd);
    }
}

static int chardev_open(struct receiver_info *info) {
    int fd;
    int ret;
    struct gpioevent_request req;

    info->fd = -1;
    info->req_fd = -1;

    fd = open("/dev/gpiochip0", O_RDONLY);
    if (fd < 0)
        return -1;

    req.lineoffset = info->gpio_pin;
    req.handleflags = GPIOHANDLE_REQUEST_INPUT | GPIOHANDLE_REQUEST_ACTIVE_LOW;
    req.eventflags = GPIOEVENT_REQUEST_BOTH_EDGES;
    strcpy(req.consumer_label, "irex-receiver");

    ret = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &req);
    if (ret == -1) {
        ret = -errno;
        error("Failed to issue GET EVENT IOCTL (%d)", ret);
        goto exit_close_error;
    }

    info->fd = fd;
    info->req_fd = req.fd;

    return 0;

exit_close_error:
    if (close(fd) == -1)
        error("Failed to close GPIO character device file");
    return -1;
}

static int read_event(int fd, struct gpioevent_data *event) {
    int ret = read(fd, event, sizeof(struct gpioevent_data));
    if (ret == -1) {
        if (errno == -EAGAIN) {
            return READ_EVENT_CONT;
        } else {
            error("Failed to read event (return=%d)", ret);
            return READ_EVENT_ERROR;
        }
    }

    if (ret != sizeof(struct gpioevent_data)) {
        error("Reading event failed");
        return -1;
    }

    return READ_EVENT_SUCCESS;
}

void *events_poller_thread(void *pinfo) {
    struct receiver_info *info = (struct receiver_info *)pinfo;
    int fd = info->req_fd;
    int pipefd = info->poller_pipe[0];
    struct pollfd fdset[2];
    struct gpioevent_data event;
    ErlNifEnv *env = enif_alloc_env();
    ERL_NIF_TERM atom_event = enif_make_atom(env, "recv_event");

    debug("Poller thread started");

    for(;;) {

        fdset[0].fd = fd;
        fdset[0].events = POLLIN | POLLPRI;
        fdset[0].revents = 0;

        fdset[1].fd = pipefd;
        fdset[1].events = POLLIN;
        fdset[1].revents = 0;

        int rc = poll(fdset, 2, -1);

        if (rc < 0) {
            if (errno == EINTR)
                continue;

            error("Poll failed (errno=%d)", rc);
            break;
        }

        if (fdset[1].revents & (POLLIN | POLLHUP | POLLNVAL)) {
            break;
        }

        if (fdset[0].revents) {
            if (fdset[0].revents & POLLIN) {
                switch (read_event(fd, &event)) {
                case READ_EVENT_ERROR:
                    break;
                case READ_EVENT_CONT:
                    break;
                default:
                    switch (event.id) {
                    case GPIOEVENT_EVENT_RISING_EDGE:
                        send_message(env, atom_event, &info->pid, event.timestamp, 1);
                        break;
                    case GPIOEVENT_EVENT_FALLING_EDGE:
                        send_message(env, atom_event, &info->pid, event.timestamp, 0);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }

    enif_free_env(env);

    debug("Poller thread finished");

    return NULL;
}

void receiver_close(struct receiver_info* info) {
    close(info->poller_pipe[0]);
    close(info->poller_pipe[1]);

    enif_thread_join(info->poller_tid, NULL);

    chardev_close(info);
}

int receiver_init(struct receiver_info* info) {

    if (pipe(info->poller_pipe) < 0) {
        error("Poller thread pipe failed");
        return -1;
    }

    if (chardev_open(info) != 0) {
        error("Open GPIO failed");
        return -1;
    }

    // Create thread
    if (enif_thread_create("gpio_poller", &info->poller_tid, events_poller_thread, info, NULL) != 0) {
        error("Create poller thread failed");
        return -1;
    }

    return 0;
}
