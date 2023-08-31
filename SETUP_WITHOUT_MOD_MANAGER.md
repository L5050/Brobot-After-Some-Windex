# Alternative setup guide

## Instructions

1. Find out which version of Super Paper Mario you have.
2. Download the [mod](https://github.com/L5050/Brobot-After-Some-Windex/releases/tag/V1.0.5), choose the download that matches your SPM version.
3. Download and install [Wiimms](https://wit.wiimm.de/download.html).
4. Open the Command Prompt in the same folder as your SPM ISO.
5. Type `wit x spm.iso extracted` and press Enter.
6. Once that's done, copy all the files from the mod EXCEPT `modinfo.ini` into the `extracted/files` folder. When Windows asks if you want to replace files, say Yes.
7. In the command prompt, type `wit copy --align-files extracted Brobot.wbfs`
8. The mod should now appear in Dolphin (as Super Paper Mario). Right click the mod and select Properties, then under the Gecko Codes tab click `Add New Code...` and paste in the gecko code for your version from [here](https://github.com/SeekyCt/spm-rel-loader/tree/master/spm-rel-loader/loader).
9. Download [this save file](https://github.com/L5050/Brobot-After-Some-Windex/releases/tag/save). which will automatically place you at the point in the game where the changes were made.
10. Right click Super Paper Mario in Dolphin and select `Open Wii Save Folder`, then place the downloaded save file in there.

Warning: This will overwrite the save file on the bottom left. If you want the save to use a different file, simply change the number at the end of the file name. (0 = top left, 1 = top right, 2 = bottom left, 3 = bottom right)
