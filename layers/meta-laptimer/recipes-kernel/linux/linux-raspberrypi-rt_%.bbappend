FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append = "\
    file://laptimer.cfg\
    "

# disable the serial cmdline
SERIAL=""
