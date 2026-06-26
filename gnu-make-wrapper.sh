#!/bin/sh

mode=$1
pkgdir=$2
shift 2

case "$mode" in
make-gnu)
	makefile="./make-gnu"
	optflags='-g -DABKDEBUG'
	ccflags='-fPIC -Wall -pedantic -pipe'
	;;
makeOpt-gnu)
	makefile="./makeOpt-gnu"
	optflags='-O3 -march=native -funroll-loops $(ABKSYMBOLS)'
	ccflags='-fPIC -pipe'
	;;
*)
	echo "gnu-make-wrapper.sh: unknown mode '$mode'" >&2
	exit 1
	;;
esac

make_pkg() {
	case "$1" in
	with-c)
		shift
		exec make \
			MAKE="$makefile" DEPENDOPT="-MM" CC4LIBS="g++" \
			OPTFLAGS="$optflags" CCFLAGS="$ccflags" \
			C='gcc $(OPTFLAGS) $(CCFLAGS)' \
			CC='g++ $(OPTFLAGS) $(CCFLAGS)' \
			LD="g++ -shared" LDFINAL="g++" SODIRKEY="-Wl,-rpath," \
			SOLABEL="" \
			"$@"
		;;
	*)
		exec make \
			MAKE="$makefile" DEPENDOPT="-MM" CC4LIBS="g++" \
			OPTFLAGS="$optflags" CCFLAGS="$ccflags" \
			CC='g++ $(OPTFLAGS) $(CCFLAGS)' \
			LD="g++ -shared" LDFINAL="g++" SODIRKEY="-Wl,-rpath," \
			SOLABEL="" \
			"$@"
		;;
	esac
}

make_pkg "$@"
