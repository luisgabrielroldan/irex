#ifndef __GPIO_H
#define __GPIO_H

#include <linux/ioctl.h>
#include <linux/types.h>

/* Maximum number of requested handles */
#define GPIOHANDLES_MAX 64

/* Linerequest flags */
#define GPIOHANDLE_REQUEST_INPUT	(1UL << 0)
#define GPIOHANDLE_REQUEST_OUTPUT	(1UL << 1)
#define GPIOHANDLE_REQUEST_ACTIVE_LOW	(1UL << 2)
#define GPIOHANDLE_REQUEST_OPEN_DRAIN	(1UL << 3)
#define GPIOHANDLE_REQUEST_OPEN_SOURCE	(1UL << 4)

/* Eventrequest flags */
#define GPIOEVENT_REQUEST_RISING_EDGE	(1UL << 0)
#define GPIOEVENT_REQUEST_FALLING_EDGE	(1UL << 1)
#define GPIOEVENT_REQUEST_BOTH_EDGES	((1UL << 0) | (1UL << 1))

/**
 * struct gpioevent_request - Information about a GPIO event request
 * @lineoffset: the desired line to subscribe to events from, specified by
 * offset index for the associated GPIO device
 * @handleflags: desired handle flags for the desired GPIO line, such as
 * GPIOHANDLE_REQUEST_ACTIVE_LOW or GPIOHANDLE_REQUEST_OPEN_DRAIN
 * @eventflags: desired flags for the desired GPIO event line, such as
 * GPIOEVENT_REQUEST_RISING_EDGE or GPIOEVENT_REQUEST_FALLING_EDGE
 * @consumer_label: a desired consumer label for the selected GPIO line(s)
 * such as "my-listener"
 * @fd: if successful this field will contain a valid anonymous file handle
 * after a GPIO_GET_LINEEVENT_IOCTL operation, zero or negative value
 * means error
 */
struct gpioevent_request {
	__u32 lineoffset;
	__u32 handleflags;
	__u32 eventflags;
	char consumer_label[32];
	int fd;
};

/**
 * GPIO event types
 */
#define GPIOEVENT_EVENT_RISING_EDGE 0x01
#define GPIOEVENT_EVENT_FALLING_EDGE 0x02

/**
 * struct gpioevent_data - The actual event being pushed to userspace
 * @timestamp: best estimate of time of event occurrence, in nanoseconds
 * @id: event identifier
 */
struct gpioevent_data {
	__u64 timestamp;
	__u32 id;
};

#define GPIO_GET_CHIPINFO_IOCTL _IOR(0xB4, 0x01, struct gpiochip_info)
#define GPIO_GET_LINEINFO_IOCTL _IOWR(0xB4, 0x02, struct gpioline_info)
#define GPIO_GET_LINEHANDLE_IOCTL _IOWR(0xB4, 0x03, struct gpiohandle_request)
#define GPIO_GET_LINEEVENT_IOCTL _IOWR(0xB4, 0x04, struct gpioevent_request)

#endif /* __GPIO_H */
