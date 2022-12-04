FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Ensure we have introspection in the DBUS
# API - python dbus needs this
SRC_URI:append = "\
    file://defconfig.patch;patchdir=${WORKDIR}\
    "
