#!/bin/sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin
NAME=spi-daemon
DAEMON=/usr/bin/spi-daemon
DESC="SPI daemon"
OPTS="-s 2000 -d /dev/spidev0.0 -u tcp://0.0.0.0:5000"

case "$1" in
  start)
      echo -n "Starting $DESC: "
      /usr/bin/tune-priorities.py
      start-stop-daemon --start -b -x "$DAEMON" -- $OPTS
      echo "$NAME."
      ;;
  stop)
      echo -n "Stopping $DESC: "
      start-stop-daemon --stop -x "$DAEMON"
      echo "$NAME."
      ;;
  restart|force-reload)
      echo -n "Restarting $DESC: "
      start-stop-daemon --stop -x "$DAEMON"
      sleep 1
      start-stop-daemon --start -b -x "$DAEMON" -- $OPTS
      echo "$NAME."
      ;;
  *)
      N=/etc/init.d/$NAME
      echo "Usage: $N {start|stop|restart|force-reload}" >&2
      exit 1
      ;;
esac

exit 0
