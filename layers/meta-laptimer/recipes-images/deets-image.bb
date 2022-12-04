include rotorhazard-image.bb

CORE_IMAGE_EXTRA_INSTALL:append = "\
    nanomsg \
    propman \
    "

# pull in QT5 sdk
inherit populate_sdk_qt5_base

TOOLCHAIN_HOST_TASK:append = " nativesdk-qttools-tools"
