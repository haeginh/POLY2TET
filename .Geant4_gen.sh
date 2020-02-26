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
new_dir=$1"_Geant4"
new_dir_base=$1"_Geant4"
num=1;
while [ -d $new_dir ]; do
	new_dir=${new_dir_base}"_"${num}
	num=`expr $num + 1`
done

mkdir ${new_dir}

for file in "${filelist[@]}"; do
	cp -r ${geant4_dir}${file} ${new_dir}"/"
done

##Specify the phantom names
find ${new_dir} -type f -exec sed -i -e "s/PHANTOM/$1/g" -e "s/NEWDIR/${new_dir}/g" -e "s/DEFAULTMAT/145/g" {} \;

##Specify beam size
area=`expr $2 \* $3 \* 4`
sed -e "s/{halfx}/$2/g" -e "s/{halfy}/$3/g" -e "s/{area}/${area}/g" ${new_dir}"/example.in">${new_dir}"/_example.in"

##Rename PHANTOM.cc ane example.in
mv ${new_dir}"/PHANTOM.cc" ${new_dir}"/"$1".cc"
mv ${new_dir}"/_example.in" ${new_dir}"/example.in"

##export path
#export G4TET_dir=${new_dir}
echo ${new_dir} 
