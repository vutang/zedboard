bootgen -image output.bif -o /home/vux/Workspace/zed/zPetalinux/images/linux/BOOT.BIN -w
curdir=`pwd`
cd $curdir/images/linux/
cp BOOT.BIN image.ub rootfs.jffs2 /tftpboot/
cd $curdir
