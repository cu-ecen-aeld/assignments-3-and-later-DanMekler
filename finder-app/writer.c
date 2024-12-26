#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]){

    openlog("writer", LOG_PID, LOG_USER);

    if (argc < 3){
        syslog(LOG_ERR, "Error: both arguments are required");
        printf("Error: both arguments are required\n");
        closelog();
        return 1;
    }

    char* writefile = argv[1];
    char* writestr = argv[2];

    int fd = open(writefile, O_WRONLY | O_CREAT |O_TRUNC, 777);
    if (fd == -1){
        syslog(LOG_ERR, "open falied: %s", strerror(errno));
        printf("open failed: %s\n", strerror(errno));
        closelog();
        return 1;
    }

    int write_bytes = write(fd, writestr, strlen(writestr));
    if (write_bytes == -1){
        syslog(LOG_ERR, "write falied: %s", strerror(errno));
        printf("write failed: %s\n", strerror(errno));
        close(fd);
        closelog();
        return 1;
    }

    syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);
    printf("Writing %s to %s\n", writestr, writefile);

    close(fd);
    closelog();
    return 0;

}