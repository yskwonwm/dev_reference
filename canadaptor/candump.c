#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/net_tstamp.h>

#include "lib.h"

/* for hardware timestamps - since Linux 2.6.30 */
#ifndef SO_TIMESTAMPING
#define SO_TIMESTAMPING 37
#endif

#define TIMESTAMPSZ 50 /* string 'absolute with date' requires max 49 bytes */

#define MAXSOCK 16    /* max. number of CAN interfaces given on the cmdline */
#define MAXIFNAMES 30 /* size of receive name index to omit ioctls */
#define MAXCOL 6      /* number of different colors for colorized output */
#define ANYDEV "any"  /* name of interface to receive from any CAN interface */
#define ANL "\r\n"    /* newline in ASC mode */



struct if_info { /* bundled information per open socket */
	int s; /* socket */
	char *cmdlinename;
	__u32 dropcnt;
	__u32 last_dropcnt;
};
static struct if_info sock_info[MAXSOCK];

static char *progname;
static char devname[MAXIFNAMES][IFNAMSIZ+1];
static int dindex[MAXIFNAMES];
static int max_devname_len; /* to prevent frazzled device name output */
//static const int canfd_on = 1;

#define MAXANI 4

extern int optind, opterr, optopt;

static volatile int running = 1;


static void sigterm(int signo)
{
	running = 0;
}

static int idx2dindex(int ifidx, int socket)
{

	int i;
	struct ifreq ifr;

	for (i = 0; i < MAXIFNAMES; i++) {
		if (dindex[i] == ifidx)
			return i;
	}

	/* create new interface index cache entry */
	/* remove index cache zombies first */
	for (i = 0; i < MAXIFNAMES; i++) {
		if (dindex[i]) {
			ifr.ifr_ifindex = dindex[i];
			if (ioctl(socket, SIOCGIFNAME, &ifr) < 0)
				dindex[i] = 0;
		}
	}

	for (i = 0; i < MAXIFNAMES; i++)
		if (!dindex[i]) /* free entry */
			break;

	if (i == MAXIFNAMES) {
		fprintf(stderr, "Interface index cache only supports %d interfaces.\n",
			MAXIFNAMES);
		exit(1);
	}

	dindex[i] = ifidx;

	ifr.ifr_ifindex = ifidx;
	if (ioctl(socket, SIOCGIFNAME, &ifr) < 0)
		perror("SIOCGIFNAME");

	if (max_devname_len < (int)strlen(ifr.ifr_name))
		max_devname_len = strlen(ifr.ifr_name);

	strcpy(devname[i], ifr.ifr_name);

	fprintf(stdout,"new index %d (%s)\n", i, (char*)devname[i]);

	return i;
}


int main(int argc, char **argv)
{
	int fd_epoll;
	struct epoll_event events_pending[MAXSOCK];
	struct epoll_event event_setup = {
		.events = EPOLLIN, /* prepare the common part */
	};

    struct can_frame frame2;
	struct can_filter candil;
	unsigned char down_causes_exit = 1;
	unsigned char view = 0;

	int rcvbuf_size = 0;
	int num_events;
	int currmax, numfilter;
	char *ptr, *nptr;
	struct sockaddr_can addr;
	char ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) +
		     CMSG_SPACE(3 * sizeof(struct timespec)) +
		     CMSG_SPACE(sizeof(__u32))];
	struct iovec iov;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	struct can_filter *rfilter;
	can_err_mask_t err_mask;
	struct canfd_frame frame;
	int nbytes, i, maxdlen;
	struct ifreq ifr;
	struct timeval tv;
	int timeout_ms = -1; /* default to no timeout */


	signal(SIGTERM, sigterm);
	signal(SIGHUP, sigterm);
	signal(SIGINT, sigterm);

	//last_tv.tv_sec = 0;
	//last_tv.tv_usec = 0;

	progname = basename(argv[0]);
	currmax = argc - optind; /* find real number of CAN devices */

	if (currmax > MAXSOCK) {
		fprintf(stderr, "More than %d CAN devices given on commandline!\n", MAXSOCK);
		return 1;
	}

	fd_epoll = epoll_create(1);
	if (fd_epoll < 0) {
		perror("epoll_create");
		return 1;
	}

	for (i = 0; i < currmax; i++) {
		struct if_info* obj = &sock_info[i];
		ptr = argv[optind+i];
		nptr = strchr(ptr, ',');

		fprintf(stdout,"open %d '%s'.\n", i, ptr);
//		fprintf(stdout,"open %d '%s'. optind : %d \n", i, ptr,optind);
		obj->s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
		if (obj->s < 0) {
			perror("socket");
			return 1;
		}

		event_setup.data.ptr = obj; /* remember the instance as private data */
		if (epoll_ctl(fd_epoll, EPOLL_CTL_ADD, obj->s, &event_setup)) {
			perror("failed to add socket to epoll");
			return 1;
		}

		obj->cmdlinename = ptr; /* save pointer to cmdline name of this socket */
//fprintf(stdout,"nptr : '%s',ptr : '%s' \n", nptr,ptr);
		if (nptr)
			nbytes = nptr - ptr;  /* interface name is up the first ',' */
		else
			nbytes = strlen(ptr); /* no ',' found => no filter definitions */

		if (nbytes >= IFNAMSIZ) {
			fprintf(stderr, "name of CAN device '%s' is too long!\n", ptr);
			return 1;
		}

		if (nbytes > max_devname_len)
			max_devname_len = nbytes; /* for nice printing */

		addr.can_family = AF_CAN;

		memset(&ifr.ifr_name, 0, sizeof(ifr.ifr_name));
		strncpy(ifr.ifr_name, ptr, nbytes);

		fprintf(stdout,"using interface name '%s'.\n", ifr.ifr_name);

		if (strcmp(ANYDEV, ifr.ifr_name) != 0) {
			if (ioctl(obj->s, SIOCGIFINDEX, &ifr) < 0) {
				perror("SIOCGIFINDEX");
				exit(1);
			}
			addr.can_ifindex = ifr.ifr_ifindex;
		} else
			addr.can_ifindex = 0; /* any can interface */



		if (nptr) {
			/* found a ',' after the interface name => check for filters */
			/* determine number of filters to alloc the filter space */
			numfilter = 0;
			ptr = nptr;
			while (ptr) {
				numfilter++;
				ptr++; /* hop behind the ',' */
				ptr = strchr(ptr, ','); /* exit condition */
			//	fprintf(stdout,"%d ptr : '%s' \n", numfilter,ptr);
			}

			rfilter = (struct can_filter*)malloc(sizeof(struct can_filter) * numfilter);
			if (!rfilter) {
				fprintf(stderr, "Failed to create filter space!\n");
				return 1;
			}

			numfilter = 0;
			err_mask = 0;

			while (nptr) {

				ptr = nptr + 1; /* hop behind the ',' */
				nptr = strchr(ptr, ','); /* update exit condition */

				if (sscanf(ptr, "%x:%x",
					   &rfilter[numfilter].can_id,
					   &rfilter[numfilter].can_mask) == 2) {
					rfilter[numfilter].can_mask &= ~CAN_ERR_FLAG;
					if (*(ptr + 8) == ':')
						rfilter[numfilter].can_id |= CAN_EFF_FLAG;
					numfilter++;
				} else if (sscanf(ptr, "#%x", &err_mask) != 1) {
					fprintf(stderr, "Error in filter option parsing: '%s'\n", ptr);
					return 1;
				}
			}

			if (err_mask)
				setsockopt(obj->s, SOL_CAN_RAW, CAN_RAW_ERR_FILTER,
					   &err_mask, sizeof(err_mask));

			if (numfilter)
				setsockopt(obj->s, SOL_CAN_RAW, CAN_RAW_FILTER,
					   rfilter, numfilter * sizeof(struct can_filter));

			free(rfilter);

		} /* if (nptr) */

		/* try to switch the socket into CAN FD mode */
		//setsockopt(obj->s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canfd_on, sizeof(canfd_on));

		if (rcvbuf_size) {
			int curr_rcvbuf_size;
			socklen_t curr_rcvbuf_size_len = sizeof(curr_rcvbuf_size);

			/* try SO_RCVBUFFORCE first, if we run with CAP_NET_ADMIN */
			if (setsockopt(obj->s, SOL_SOCKET, SO_RCVBUFFORCE,
				       &rcvbuf_size, sizeof(rcvbuf_size)) < 0) {
				fprintf(stdout,"SO_RCVBUFFORCE failed so try SO_RCVBUF ...\n");
				if (setsockopt(obj->s, SOL_SOCKET, SO_RCVBUF,
					       &rcvbuf_size, sizeof(rcvbuf_size)) < 0) {
					perror("setsockopt SO_RCVBUF");
					return 1;
				}

				if (getsockopt(obj->s, SOL_SOCKET, SO_RCVBUF,
					       &curr_rcvbuf_size, &curr_rcvbuf_size_len) < 0) {
					perror("getsockopt SO_RCVBUF");
					return 1;
				}

				/* Only print a warning the first time we detect the adjustment */
				/* n.b.: The wanted size is doubled in Linux in net/sore/sock.c */
				if (!i && curr_rcvbuf_size < rcvbuf_size * 2)
					fprintf(stderr, "The socket receive buffer size was "
						"adjusted due to /proc/sys/net/core/rmem_max.\n");
			}
		}

		if (bind(obj->s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			perror("bind");
			return 1;
		}
	}

	/* these settings are static and can be held out of the hot path */
	iov.iov_base = &frame;
	msg.msg_name = &addr;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = &ctrlmsg;

	while (running) {
		num_events = epoll_wait(fd_epoll, events_pending, currmax, timeout_ms);
		if (num_events == -1) {
			if (errno != EINTR)
				running = 0;
			continue;
		}

		/* handle timeout */
		if (!num_events && timeout_ms >= 0) {
			running = 0;
			continue;
		}

		for (i = 0; i < num_events; i++) {  /* check waiting CAN RAW sockets */
			struct if_info* obj = (struct if_info*)events_pending[i].data.ptr;
			int idx;

			/* these settings may be modified by recvmsg() */
			iov.iov_len = sizeof(frame);
			msg.msg_namelen = sizeof(addr);
			msg.msg_controllen = sizeof(ctrlmsg);
			msg.msg_flags = 0;

			nbytes = recvmsg(obj->s, &msg, 0);
			idx = idx2dindex(addr.can_ifindex, obj->s);
fprintf(stdout, "read size %d \n",nbytes);

			if (nbytes < 0) {
				if ((errno == ENETDOWN) && !down_causes_exit) {
					fprintf(stderr, "%s: interface down\n", devname[idx]);
					continue;
				}
				perror("read");
				return 1;
			}

			if ((size_t)nbytes == CAN_MTU)
				maxdlen = CAN_MAX_DLEN;
			else if ((size_t)nbytes == CANFD_MTU)
				maxdlen = CANFD_MAX_DLEN;
			else {
				fprintf(stderr, "read: incomplete CAN frame\n");
				return 1;
			}

			/* once we detected a EFF frame indent SFF frames accordingly */
			if (frame.can_id & CAN_EFF_FLAG)
				view |= CANLIB_VIEW_INDENT_SFF;

			printf("%*s", max_devname_len, devname[idx]);

			fprint_long_canframe(stdout, &frame, NULL, view, maxdlen);
			printf("\n");

			fflush(stdout);
		}
	}

	for (i = 0; i < currmax; i++)
		close(sock_info[i].s);

	close(fd_epoll);

	return 0;
}
