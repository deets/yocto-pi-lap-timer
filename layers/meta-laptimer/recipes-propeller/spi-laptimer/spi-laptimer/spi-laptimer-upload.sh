#!/bin/sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin
NAME=spi-laptimer-upload
DESC="spi laptimer upload"
OPTS=""

case "$1" in
  start)
      propman -p17 /usr/share/spi-laptimer/spi-laptimer.binary 2>&1 > /tmp/propman.log
	;;
  *)
	N=/etc/init.d/$NAME
	echo "Usage: $N {start|stop|restart|force-reload}" >&2
	exit 1
	;;
esac

exit 0
