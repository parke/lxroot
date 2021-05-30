

#  version  20210526


default     ?=  help
Wextra      ?=  -Wextra  -Wno-unused-parameter
gpp_opts    ?=  -fmax-errors=2  -Wall  -Werror  $(Wextra)
bin_lxroot  ?=  bin/lxroot
demo_dir    ?=  /tmp/lxroot-demo
unit_dir    ?=  /tmp/lxroot-unit


default:  $(default)


help:
	@  echo
	@  echo  'usage:'
	@  echo  '  make  build         #  build lxroot'
	@  echo  '  make  unit          #  run the unit tests'
	@  echo  '  make  unit-clean    #  delete the test chroot environment'
	@  echo  '  make  demo          #  run the interactive demo'
	@  echo  '  make  demo-clean    #  delete the demo chroot environment'


$(bin_lxroot):  Makefile  lxroot.cpp  bin
	g++  -g   $(gpp_opts)  lxroot.cpp  -o $@


bin:
	mkdir  bin


build:  $(bin_lxroot)


$(demo_dir)  $(unit_dir):
	mkdir  -p  $@


unit:  $(bin_lxroot)  $(demo_dir)  $(unit_dir)
	cp  $(bin_lxroot)  $(unit_dir)/lxr
	bash  demo.sh  alpine_extract  $(unit_dir)  nr
	bash  unit.sh  $(unit_dir)


unit-clean:
	@  #  paths hardcoded for safety
	rm  -f   /tmp/lxroot-unit/lxr
	rm  -rf  /tmp/lxroot-unit/nr


demo:  $(bin_lxroot)  $(demo_dir)
	cp  $(bin_lxroot)  $(demo_dir)/lxroot
	bash  demo.sh  alpine  $(demo_dir)


demo-clean:
	@  #  paths hardcoded for safety.
	rm  -f   /tmp/lxroot-demo/lxroot
	rm  -rf  /tmp/lxroot-demo/newroot


clean:
	rm  -f  $(bin_lxroot)
