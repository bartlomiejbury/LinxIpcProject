#!/bin/sh

#!/bin/sh

while getopts 'r:o:' opt; do
  case "$opt" in
    r)
      reroute_file="$OPTARG"
      echo "Processing option 'r' with '${reroute_file}' argument"
      ;;
   
    ?|h)
      echo "Usage: $(basename $0) [-a] [-b] [-c arg]"
      exit 1
      ;;
  esac
done

shift $(expr $OPTIND - 1 )
for file in $@
do
    input="$(basename ${file})"
    objcopy --redefine-syms=${reroute_file} ${file}
done
