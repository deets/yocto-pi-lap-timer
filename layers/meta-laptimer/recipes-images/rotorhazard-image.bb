include laptimer-image.bb

# TODO: verify if all python-* after python-rotorhazard
# are needed. Should come automatically through RDEPENDS
CORE_IMAGE_EXTRA_INSTALL_append = "\
    python-rotorhazard\
    python\
    python-flask\
    python-flask-socketio\
    python-gevent\
    python-monotonic\
    python-werkzeug-tests\
    python-flask-alchemy\
    python-smbus\
"