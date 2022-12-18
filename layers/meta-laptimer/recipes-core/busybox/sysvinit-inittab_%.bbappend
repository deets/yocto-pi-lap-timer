
do_install:append() {
    sed -i 's$AMA0:12345:respawn:/bin/start_getty 115200 ttyAMA0 vt102$AMA0:12345:respawn:/bin/login -f root$g' ${D}${sysconfdir}/inittab
}
