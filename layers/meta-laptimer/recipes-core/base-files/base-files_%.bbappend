FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " \
    file://root.profile\
    file://interfaces\
    file://fstab.patch;patchdir=${WORKDIR}\
    "

do_install:append () {
    # install root .profile
    install -d ${D}/home/root
    install root.profile ${D}/home/root/.profile
    # install minimal /etc/network/interfaces
    install -d ${D}/etc/network
    install -m 0644 ${WORKDIR}/interfaces ${D}${sysconfdir}/network/interfaces
}
