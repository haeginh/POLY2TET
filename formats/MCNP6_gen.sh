#! /bin/bash

##Generate new folder
new_dir=$7

newFile=${new_dir}"/"$1"_MCNP6.i"
cp ./formats/MCNP6/PHANTOM_MCNP6.i ${newFile}

phantom=`echo $1 | tr '[A-Z]' '[a-z]'`
c1=`expr $6 + 1`
c2=`expr $6 + 2`
c3=`expr $6 + 3`
impNo=`expr $5 + 2`

#cp   ${new_dir}"/../"$1".material" ${new_dir}"/"${phantom}.material

sed -i -e "s/{PHANTOM}/${phantom}/g" \
       -e "s/{c1}/${c1}/g" -e "s/{c2}/${c2}/g" -e "s/{c3}/${c3}/g" \
       -e "s/{HX}/$2/g"    -e "s/{HY}/$3/g"    -e "s/{HZ}/$4/g"    \
       -e "s/{impNo}/${impNo}/g" ${newFile}
## echo ${newFile} 
