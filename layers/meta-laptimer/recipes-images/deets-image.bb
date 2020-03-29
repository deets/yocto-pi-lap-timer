include rotorhazard-image.bb

CORE_IMAGE_EXTRA_INSTALL_append = "\
    propman \
    nanomsg \
    "

# pull in QT5 sdk
inherit populate_sdk_qt5_base

TOOLCHAIN_HOST_TASK_append = " nativesdk-qttools-tools"
