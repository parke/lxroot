# Lxroot - a software virtualization tool


### About Lxroot

`lxroot` is a lightweight and safe alternative to `chroot`, Docker, and other software virtualization tools.

`lxroot` allows a non-root user to easily and safely create a "chroot-style" virtual software environment (via Linux namespaces), and then run one or more programs inside that environment.


###  Project Status

As of September 2023, Lxroot is still under intermittent development.
I seem to work on Lxroot several times per year.  Typically, when I
work on Lxroot, I spend several days either adding new features or
refactoring existing features to make Lxroot easier to use.

I personally run software inside Lxroot every hour of every day, all year long.
As time passes, I am increasing the quantity and variety of programs that I run inside of Lxroot.

To the best of my knowledge, I am the only person who uses Lxroot
regularly.  Therefore, I have been investing my energy in improving
Lxroot, rather than documenting the improvements and publishing
updates.  **Therefore, if you are interested in using Lxroot, please let
me know so that I can provide you with the updated and improved source
code.**

###  Lxroot Worlds

As of September 2023, I now call an Lxroot-based chroot-style-environment a "world".

To manually create Apline Linux world named `alpine`, I could
(for example), do the following:

```
$  mkdir  -p  alpine/newroot    #  this directory is required.
$  tar  xzf  alpine-minirootfs.tar.gz  -C alpine/newroot
$  mkdir  alpine/home           #  this directory is optional.
$  mkdir  alpine/tmp            #  this directory is optional.
$  lxroot  alpine               #  use Lxroot to enter the world.
```

When Lxroot runs, the following directories will be bind-mounted into
`alpine/newroot`:

```
source           target
alpine/home  ->  alpine/newroot/home
alpine/tmp   ->  alpine/newroot/tmp
```

Then Lxroot will chroot into `alpine/newroot`.

Any of `newroot`, `home`, and `tmp` can be symbolic links.  This
allows directories to be shared across multiple worlds.  (Lxroot can
bind-mount any subdirectory into `newroot`, not just the two examples shown above.)

On a Linux kernel version 5.11 and later,
Lxroot can also create and chroot into a kernel-level overlay
filesystem.  An Lxroot-world can simultaneously use both an overlay filesystem
and one or more bind-mounted subdirectories.

I have written custom scripts that create Lxroot-worlds based on
various Linux distributions.  The below table summaries the types of
Lxroot-worlds that I regularly create and/or use.  The distros are
ordered from most commonly used (by me) to least commonly used.

|  Base distro           |  Create world with     |  Install binary packages with  |  Can build packages?  |
|  :--                   |  :-:                   |  :-:                           |  :-:                  |
|  Debian, Ubuntu, Mint  |  [`mk-deb.sh`](https://github.com/parke/lxroot/blob/master/aux/mk-deb.sh)  |  `fakeroot` and `apt`          |  Probably?            |
|  Alpine                |  `mk-alpine.sh`  (1)   |  `apk`                         |  Probably?            |
|  Flatpak               |  `mk-alpine.sh`  (1)   |  `flatpak`                     |  Probably?            |
|  Arch                  |  `mk-arch.sh`  (1)     |  `pacman`                      |  Probably?            |
|  Arch AUR              |  `mk-arch.sh`  (1)     |  (n/a)                         |  Probably?            |
|  Void                  |  Create by hand?  (2)  |  `xbps-install`?  (2)          |  Probably?            |

Notes:  (1) I have not yet published `mk-alpine.sh` and `mk-arch.sh`.  (2) It has been a long time since I used Void Linux inside Lxroot, but there is a good chance that XBPS will "just work" inside Lxroot.

(Aside: Previously, I used a separate tool called `vland` to create and
work with Lxroot environments.  However, `vland` is now obsolete and
unsupported.  `vland`'s functionality was either (a) moved into Lxroot
itself, or (b) moved into smaller, stand-alone scripts.)

### Videos

-  Lxroot presentation at PackagingCon 2021:  [**abstract**](https://pretalx.com/packagingcon-2021/talk/PMPUSW/)  |  [**slides**](https://pretalx.com/media/packagingcon-2021/submissions/PMPUSW/resources/20211110_Lxroot_7ILURuB.pdf)  |  [**video**](https://www.youtube.com/watch?v=1rw7ww0k_mk)

<!--  version  20230926.0  -->
