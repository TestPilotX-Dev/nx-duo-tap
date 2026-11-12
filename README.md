# NX Duo Tap

NX Duo Tap is the Switch side of **NX Duo**, the local-multiplayer-at-a-distance
mode of [NX Paradox](https://testpilotx.net/nx-paradox/). It is an Atmosphère
sysmodule that sends the console's picture and sound to the PC over USB and
receives the remote player's controller.

NX Duo Tap is a modified version of **[Switch Frame Tap](https://github.com/TomasUribe/switch-frame-tap)**
by Tomas Uribe. All the credit for the original work is his. NX Duo Tap is
maintained by TestPilotX so NX Duo can follow new Switch firmware and
Atmosphère releases quickly.

## Install

NX Paradox installs it for you: **NX Duo → Prepare Switch and PC**.

To install it by hand, unzip `nx-duo-tap-<version>-switch.zip` from the
[releases](https://github.com/TestPilotX-Dev/nx-duo-tap/releases) over the root of
the microSD card and restart the console. It uses the same files and title IDs
as Switch Frame Tap (`0100000000000C20` and `0100000000000C21`), so it replaces
it in place.

### Windows (USB driver)

The PC needs the WinUSB driver for the USB device **Switch Frame Tap** (USB ID
`1209 5F1E`); NX Duo Tap keeps that device name. The NX Duo assistant in NX
Paradox walks you through it (**NX Duo → Prepare Switch and PC**): it opens
Zadig and checks the result. For any other PC viewer, install it once with
[Zadig](https://zadig.akeo.ie/), with the console out of the dock (handheld)
and connected to the PC with a USB-C data cable: the device appears a few
seconds after the console reaches the HOME menu. If it never appears, open the
NX Duo Tap app on the Switch: it says whether the module is running, or that it
is not compatible with this firmware yet.

1. Do not enable **Options → List All Devices**.
2. Select **Switch Frame Tap** and check that the USB ID boxes read `1209`
   `5F1E`. If it is not in the list, it already has a driver or the module is
   not running: close Zadig.
3. Only then choose **WinUSB** and press **Install Driver**.

If another device is selected, or the button reads **Replace Driver**, don't
press it: Zadig would put WinUSB on that device, and it could stop working.

## Build

The Switch side is built in GitHub Actions: **Actions → NX Duo Tap · Switch
build → Run workflow**, with the version and the Atmosphère tag to build
against. The libnx and libtesla commits and the `devkitpro/devkita64` image
digest default to those of the last good build. The workflow runs
`tools/make_release.sh` in that image and keeps the sysmodule's ELF next to the
zip, to read crash reports. Its `BUILDINFO` lists what went into the build and
the firmware range the zip works on.

## License

GPL-2.0, like Switch Frame Tap and Atmosphère. See `LICENSE` and `NOTICE`.
The source of every release is the tagged commit of this repository.
