FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

# Ensure we have pgrep
SRC_URI_append = "\
    file://defconfig.patch;patchdir=${WORKDIR}\
    "
