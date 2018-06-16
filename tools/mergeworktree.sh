#!/bin/bash


OPTIND=1
chk=
while getopts "hc" o; do
    case "$o" in
	h)  echo usage: mergeworktree.sh "[-c]"
	    exit 0;
	    ;;
	c)  chk=1
            ;;
    esac
done
shift $((OPTIND-1))
[ "$1" = "--" ] && shift


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
this="`pwd`"
hash=`git log --pretty=format:'%h' -n 1`
root="$this"

while true ; do
    if [ "`pwd`" = "/" ]; then
	break;
    fi
    cd ..
    
    if [ -d libcpp-core ]; then
	node="`pwd`"/libcpp-core
	if [ ! "$node" = "$this" ]; then
	    if [ "$chk" = "1" ]; then
		pushd "$node"
		if [ ! "`git log --pretty=format:'%h' -n 1`" = "$hash" ]; then
		    echo "node \"$node\" not in sync with this \"$this\"" 1>&2
		    exit 1
		fi
		popd
	    fi
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
    if [ ! -e "$root"/src/doxygen/html ]; then
	mkdir "$root"/src/doxygen/html
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
    cd "$this"/src/doxygen
    if [ ! -h html ]; then
	echo linking html to "$root"/src/doxygen/html
	r=$(relpath "$root"/src/doxygen/html "$this"/src/doxygen)
	rm -rf html
	ln -s $r .
    fi
fi

popd
