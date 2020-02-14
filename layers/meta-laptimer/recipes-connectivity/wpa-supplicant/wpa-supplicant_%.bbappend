FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

# Ensure we have introspection in the DBUS
# API - python dbus needs this
SRC_URI_append = "\
    file://defconfig.patch;patchdir=${WORKDIR}\
    "
