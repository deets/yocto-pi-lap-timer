#!/bin/sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin
DAEMON=/usr/bin/pigpiod
NAME=pigpiod
DESC="pigpiod daemon"
OPTS=""

case "$1" in
  start)
	echo -n "Starting $DESC: "
	start-stop-daemon --start -x "$DAEMON" -- $OPTS
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
	start-stop-daemon --start -x "$DAEMON" -- $OPTS
	echo "$NAME."
	;;
  *)
	N=/etc/init.d/$NAME
	echo "Usage: $N {start|stop|restart|force-reload}" >&2
	exit 1
	;;
esac

exit 0
