FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Ensure we have pgrep
SRC_URI:append = "\
    file://laptimer.cfg\
    "
