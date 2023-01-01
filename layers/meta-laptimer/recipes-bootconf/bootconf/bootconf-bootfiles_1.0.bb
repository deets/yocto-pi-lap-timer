DESCRIPTION = "Places system.conf into the FAT32 partition"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

COMPATIBLE_MACHINE = "^rpi$"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " \
    file://system.conf\
    "

inherit deploy nopackages

do_deploy() {
    install -d ${DEPLOYDIR}/${BOOTFILES_DIR_NAME}
    cp ${WORKDIR}/system.conf ${DEPLOYDIR}/${BOOTFILES_DIR_NAME}/
}

addtask deploy before do_build after do_install
do_deploy[dirs] += "${DEPLOYDIR}/${BOOTFILES_DIR_NAME}"

PACKAGE_ARCH = "${MACHINE_ARCH}"
