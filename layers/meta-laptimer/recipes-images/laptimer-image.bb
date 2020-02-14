# Base this image on core-image-base
include recipes-core/images/core-image-base.bb

SUMMARY = "A console-only minimal FPV laptimer image"

CORE_IMAGE_EXTRA_INSTALL_append = "\
    avahi-daemon \
    dropbear \
    rsync \
    udev-extra-rules \
    os-release \
    wpa-supplicant \
    iw \
    i2c-tools \
    bootconf \
    "
