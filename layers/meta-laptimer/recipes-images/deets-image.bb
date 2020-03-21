include rotorhazard-image.bb

CORE_IMAGE_EXTRA_INSTALL_append = "\
    qtserialport \
    "

# pull in QT5 sdk
inherit populate_sdk_qt5_base

TOOLCHAIN_HOST_TASK_append = " nativesdk-qttools-tools"
# turn off bt because the propeller needs that serial port
RPI_EXTRA_CONFIG_append = "dtoverlay=pi3-disable-bt\n"
