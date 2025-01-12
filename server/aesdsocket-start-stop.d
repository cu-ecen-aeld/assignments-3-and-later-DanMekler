#! /bin/sh

case "$1" in
    start)
        echo "Starting aesdsocket"
        start-stop-daemon -S -n aesdsocket -a ~/server
    ;;
    stop)
        echo "Stopping aesdsocket"
        start-stop-daemon -K -n aesdsocket
    ;;
    *)
        echo "Usage: $0 {start|stop}"
        exit 1
    ;;
exac

exit 0