

#  version  20211020


bin       ?=  bin
demo      ?=  /tmp/lxroot-demo
static    ?=  /tmp/lxroot-static
unit      ?=  /tmp/lxroot-unit

Wextra    ?=  -Wextra  -Wno-unused-parameter
gpp_opts  ?=  -fmax-errors=2  -Wall  -Werror  $(Wextra)


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
  make  static-clean    #  delete the static build environment  "


$(bin)/lxroot:  Makefile  lxroot.cpp  help.cpp  str.cpp  $(bin)
	g++  -g   $(gpp_opts)  lxroot.cpp  -o $@


$(bin)  $(unit)  $(demo)  $(demo)-chromium:
	mkdir  -p  $@


build:  $(bin)/lxroot


clean:
	rm  -f  bin/lxroot  ;  true


unit:  $(bin)/lxroot  $(unit)
	cp  $(bin)/lxroot  $(unit)/lxr
	@  echo
	bash  aux/demo.sh  demo1_extract  $(demo)  $(unit)/nr  2>&1
	@  echo
	bash  aux/unit.sh  $(unit)  2>&1


unit-clean:  clean
	@  #  paths hardcoded for safety
	rm  -rf   /tmp/lxroot-unit


demo:  $(bin)/lxroot  $(demo)
	cp  $(bin)/lxroot  $(demo)/lxroot
	bash  aux/demo.sh  demo1  $(demo)


demo-clean:  clean
	@  #  paths hardcoded for safety.
	rm  -f   /tmp/lxroot-demo/lxroot
	rm  -rf  /tmp/lxroot-demo/demo1


static:
	bash  aux/static.sh  $(bin)/lxroot  $(static)


#  demo 3  -----------------------------------------------------------  demo 3


demo3_iso=$(demo)/dist/archlinux-2021.06.01-x86_64.iso
demo3_url=https://mirror.rackspace.com/archlinux/iso/2021.06.01/archlinux-2021.06.01-x86_64.iso


demo3-iso-soft:
	if  [ ! -f $(demo3_iso) ]  ;  then  \
	  wget  --continue  -O  $(demo3_iso)  $(demo3_url)  ;  fi


demo3-base:  demo3-iso-soft  $(bin)/lxroot

	@  echo
	@  echo  'demo3  create userland1'
	bash  aux/demo.sh  demo1_extract  $(demo)  $(demo)/demo3
	cp  /etc/resolv.conf  $(demo)/demo3/etc/
	mkdir  -p  $(demo)/demo3/dist
	ln  -f  $(demo3_iso)  $(demo)/demo3/dist

	@  echo
	cp  aux/demo.sh  $(demo)/demo3/root/
	$(bin)/lxroot  -nw  $(demo)/demo3  \
	  --  /bin/ash  /root/demo.sh  demo3_u1_create_u2

	@  echo
	cp  aux/demo.sh  $(demo)/demo3/userland2/root/
	$(bin)/lxroot  -nr  $(demo)/demo3/userland2  \
	  --  /bin/bash  /root/demo.sh  demo3_u2_create_u3

	@  echo
	cp  aux/demo.sh  $(demo)/demo3/userland2/userland3/root/
	$(bin)/lxroot  -nr  $(demo)/demo3/userland2/userland3  \
	  --  /bin/bash  /root/demo.sh  demo3_u3_finish

	@  echo
	mkdir  -p  $(demo)/demo3/userland2/userland3/$(HOME)


demo3:
	make  demo3-base    #  This allows overrding of the demo3 recipe.
	@  echo
	@  echo
	@  echo  "(  The Demo #3 guest userland has been created.       )"
	@  echo  "(                                                     )"
	@  echo  "(  Make will now launch an interactive shell in the   )"
	@  echo  "(  Demo #3 guest userland.                            )"
	@  echo  "(                                                     )"
	@  echo  "(  Please run 'chromium' in this shell to attempt to  )"
	@  echo  "(  run Chromium.                                      )"
	@  echo  "(                                                     )"
	@  echo  "(  Please press ENTER when you are ready to proceed.  )"
	@  echo
	@  echo
	@  read  discard
	$(bin)/lxroot  -nx  $(demo)/demo3/userland2/userland3


demo3-root:  demo3-base
	$(bin)/lxroot  -nrx  $(demo)/demo3/userland2/userland3


demo3-clean:
	@  #  paths hardcoded for safety
	mkdir  -p       /tmp/lxroot-demo/demo3
	chmod  -R  u+w  /tmp/lxroot-demo/demo3
	rm     -rf      /tmp/lxroot-demo/demo3


