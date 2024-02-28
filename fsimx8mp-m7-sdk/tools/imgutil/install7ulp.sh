#!/bin/bash

echo "Converting binaries to images..."
echo "Ivt will look like 
Combined Image Info:
--------------------------------------
   base_addr         = 0x1ffd0000
   ivt_offset        = 0x00001000
   hab_ivt.hdr       = 0x412000d1
   hab_ivt.entry     = 0x1ffd2311
   hab_ivt.self      = 0x1ffd1000
   hab_ivt.csf       = 0x00000000
   hab_ivt.boot_data = 0x1ffd1020
   hab_ivt.dcd       = 0x1ffd1040
   boot_data.start   = 0x1ffd0000
   boot_data.size    = 0x0000b7c0
   boot_data.plugin  = 0x00000000

 Info for CSF file generation
--------------------------------------
   ivt_addr  = 0x1ffd1000, ivt_offset  = 0x00001000
   app_addr  = 0x1ffd2000, app_offset  = 0x00002000, app_size = 0x????????
--------------------------------------
"
BUILD_PATH=${PWD}
for f in ${BUILD_PATH}/bin/picocoremx7ulp/*.bin ; do
	filename=$(basename -- "$f")
	filename="${filename%.*}"

	mv $f ${BUILD_PATH}/tools/imgutil/evkmcimx7ulp/
	cd ${BUILD_PATH}/tools/imgutil/evkmcimx7ulp/
	${BUILD_PATH}/tools/imgutil/evkmcimx7ulp/mkimg.sh ram $filename
	mv -f $filename.img ${BUILD_PATH}/bin/picocoremx7ulp
	rm -f $filename.bin
done
echo "Done!"
