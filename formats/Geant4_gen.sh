#! /bin/bash

geant4_dir="./formats/Geant4/"
filelist=("PHANTOM.cc"
          "init_vis.mac"
          "vis.mac"
          "CMakeLists.txt"
          "example.in"
          "src"
          "include")

##Generate new folder
new_dir=$4
mkdir ${new_dir}

for file in "${filelist[@]}"; do
	cp -r ${geant4_dir}${file} ${new_dir}"/"
done

##Specify the phantom names
find ${new_dir} -type f -exec sed -i "s/{PHANTOM}/$1/g" {} \;

##Specify beam size
area=`expr $2 \* $3 \* 4`
sed -i -e "s/{halfx}/$2/g" -e "s/{halfy}/$3/g" -e "s/{area}/${area}/g" ${new_dir}"/example.in"

##Rename MRCP_AM_2.cc ane example.in
mv ${new_dir}"/PHANTOM.cc" ${new_dir}"/"$1".cc"

echo "Geant4 files were generated in "${new_dir} 
