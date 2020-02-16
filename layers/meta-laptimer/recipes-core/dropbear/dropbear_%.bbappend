FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI_append = "\
    file://dropbear.default\
    "

ROOTFS_POSTPROCESS_COMMAND += "force_dropbear_defaults"

force_dropbear_defaults() {
    install -m 0644 ${WORKDIR}/dropbear.default ${D}${sysconfdir}/default/dropbear
}