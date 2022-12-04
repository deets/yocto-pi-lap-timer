# Based on https://github.com/troydhanson/meta-nanomsg
DESCRIPTION = "nanomsg"
HOMEPAGE = "https://nanomsg.org"
SECTION = "base"
DEPENDS = ""
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=587b3fd7fd291e418ff4d2b8f3904755"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}-${PV}:"

SRC_URI = "git://github.com/nanomsg/nanomsg.git;protocol=https"
SRCREV = "1749fd7b039165a91b8d556b4df18e3e632ad830"

S = "${WORKDIR}/git"

inherit cmake

# The autotools configuration I am basing this on seems to have a problem with a race condition when parallel make is enabled
PARALLEL_MAKE = ""
