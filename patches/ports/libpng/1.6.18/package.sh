REMOTE_ARCHIVE="https://ghostkernel.org/repository/libpng/libpng-1.6.18.tar.gz"
UNPACKED_DIR=libpng-1.6.18
ARCHIVE=libpng-1.6.18.tar.gz

port_unpack() {
	tar -xf $ARCHIVE
}

port_install() {
	../$UNPACKED_DIR/configure --host=$HOST --prefix=$PREFIX
	make
	make DESTDIR=$INSTALL_DIR install
	sudo cp -r $INSTALL_DIR/* $SYSROOT  
}
