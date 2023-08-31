# Alternative setup guide

## Instructions

1. Find out which version of Super Paper Mario you have.
2. Download the [mod](https://github.com/L5050/Brobot-After-Some-Windex/releases/tag/V1.0.5), choose the download that matches your SPM version.
3. Download and install [Wiimms](https://wit.wiimm.de/download.html).
4. Open the Command Prompt in the same folder as your SPM ISO.
5. Type `wit x spm.iso extracted` and press Enter.
6. Once that's done, copy all the files from the mod EXCEPT `modinfo.ini` into the `extracted/files` folder. When Windows asks if you want to replace files, say Yes.
7. In the command prompt, type `wit copy --align-files extracted Brobot.wbfs`
8. The mod should now appear in Dolphin (as Super Paper Mario). Right click the mod and select Properties, then under the Gecko Codes tab click `Add New Code...` and paste in the code below.
9. Download [this save file](https://github.com/L5050/Brobot-After-Some-Windex/releases/tag/save). which will automatically place you at the point in the game where the changes were made.
10. Right click Super Paper Mario in Dolphin and select `Open Wii Save Folder`, then place the downloaded save file in there.

Warning: This will overwrite the save file on the bottom left. If you want the save to use a different file, simply change the number at the end of the file name. (0 = top left, 1 = top right, 2 = bottom left, 3 = bottom right)

## Code

```
0423c878 88030009
0423ca00 98030009
0423c91c 98030009
c223ca18 0000003e
7c0802a6 90010004
9421ffe8 bf810008
48000081 48000048
80279860 80004000
8019edc4 8019f30c
8019f074 80270908
80272b5c 801a59a8
804f3998 2e2f6d6f
642f6d6f 642e7265
6c004552 524f523a
206d6f64 2e72656c
20776173 206e6f74
20666f75 6e640045
52524f52 3a206661
696c6564 20746f20
6c6f6164 206d6f64
2e72656c 00ffffff
ff000000 ff000000
7fe802a6 807f0024
88830009 2c040000
41820138 88830008
2c040000 4082012c
387f0028 801f0004
7c0803a6 4e800021
2c03ffff 40a20020
387f0071 389f0075
38bf0036 801f0018
7c0803a6 4e800021
480000f8 801f0000
901f009c 387f009c
7c0018ac 7c001fac
387f0028 38800000
38a00000 801f0010
7c0803a6 4e800021
2c030000 41a200c4
387f0028 38800000
38a00000 801f000c
7c0803a6 4e800021
2c03ffff 40a20020
387f0071 389f0075
38bf0053 801f0018
7c0803a6 4e800021
48000088 7c7e1b78
83a300a4 38600000
809d0004 801f0020
7c0803a6 4e800021
7c7c1b78 809d0000
80bd0004 801f0008
7c0803a6 4e800021
7fc3f378 801f0014
7c0803a6 4e800021
38600000 809c0020
801f0020 7c0803a6
4e800021 7c641b78
7f83e378 801f001c
7c0803a6 4e800021
801c0034 7c0803a6
4e800021 807f0024
38000001 98030008
bb810008 38210018
80010004 7c0803a6
4e800020 00000000
```
