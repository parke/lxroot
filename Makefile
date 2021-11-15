

#  version  20211111


bin       ?=  bin
demo      ?=  /tmp/lxroot-demo
static    ?=  /tmp/lxroot-static
unit      ?=  /tmp/lxroot-unit

Wextra    ?=  -Wextra  -Wno-unused-parameter
gpp_opts  ?=  -fmax-errors=2  -Wall  -Werror  $(Wextra)

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
  make  demo3           #  run the Arch Linux + Chromium demo	\n\
  make  static          #  build a statically linked lxroot	\n\
								\n\
  make  clean           #  delete bin/lxroot			\n\
  make  unit-clean      #  delete the unit test environment	\n\
  make  demo-clean      #  delete the demo environment		\n\
  make  demo3-clean     #  delete the demo3 environment		\n\
  make  static-clean    #  delete the static build environment  \n\
								\n\
  make  demo3-root      #  enter demo3 as (simulated) root	"


$(bin)/lxroot:  $(deps)  $(bin)/lxroot-unit
	$(bin)/lxroot-unit
	g++  -g   $(gpp_opts)  lxroot.cpp  -o $@


#  Note:  Lxroot has two sets of unit tests.
#    1)  unit.cpp compiles to bin/lxroot-unit and test various C++ functions.
#    2)  aux/unit.sh tests the compiled bin/lxroot executable.
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


#  demo 3  -----------------------------------------------------------  demo 3




demo3:  demo-prepare
	/bin/sh  aux/demo.sh  demo3  $(demo)


demo3-root:  demo-prepare
	/bin/sh  aux/demo.sh  demo3  $(demo)  root


demo3-clean:
	@  #  paths hardcoded for safety
	mkdir  -p       /tmp/lxroot-demo/demo3
	chmod  -R  u+w  /tmp/lxroot-demo/demo3
	rm     -rf      /tmp/lxroot-demo/demo3
