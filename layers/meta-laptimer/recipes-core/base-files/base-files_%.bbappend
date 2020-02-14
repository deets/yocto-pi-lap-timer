FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI_append = " \
    file://root.profile\
    file://interfaces\
    "

do_install_append () {
    # install root .profile
    install -d ${D}/home/root
    install root.profile ${D}/home/root/.profile
    # install minimal /etc/network/interfaces
    install -d ${D}/etc/network
    install -m 0644 ${WORKDIR}/interfaces ${D}${sysconfdir}/network/interfaces
}
