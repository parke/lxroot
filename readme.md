# Lxroot - a software virtualization tool


### About Lxroot

`lxroot` is a lightweight and safe alternative to `chroot`, Docker, and other software virtualization tools.

`lxroot` allows a non-root user to easily and safely create a "chroot-style" virtual software environment (via Linux namespaces), and then run one or more programs (a "guest userland") inside that environment.


###  Quickstart tutorial

`vland` is a convenience wrapper around `lxroot`.  The quickest way to start using `lxroot` is to use `vland`.  For example:

```
$  wget  https://github.com/parke/vland/raw/main/vland
$  /bin/sh  vland  --create  arch  arch-guest
$  /bin/sh  vland  --pkg  arch-guest  install  chromium
$  /bin/sh  vland  arch-guest  -nx  --  chromium
```

The above commands will use `vland` to: create an Arch Linux guest named `arch-guest`, and then install and run the Chromium web browser.  (The guest will be created inside `$HOME/.local/vland`.)

`vland` can also build packages inside a guest.  For example, the below commands will build and install Arch Linux's `lua` package:

```
$  /bin/sh  vland  --pkg  arch-guest  build-install  lua
$  /bin/sh  vland  arch-guest  --  lua  -v
```

`lxroot` is a small C++ program.  `vland` is a single file Dash shell script.  Both run without root access.

Learn more by reading the [**`vland` tutorial**](https://github.com/parke/vland/wiki/tutorial) and the [**`lxroot` tutorial**](https://github.com/parke/lxroot/wiki/tutorial).


### Videos

-  Lxroot presentation at PackagingCon 2021:  [**abstract**](https://pretalx.com/packagingcon-2021/talk/PMPUSW/)  |  [**slides**](https://pretalx.com/media/packagingcon-2021/submissions/PMPUSW/resources/20211110_Lxroot_7ILURuB.pdf)  |  [**video**](https://www.youtube.com/watch?v=1rw7ww0k_mk)


###  Supported guest Linux distributions

The below table summarizes the Linux distributions that I have used as guests with `vland` and `lxroot`.  

|  Guest distro  |  Can install as guest?  |  Can install packages?  |  Can build packages?  |
|  :--           |  :-:                    |  :-:                    |  :-:                  |
|  Alpine        |  yes                    |  yes                    |  yes                  |
|  Arch          |  yes                    |  yes                    |  yes                  |
|  Ubuntu        |  work in progress       |  work in progress       |  probably             |
|  Void          |  yes                    |  yes                    |  probably             |

Other Linux distributions may also work inside `lxroot`.  (Some level of custom shimming may be required.)

Due to `lxroot`'s safety, simplicity, efficiency, and ability to run without root access, `lxroot` has a few [limitations](https://github.com/parke/lxroot/wiki/limitations).  These limitations may or may not affect your particular use case.


### Learn more

*  `vland`
   *  [use cases](https://github.com/parke/lxroot/wiki/use_cases)
   *  [tutorial](https://github.com/parke/vland/wiki/tutorial) - installation and usage
   *  [man page](https://github.com/parke/vland/wiki/man_page)
   *  [wiki](https://github.com/parke/lxroot/wiki)
* `lxroot`
   *  [tutorial](https://github.com/parke/lxroot/wiki/tutorial) - installation and usage
   *  [man page](https://github.com/parke/lxroot/wiki/man_page)
   *  [limitations](https://github.com/parke/lxroot/wiki/limitations)
