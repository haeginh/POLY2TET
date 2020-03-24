#! /bin/bash

##Generate new folder
new_dir=$5

##variables
phantom=$1
HX=$2
HY=$3
HZ=$4

##copy the format file
newFile=${new_dir}"/"${phantom}"_PHITS.pin"
cp ./formats/PHITS/PHANTOM_PHITS.pin ${newFile}
XX=`echo "${HX}+0.2"|bc`
XY=`echo "${HY}+0.2"|bc`
XZ=`echo "${HZ}+0.2"|bc`
sed -i -e "s/{PHANTOM}/${phantom}/g" \
       -e "s/{HX}/${HX}/g"    -e "s/{HY}/${HY}/g"    -e "s/{HZ}/${HZ}/g"    \
       -e "s/{XX}/${XX}/g"    -e "s/{XY}/${XY}/g"    -e "s/{XZ}/${XZ}/g"    \
       ${newFile}
sed -i "s/{PHANTOM}/${phantom}/g" ${new_dir}"/"${phantom}".cell"
echo ${newFile} 
