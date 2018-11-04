# #!/bin/sh
home_dir="$PROOT"
echo "Home Directory is $home_dir"

bit_file=`ls $home_dir/subsystems/linux/hw-description | grep .bit`
echo "Bit file name is $bit_file"
echo "the_ROM_image:" > $home_dir/output.bif
echo "{" >> $home_dir/output.bif
echo "[bootloader]$home_dir/images/linux/zynq_fsbl.elf" >> $home_dir/output.bif
echo "$home_dir/subsystems/linux/hw-description/$bit_file" >> $home_dir/output.bif 
echo "$home_dir/images/linux/u-boot.elf" >> $home_dir/output.bif
echo "}" >> $home_dir/output.bif

echo "Generating BOOT.BIN"
$home_dir/bootgen/bin/bootgen -image  $home_dir/output.bif -o $home_dir/images/linux/BOOT.BIN -w on
cp $home_dir/images/linux/BOOT.BIN /tftpboot/BOOT.BIN
