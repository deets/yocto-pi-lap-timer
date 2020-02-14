FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI_append = " \
    file://root.profile\
    "

do_install_append () {
    # install root .profile
    install -d ${D}/home/root
    install root.profile ${D}/home/root/.profile
}
