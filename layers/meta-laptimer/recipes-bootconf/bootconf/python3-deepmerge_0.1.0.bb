inherit setuptools3
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SUMMARY = "A tools to handle merging of nested data structures in python."
SECTION = "devel/python"
HOMEPAGE = "https://deepmerge.readthedocs.io/en/latest/"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://setup.py;beginline=33;endline=33;md5=76f61a7d8ab2ed53293639c86c95ad4b"

inherit pypi

SRC_URI += " file://0001-Use-explicit-Version.patch"
SRC_URI[md5sum] = "e9c8fba06837c1bd9ffd27f35e28ef1a"
SRC_URI[sha256sum] = "3d37f739e74e8a284ee0bd683daaef88acc8438ba048545aefb87ade695a2a34"

BBCLASSEXTEND = "native nativesdk"
