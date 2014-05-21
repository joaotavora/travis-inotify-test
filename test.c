#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
 
#define INOTIFY_TEST_PATH "/tmp/inotify_test.tmp"
static int test_inotify_support()
{
    int is_ok = 1;
    int fd = inotify_init();
    if (fd < 0)
    {
        fprintf(stderr, "inotify_init failed");
    }
    
    /* Mark fd as nonblocking */
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        fprintf(stderr, "fcntl GETFL failed");
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        fprintf(stderr, "fcntl SETFL failed");
    }
    
    if (system("touch " INOTIFY_TEST_PATH))
    {
        fprintf(stderr, "touch failed");
    }
    
    /* Add file to watch list */
    int wd = inotify_add_watch(fd, INOTIFY_TEST_PATH, IN_DELETE | IN_DELETE_SELF);
    if (wd < 0)
    {
        fprintf(stderr, "inotify_add_watch failed: %s", strerror(errno));
    }
    
    /* Delete file */
    if (system("rm " INOTIFY_TEST_PATH))
    {
        fprintf(stderr, "rm failed");
    }
    
    /* Verify that file is deleted */
    struct stat statbuf;
    if (stat(INOTIFY_TEST_PATH, &statbuf) != -1 || errno != ENOENT)
    {
        fprintf(stderr, "File at path " INOTIFY_TEST_PATH " still exists after deleting it");
    }
    
    /* The fd should be readable now or very shortly */
    struct timeval tv = {1, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    int count = select(fd + 1, &fds, NULL, NULL, &tv);
    if (count == 0 || ! FD_ISSET(fd, &fds))
    {
        fprintf(stderr, "inotify file descriptor not readable. Is inotify busted?\n");
        is_ok = 0;
    }
    else
    {
        fprintf(stderr, "inotify seems OK\n");
        is_ok = 1;
    }
    
    if (fd >= 0)
    {
        close(fd);
    }
    return is_ok;
}
 
int main(void)
{
    return test_inotify_support() ? EXIT_SUCCESS : EXIT_FAILURE;
}

