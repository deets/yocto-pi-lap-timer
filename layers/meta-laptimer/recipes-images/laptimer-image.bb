# Base this image on core-image-base
include recipes-core/images/core-image-base.bb

SUMMARY = "A console-only minimal FPV laptimer image"

RDEPENDS:append = "\
    bootconf-bootfiles \
    "

CORE_IMAGE_EXTRA_INSTALL:append = "\
    avahi-daemon \
    bootconf \
    dropbear \
    i2c-tools \
    iw \
    os-release \
    pigpio \
    python3-pip \
    python3-tailon \
    rsync \
    screen \
    tcpdump \
    udev-extra-rules \
    userland \
    wpa-supplicant \
    "

inherit extrausers

EXTRA_USERS_PARAMS = " \
    usermod -p '\$6\$rotorhazard\$KH8j4pzn6P25yIQeNpurAAGEJhO7qu2A4/PP1nXSHWTtiHegb0mC50WeuuZZ9mgBMEaa8QLtaqf0UllmqAjC30' root \
"
