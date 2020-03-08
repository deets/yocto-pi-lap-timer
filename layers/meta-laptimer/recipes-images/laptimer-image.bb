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
    rsync \
    udev-extra-rules \
    wpa-supplicant \
    "
