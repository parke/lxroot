Lxroot is a clone of chroot, however lxroot does not require root
access.

Lxroot allows a non-root user to quickly and easily create a
chroot-like environment (technically a Linux user namespace) and then
run one or more programs inside that namespace.  Lxroot can be used to:

  -  run a rolling Linux distro's userland on a non-rolling host system

  -  run a legacy userland on a modern Linux system

  -  run software in an "altered version" of the host OS itself

  -  create clean, controlled and isolated environments for installing
       building, and/or running software packages

  -  deny software access to the network

  -  limit access to the filesystem

  -  and possibly for other purposes, as well


====  Usage

Usage:  lxroot  [-nprx]  [ path | @profile ]  [ -- [n=v] command ]

  common options
    -n     provide network access (via CLONE_NEWNET == 0)
    -p     provide access to pulseaudio (may only work on Ubuntu?)
    -r     simulate root user (map uid and gid to zero)
    -x     provide X11 access (mount --bind /tmp/.X11-unix)
    n=v    name=value ...    (set environment variables)

  other options
    --version    display version information

path is the new root direcotry.

profile is currently an undocmunted feature that allows multiple
directories to be mounted (via binds) in new locations.  For example,
you could bind directory A to /, and then bind directory B to
/home/user.  You can also specify which directories should be
read-only.  Please contact me if you have questions about profiles.

Environment variables may be specified via name=value.

Note:  It is possible to run lxroot inside of an lxroot environment,
thereby creating nested environments (up to the nesting limit imposed
by the Linux kernel).


====  Examples

Below are examples showing how to run Alpine Linux and Void Linux in
an lxroot.  These examples were tested on an Ubuntu 20.04 host system.


====  Example  -  Alpine Linux

Run the following commands:

$  git  clone  https://github.com/parke/lxroot.git
$  cd  lxroot
$  g++  -Wall  -Werror  lxroot.c  -o lxroot
$  wget  'http://dl-cdn.alpinelinux.org/alpine/v3.11/releases/x86_64/alpine-minirootfs-3.11.6-x86_64.tar.gz'
$  mkdir  alpine-3.11.6
$  cd  alpine-3.11.6
$  tar  xzf  ../alpine-minirootfs-3.11.6-x86_64.tar.gz
$  cp  -i  /etc/resolv.conf  etc/
$  ../lxroot  -nr  .

The above ../lxroot command will create an "lxroot environment" and
run /bin/sh inside that environment.  Your prompt should change to
something like this:

user  -nr  ./alpine-3.11.6  ~

"user" will be your username from outside of the lxroot.  "-nr" means
that lxroot has network access and appears (inside the lxroot) to have
uid and gid of zero.  "./alpine-3.11.6" is the "name" of the lxroot.
In this case, it is the name of the directory that contains the
lxroot.  '~' is the current working directory.

You can now run commands inside the lxroot:

user  -nr  ./alpine-3.11.6  ~  id
uid=0(root) gid=0(root) groups=0(root)

user  -nr  ./alpine-3.11.6  ~  pwd
/root

user  -nr  ./alpine-3.11.6  ~  echo "$PS1"
\[\e[0;36m\]user  \[\e[0;91m\]-nr\[\e[0;36m\]  ./alpine-3.11.6  \W\[\e[0;39m\]  

user  -nr  ./alpine-3.11.6  ~  apk update
fetch http://dl-cdn.alpinelinux.org/alpine/v3.11/main/x86_64/APKINDEX.tar.gz
fetch http://dl-cdn.alpinelinux.org/alpine/v3.11/community/x86_64/APKINDEX.tar.gz
v3.11.6-90-g318b6c3504 [http://dl-cdn.alpinelinux.org/alpine/v3.11/main]
v3.11.6-90-g318b6c3504 [http://dl-cdn.alpinelinux.org/alpine/v3.11/community]
OK: 11271 distinct packages available

user  -nr  ./alpine-3.11.6  ~  apk add bash
(1/4) Installing ncurses-terminfo-base (6.1_p20200118-r4)
(2/4) Installing ncurses-libs (6.1_p20200118-r4)
(3/4) Installing readline (8.0.1-r0)
(4/4) Installing bash (5.0.11-r1)
Executing bash-5.0.11-r1.post-install
Executing busybox-1.31.1-r9.trigger
OK: 8 MiB in 18 packages

user  -nr  ./alpine-3.11.6  ~


====  Example  -  Arch Linux

Download the Arch Linux .iso file.  I assume you already know how to
download it.  The file I downloaded is:
archlinux-2020.06.01-x86_64.iso

Next, we extract the chroot installation environment from the .iso
file.  This is a two step process.  We will use 7z to exrcat the .sfs
file from the .iso file.  We will then use unsquashfs to unsquash the
.sfs file.

For details on 7z, see:
https://www.tecmint.com/extract-files-from-iso-files-linux/

On Ubuntu, if you need to install 7zip, run:
sudo  apt-get  install  p7zip-full

On Ubuntu, if you need to install unsquasfs, run:
sudo  apt-get  install  squashfs-tools

$  7z  l  archlinux-2020.06.01-x86_64.iso  |  grep  sfs
$  7z  x  archlinux-2020.06.01-x86_64.iso  arch/x86_64/airootfs.sfs
$  unsquashfs  -no  arch/x86_64/airootfs.sfs
$  rm  -r  arch

$  rm  squashfs-root/etc/resolv.conf
$  cp  /etc/resolv.conf  squashfs-root/etc/
$  lxroot  -nr  squashfs-root

#  pacman-key  --init
#  pacman-key  --populate  archlinux
#  mkdir  -p  /mnt/etc/pacman.d
#  mkdir  -p  /mnt/var/lib/pacman/
#  cp  -a  /etc/pacman.d/gnupg  /mnt/etc/pacman.d/

Edit /etc/pacman.d/mirrorlist and add a line at the top to specify a
server near your location.

#  pacman  -r /mnt  -Sy  base

(Aside:  Instead of "base", consider installing only "pacman".  This
will avoid part (most but not all?) of systemd.)

The above command will ask the following twice:
downloading required keys...
:: Import PGP key 3B94A80E50A477C7, "Jan Alexander Steffens (heftig) <[snip]>"? [Y/n]

Both times, I pressed: "y" and then enter.

The above command will then freeze after installing gnupg.  I do not
know why the command freezes.  I pressed ^C, and then ran the same
command again.  This time the command will print some other warnings,
but it will exit.

We have now installed the base of Arch Linux.  We will now exit the
lxroot.

#  exit

We need to edit the new mirrorlist and specify a server.

$  vi  squashfs-root/mnt/etc/pacman.d/mirrorlist
$  cp  /etc/resolv.conf  squashfs-root/mnt/etc/

We now enter an lxroot in the /mnt directory.

$  lxroot  -nr  squashfs-root/mnt/

Inside the lxroot, your prompt should be:
user  -nr  ./mnt  ~

You should now be able to use pacman to install packages.  For
example, to install nodejs and npm:

user  -nr  ./mnt  ~  pacman  -Sy  nodejs  npm

user  -nr  ./mnt  ~  node
Welcome to Node.js v14.4.0.
Type ".help" for more information.
>


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

user  -nr  ./void  ~

(Note, the above prompt is out of date.  The prompt should be similar
to the prompt described in the Alpine Linux example.)

You can now run commands inside the lxroot:

user  -nr  ./void  ~  xbps-install -S

user  -nr  ./void  ~  xbps-install -Su

user  -nr  ./void  ~  xbps-install -Su

user  -nr  ./void  ~  which htop
which: no htop in (/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin)

user  -nr  ./void  ~  xbps-install -Su htop

user  -nr  ./void  ~  htop


====  Exmaple  -  Debian or Ubuntu

To install .deb packages inside an lxroot, please see:
https://github.com/parke/rapt
