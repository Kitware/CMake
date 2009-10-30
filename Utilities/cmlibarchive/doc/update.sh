#!/bin/sh

#
# Simple script to repopulate the 'doc' tree from
# the mdoc man pages stored in each project.
#

# Build Makefile in 'man' directory
cd man
rm -f *.[135]
echo > Makefile
echo "default: all" >>Makefile
echo >>Makefile
all="all:"
for d in libarchive tar cpio; do
    for f in ../../$d/*.[135]; do
    outname="`basename $f`"
    echo >> Makefile
    echo $outname: ../mdoc2man.awk $f >> Makefile
    echo "  awk -f ../mdoc2man.awk < $f > $outname" >> Makefile
        all="$all $outname"
    done
done
echo $all >>Makefile
cd ..

# Rebuild Makefile in 'text' directory
cd text
rm -f *.txt
echo > Makefile
echo "default: all" >>Makefile
echo >>Makefile
all="all:"
for d in libarchive tar cpio; do
    for f in ../../$d/*.[135]; do
    outname="`basename $f`.txt"
    echo >> Makefile
    echo $outname: $f >> Makefile
    echo "  nroff -mdoc $f | col -b > $outname" >> Makefile
        all="$all $outname"
    done
done
echo $all >>Makefile
cd ..

# Rebuild Makefile in 'pdf' directory
cd pdf
rm -f *.pdf
echo > Makefile
echo "default: all" >>Makefile
echo >>Makefile
all="all:"
for d in libarchive tar cpio; do
    for f in ../../$d/*.[135]; do
    outname="`basename $f`.pdf"
    echo >> Makefile
    echo $outname: $f >> Makefile
    echo "  groff -mdoc -T ps $f | ps2pdf - - > $outname" >> Makefile
        all="$all $outname"
    done
done
echo $all >>Makefile
cd ..

# Build Makefile in 'html' directory
cd html
rm -f *.html
echo > Makefile
echo "default: all" >>Makefile
echo >>Makefile
all="all:"
for d in libarchive tar cpio; do
    for f in ../../$d/*.[135]; do
    outname="`basename $f`.html"
    echo >> Makefile
    echo $outname: ../mdoc2man.awk $f >> Makefile
    echo "  groff -mdoc -T html $f > $outname" >> Makefile
        all="$all $outname"
    done
done
echo $all >>Makefile
cd ..

# Build Makefile in 'wiki' directory
cd wiki
rm -f *.wiki
echo > Makefile
echo "default: all" >>Makefile
echo >>Makefile
all="all:"
for d in libarchive tar cpio; do
    for f in ../../$d/*.[135]; do
    outname="`basename $f | awk '{ac=split($0,a,"[_.-]");o="ManPage";for(w=0;w<=ac;++w){o=o toupper(substr(a[w],1,1)) substr(a[w],2)};print o}'`.wiki"
    echo >> Makefile
    echo $outname: ../mdoc2wiki.awk $f >> Makefile
    echo "  awk -f ../mdoc2wiki.awk < $f > $outname" >> Makefile
        all="$all $outname"
    done
done
echo $all >>Makefile
cd ..

# Convert all of the manpages to -man format
(cd man && make)
# Format all of the manpages to text
(cd text && make)
# Format all of the manpages to PDF
(cd pdf && make)
# Format all of the manpages to HTML
(cd html && make)
# Format all of the manpages to Google Wiki syntax
(cd wiki && make)
