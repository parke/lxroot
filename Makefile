

#  version  20220822


bin       ?=  bin
demo      ?=  /tmp/lxroot-demo
static    ?=  /tmp/lxroot-static
unit      ?=  /tmp/lxroot-unit

Wextra    ?=  -Wextra  #  -Wno-unused-parameter
Werror    ?=  #  -Werror
gpp_opts  ?=  -fmax-errors=2  -Wall  $(Wextra)  $(Werror)

deps      ?=  Makefile  lxroot.cpp  help.cpp  str.cpp  unit.cpp  |  $(bin)


help:
	@  echo  "\
								\n\
Welcome to the Lxroot Makefile.					\n\
								\n\
Usage:								\n\
								\n\
  make  build           #  build bin/lxroot			\n\
  make  unit            #  run the unit tests			\n\
  make  demo            #  run the interactive Alpine demo	\n\
  make  static          #  build a statically linked lxroot	\n\
								\n\
  make  clean           #  delete bin/lxroot			\n\
  make  unit-clean      #  delete the unit test environment	\n\
  make  demo-clean      #  delete the demo environment		\n\
  make  static-clean    #  delete the static build environment"


$(bin)/lxroot:  $(deps)  $(bin)/lxroot-unit
	$(bin)/lxroot-unit  ||  \
          {  /usr/bin/gdb  -q  -ex run  $(bin)/lxroot-unit  ;  false  ;  }
	g++  -g   $(gpp_opts)  lxroot.cpp  -o $@


#  Note:  There are two levels of unit tests:
#    1)  C++ unit tests in unit.cpp
#    2)  command line tests in aux/unit.sh
$(bin)/lxroot-unit:  $(deps)
	g++  -g   $(gpp_opts)  unit.cpp  -o $@


$(bin)  $(unit)  $(demo)  $(demo)-chromium:
	mkdir  -p  $@


build:  $(bin)/lxroot


clean:
	rm  -f  $(bin)/lxroot       ;  true
	rm  -f  $(bin)/lxroot-unit  ;  true


unit:  $(bin)/lxroot  $(unit)
	cp  $(bin)/lxroot  $(unit)/lxr
	@  echo
	bash  aux/demo.sh  demo1_extract  $(demo)  $(unit)/nr  2>&1
	@  echo
	bash  aux/unit.sh  $(unit)  2>&1


unit-clean:  clean
	@  #  paths hardcoded for safety
	rm  -rf   /tmp/lxroot-unit


demo-prepare:  $(bin)/lxroot
	@  echo
	@  echo  'demo-prepare'
	mkdir  -p  $(demo)/bin
	@  #  note  in certain situations, $(bin) may equal $(demo)
	-  cp  $(bin)/lxroot  $(demo)/bin/lxroot
	cp     aux/demo.sh    $(demo)/bin/demo.sh


#  demo:  $(bin)/lxroot  $(demo)
demo:  demo-prepare
	cp  $(bin)/lxroot  $(demo)/lxroot
	bash  aux/demo.sh  demo1  $(demo)


demo-clean:  clean
	@  #  paths hardcoded for safety.
	rm  -f   /tmp/lxroot-demo/lxroot
	rm  -rf  /tmp/lxroot-demo/demo1


static:
	bash  aux/static.sh  $(bin)/lxroot  $(static)
