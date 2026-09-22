# Setting Up Watcom C/C++ 10.6 for Phase 2 (Byte-Matching)

To achieve a 100% bit-for-bit matching decompilation (Phase 2), we must use the exact compiler the developers used in 1997: **Watcom C/C++ 10.6**. 

Because Watcom 10.6 is proprietary abandonware and uses a 16-bit installer that doesn't natively run on Windows 11, it cannot be downloaded automatically via `tools/setup_decomp_tools.py` like Open Watcom V2. You will need to manually procure and extract it using an emulator like DOSBox.

## Step 1: Procure the Compiler
1. Download the **Watcom C/C++ 10.6 CD-ROM ISO** from an abandonware archive such as [WinWorldPC](https://winworldpc.com/product/watcom-c-c/106) (Select the "Watcom C-C++ 10.6" CD ISO).
2. Extract the downloaded archive so you have the `.iso` file ready.

## Step 2: Install via DOSBox
Since the original installer (`INSTALL.EXE`) is a 16-bit DOS application, it will crash on modern 64-bit Windows. We can use DOSBox to run the setup:

1. Download and install [DOSBox](https://www.dosbox.com/).
2. Create a temporary folder on your PC to hold the extracted compiler, e.g., `C:\watcom_temp`.
3. Open DOSBox and mount both the temporary folder and the CD ISO by running these commands:
   ```dos
   Z:\> mount c C:\watcom_temp
   Z:\> imgmount d "C:\path\to\your\watcom-10.6.iso" -t iso
   ```
4. Start the installer:
   ```dos
   Z:\> d:
   D:\> install.exe
   ```

## Step 3: Installer Configuration
Follow the Watcom Setup wizard with these specific choices:
- **Target OS:** Make sure to select DOS and Windows NT (or select all platforms). The `BINNT` host binaries are required.
- **Install Directory:** Leave it as the default `C:\WATCOM`.
- **Select Components:** Perform a "Full Installation" to ensure all libraries (`LIB386`), headers (`H`), and tools are extracted.
- Skip modifying `AUTOEXEC.BAT` or `CONFIG.SYS` when prompted at the end.
- Exit the installer and close DOSBox.

## Step 4: Integrate with the Workspace
Our project's build system (`tools/build_decomp.py`) is already natively designed to support Watcom 10.6! It automatically falls back to `binnt\wcc386.exe` because 10.6 doesn't have a `binnt64` directory like V2 does.

1. Navigate to your local project directory: `c:\Stuff\Proyects\RacingDynamite\tools`
2. Rename the current `watcom` folder (which contains Open Watcom V2) to `watcom_v2` as a backup.
3. Open your temporary install folder: `C:\watcom_temp\WATCOM`
4. Copy all of the contents (the `BINNT`, `BINW`, `H`, `LIB386` folders, etc.) into a new `tools\watcom` directory in the project.

## Step 5: Verify the Toolchain
Once the folder is in place, you can test if the project detects the 1997 compiler by running the build script:

```bash
uv run python tools/build_decomp.py
```

If successful, you will see output confirming compilation, and we can immediately begin Phase 2!
