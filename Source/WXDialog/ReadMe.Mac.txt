Hi All,

So, here is a short description on how I got installation package on mac osx.

1. Build package.
2. Copy package and related files to the following directory structure:

./Package_Root/Applications/MyApp.app/Contents/Info.plist
./Package_Root/Applications/MyApp.app/Contents/MacOS
./Package_Root/Applications/MyApp.app/Contents/MacOS/MyAppExec
./Package_Root/Applications/MyApp.app/Contents/PkgInfo
./Package_Root/Applications/MyApp.app/Contents/Resources/MyAppIcon.icns
./Package_Root/Applications/MyApp.app/Contents/Resources/MyAppResources.rsrc
./Package_Root/usr/share/somedata
./Package_Root/usr/bin/some_utility
./Resources/License.txt
./Resources/ReadMe.txt
./Resources/Welcome.txt

The file PkgInfo contains "APPL????"  The file Info.plist contains
package instructions. I would suggest looking at Info.plist.in in
wxWindows and modify it by putting your info in.  MyAppIcon.icns is an
icon created by IconComposer in /Developmer/Applications.
MyAppResources.rsrc is a compiled resources file. I used the one from
wxWindows and it seems to be working fine. Some mac person could
explain this. Make sure that Info.plist contains:

        <key>CFBundleIconFile</key>
        <string>MyAppIcon.icns</string>

This will tell it which icon to use.

The ./Package_Root/usr/share/somedata and
./Package_Root/usr/bin/some_utility are some extra data that your
application is using. For example they can be a command line version
of the application and some common files. 

The ./Resources directory contains resources used during
installation. The names of files describe what should go in. They can
be rtf, txt...

3. Create package using PackageMaker. 

Fire up PackageMaker from /Developmer/Applications. Fill all the
entries. Most of them are straight forward. Package root is
./Package_Root. Resources directory is ./Resources. If your
application needs to write to /usr (or to some other place beside
/Applications, then make sure to enable authentication. After you fill
all the entries, create package. This will create a directory on your
disk containing a whole lot of junk. Now you need to package this
directory.

4. Create a disk image.

Run Disk Copy from /Applications/Utilities. The easiest way of
creating disk image is to first create empty folder, copy all the
files in and then tell Disk Copy to create disk image from folder. So,
create folder MyApp, copy in MyApp.pkg and create package.

Now you are done. This will create one file, which you can copy around.

So, what is missing is how to make application icon show in the
finder. I can make it show when running application, but not when I am
looking at application. Even if I ask for preview.

I guess somebody else will have to answer this.

Good luck.

                        Andy

