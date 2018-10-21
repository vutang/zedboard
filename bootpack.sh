curdir=`pwd`
biffile=output_.bif

bitfile=`ls $(pwd)/subsystems/linux/hw-description/*.bit`
echo $bitfile

echo "the_ROM_image:" > $biffile
echo "{" >> $biffile
echo "	[bootloader]$(pwd)/images/linux/zynq_fsbl.elf" >> $biffile
echo "	$bitfile" >> $biffile
echo "	$(pwd)/images/linux/u-boot.elf" >> $biffile
echo "}" >> $biffile

# Setting SDK Environment
source /opt/Xilinx/SDK/2015.4/settings64.sh

# Call bootgen
bootgen -image $biffile -o $(pwd)/images/linux/BOOT.BIN -w

# Copy to tftp dir
cd $curdir/images/linux/
cp BOOT.BIN image.ub rootfs.jffs2 /tftpboot/
cd $curdir
