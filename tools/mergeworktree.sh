#!/bin/bash


function relpath ()
{
    IFS="${IFS= 	}"
    local _ifs="$IFS"
    local file="$1"
    local base=${2:-$($pwd)}
    [[ "$base" != */ ]] && base=${base}/
    
    declare -a afile abase
    IFS=/ afile=( $file )
    IFS=/ abase=( $base )
    
    local common=
    for ((lvl=1; lvl < ${#afile[*]}; lvl++))
      do
      [ "${afile[lvl]}" != "${abase[lvl]}" ] && break
      common=$common/${afile[lvl]}
    done
    
    file=${file#$common/}
    base=${base#$common/}
    
    local extglob=$(shopt -p extglob)
    shopt -s extglob
    up=${base//+([^\/])\//../}
    [ -z "$up" ] && up=./
    eval $extglob
    
    echo "${up}${file}"
    IFS="$_ifs"
}


pushd `dirname $0`/..
this="`pwd`";
root="$this"

while true ; do
    if [ "`pwd`" = "/" ]; then
	break;
    fi
    cd ..
    
    if [ -d libcpp-core ]; then
	node="`pwd`"/libcpp-core
	if [ ! "$node" = "$this" ]; then
	    root="$node"
	fi
    fi

    if [ ! -e .git ]; then
	break;
    fi
done

if [ ! "$root" = "$this" ]; then
    if [ ! -e "$root"/src/bin ]; then
	mkdir "$root"/src/bin
    fi
    if [ ! -e "$root"/src/obj ]; then
	mkdir "$root"/src/obj
    fi
    cd "$this"/src
    if [ ! -h bin ]; then
	echo linking bin to "$root"/src/bin
	r=$(relpath "$root"/src/bin "$this"/src)
	rm -rf bin
	ln -s $r .
    fi
    if [ ! -h obj ]; then
	echo linking obj to "$root"/src/obj
	r=$(relpath "$root"/src/obj "$this"/src)
	rm -rf obj
	ln -s $r .
    fi
fi

popd
