#define _GNU_SOURCE
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dlfcn.h>
#include <poll.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* Runtime workaround for the SDK RTPInterface.cpp unchecked writev().
 * Only the two-vector $/channel/length RTP framing on RTSP port 8554 is
 * affected. A partial packet is completed or the TCP connection is shut
 * down; never silently leave a truncated packet in the byte stream. */
static ssize_t (*original_writev)(int, const struct iovec *, int);
static pthread_once_t init_once = PTHREAD_ONCE_INIT;
static pthread_mutex_t send_lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned long packets, short_writes, would_block, failures;
static void resolve(void) { original_writev = dlsym(RTLD_NEXT, "writev"); }
__attribute__((constructor)) static void reserve_nal_buffer(void)
{
    /* This SDK exports live555's writable maximum-NAL size. Grow it before
     * any RTP sink is constructed; do not use addresses tied to a binary. */
    unsigned *maximum = dlsym(RTLD_DEFAULT, "_ZN15OutPacketBuffer7maxSizeE");
    if (maximum && *maximum >= 60000 && *maximum < 2*1024*1024) {
        unsigned previous = *maximum;
        *maximum = 2*1024*1024;
        fprintf(stderr,"RTSP_NAL_BUFFER previous=%u current=%u\n",previous,*maximum);
    }
}
static int matches(int fd, const struct iovec *iov, int count)
{
    struct sockaddr_in local;
    socklen_t len = sizeof(local);
    return count == 2 && iov[0].iov_len == 4 && iov[0].iov_base &&
        ((const unsigned char *)iov[0].iov_base)[0] == '$' &&
        getsockname(fd, (struct sockaddr *)&local, &len) == 0 &&
        local.sin_family == AF_INET && ntohs(local.sin_port) == 8554;
}
ssize_t writev(int fd, const struct iovec *iov, int count)
{
    pthread_once(&init_once, resolve);
    if (!original_writev) { errno = ENOSYS; return -1; }
    if (!matches(fd, iov, count)) return original_writev(fd, iov, count);
    struct iovec pending[2] = {iov[0], iov[1]};
    size_t total = iov[0].iov_len + iov[1].iov_len, done = 0;
    int first = 0, error = 0;
    pthread_mutex_lock(&send_lock);
    packets++;
    struct timespec started, now;
    clock_gettime(CLOCK_MONOTONIC, &started);
    while (done < total) {
        clock_gettime(CLOCK_MONOTONIC, &now);
        if (now.tv_sec - started.tv_sec >= 5) { error = ETIMEDOUT; break; }
        struct msghdr message = {.msg_iov=pending+first, .msg_iovlen=2-first};
        ssize_t n = sendmsg(fd, &message, MSG_NOSIGNAL | MSG_DONTWAIT);
        if (n > 0) {
            if ((size_t)n < total - done) short_writes++;
            done += n;
            size_t used = n;
            while (first < 2 && used >= pending[first].iov_len) {
                used -= pending[first].iov_len;
                first++;
            }
            if (first < 2 && used) {
                pending[first].iov_base = (char *)pending[first].iov_base + used;
                pending[first].iov_len -= used;
            }
            continue;
        }
        if (n < 0 && errno == EINTR) continue;
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            would_block++;
            struct pollfd p = {.fd = fd, .events = POLLOUT};
            int r;
            do { r = poll(&p, 1, 100); } while (r < 0 && errno == EINTR);
            if (r > 0 && (p.revents & POLLOUT)) continue;
            if (r == 0) continue;
            error = EPIPE;
            break;
        }
        error = n == 0 ? EPIPE : errno;
        break;
    }
    if (error) {
        failures++;
        fprintf(stderr,"RTSP_TCP_WRITEALL_ERROR errno=%d packet=%zu sent=%zu\n",error,total,done);
        shutdown(fd, SHUT_RDWR);
    }
    pthread_mutex_unlock(&send_lock);
    if (error) { errno = error; return -1; }
    return (ssize_t)done;
}
__attribute__((destructor)) static void report(void)
{
    fprintf(stderr, "RTSP_TCP_WRITEALL packets=%lu short_writes=%lu would_block=%lu failures=%lu\n",
            packets, short_writes, would_block, failures);
}
