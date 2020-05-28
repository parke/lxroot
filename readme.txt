lxroot is a rootless alternative to chroot.

lxroot allows non-root users to create custom software environments
via the "user namespace" capabilities of the Linux kernel.  These
softawre environments can be used to:

  -  install, build, and run software packages

  -  create clean, controlled and isolated environments for compiling
       and running software

  -  run legacy software on modern systems

  -  possibly other purposes, as well


====  Usage

Usage:  lxroot  [-nprx]  [[.]/path]  [profile]  [ -- [env] command ]

  -n     provide network access (via CLONE_NEWNET == 0)
  -p     provide access to pulseaudio (may only work on Ubuntu)
  -r     simulate root user (map uid and gid to zero)
  -x     provide X11 access (mount --bind /tmp/.X11-unix)
  env    name=value ...

/path or ./path is the new root direcotry.

profile is currently an undocmunted feature that allows multiple
directories to be mounted (via binds) in new locations.  For example,
you could bind directory A to /, and then bind directory B to
/home/user.  Some directories can be read-only, while others can be
read-write.  Please contact me if you have questions about profiles.

Environment variables may be specified via name=value.

Note:  It is possible to nest a child lxroot environment inside of a
parent lxroot environment (up to the nesting limit imposed by the
Linux kernel).


====  Examples

Below are examples showing how to run Alpine Linux and Void Linux in
an lxroot.  These examples were tested on an Ubuntu 20.04 host system.


====  Example  -  Alpine Linux

Run the following commands:

$  git  clone  https://github.com/parke/lxroot.git
$  cd  lxroot
$  g++  -Wall  -Werror  lxroot.c  -o lxroot
$  wget  'http://dl-cdn.alpinelinux.org/alpine/v3.11/releases/x86_64/alpine-minirootfs-3.11.6-x86_64.tar.gz'
$  mkdir  alpine
$  cd  alpine
$  tar  xzf  ../alpine-minirootfs-3.11.6-x86_64.tar.gz
$  cp  -i  /etc/group        etc/
$  cp  -i  /etc/passwd       etc/
$  cp  -i  /etc/resolv.conf  etc/
$  ../lxroot  -nr  .

The above ../lxroot command will create an "lxroot environment" and
run /bin/sh inside that environment.  Your prompt should change to
something like this:

root  lx(todo)  $

You can now run commands inside the lxroot:

root  lx(todo)  id
uid=0(root) gid=0(root) groups=0(root)

root  lx(todo)  $  pwd
/root

root  lx(todo)  $  apk update

root  lx(todo)  $  apk add bash


====  Example  -  Void Linux

$  git  clone  https://github.com/parke/lxroot.git
$  cd  lxroot
$  g++  -Wall  -Werror  lxroot.c  -o lxroot
$  wget  'wget https://alpha.de.repo.voidlinux.org/live/current/void-x86_64-ROOTFS-20191109.tar.xz'
$  mkdir void
$  cd void
$  tar xJf ../void-x86_64-ROOTFS-20191109.tar.xz
$  cp  -i  /etc/resolv.conf  etc/
$  ../lxroot  -nr  .

The above ../lxroot command will create an "lxroot environmont" and
run /bin/sh inside that environment.  Your prompt should change to
something like this:

root  lx(todo)  $

You can now run commands inside the lxroot:

root  lx(todo)  $  xbps-install -S

root  lx(todo)  $  xbps-install -Su

root  lx(todo)  $  xbps-install -Su

root  lx(todo)  $  which htop
which: no htop in (/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin)

root  lx(todo)  $  xbps-install -Su htop

root  lx(todo)  $  htop


====  Exmaple  -  Debian or Ubuntu

To install .deb packages inside an lxroot, please see:
https://github.com/parke/rapt
