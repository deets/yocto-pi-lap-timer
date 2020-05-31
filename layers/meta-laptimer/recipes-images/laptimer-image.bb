# Base this image on core-image-base
include recipes-core/images/core-image-base.bb

SUMMARY = "A console-only minimal FPV laptimer image"

RDEPENDS_append = "\
    bootconf-bootfiles \
    "

CORE_IMAGE_EXTRA_INSTALL_append = "\
    avahi-daemon \
    bootconf \
    dropbear \
    i2c-tools \
    iw \
    os-release \
    pigpio \
    python3-tailon \
    rsync \
    screen \
    tcpdump \
    udev-extra-rules \
    userland \
    wpa-supplicant \
    "
