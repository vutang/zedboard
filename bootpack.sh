curdir=`pwd`
biffile=output_.bif

bitfile=`ls /home/vux/Workspace/zed/zPetalinux/subsystems/linux/hw-description/*.bit`
echo $bitfile

echo "the_ROM_image:" > $biffile
echo "{" >> $biffile
echo "	[bootloader]/home/vux/Workspace/zed/zPetalinux/images/linux/zynq_fsbl.elf" >> $biffile
echo "	$bitfile" >> $biffile
echo "	/home/vux/Workspace/zed/zPetalinux/images/linux/u-boot.elf" >> $biffile
echo "}" >> $biffile

# Setting SDK Environment
source /opt/Xilinx/SDK/2015.4/settings64.sh

# Call bootgen
bootgen -image $biffile -o /home/vux/Workspace/zed/zPetalinux/images/linux/BOOT.BIN -w

# Copy to tftp dir
cd $curdir/images/linux/
cp BOOT.BIN image.ub rootfs.jffs2 /tftpboot/
cd $curdir
