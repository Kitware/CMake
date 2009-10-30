# $FreeBSD: src/usr.bin/cpio/Makefile,v 1.6 2008/12/06 07:30:40 kientzle Exp $

.include <bsd.own.mk>
.PATH: ${.CURDIR}/../libarchive_fe

PROG=   bsdcpio
BSDCPIO_VERSION_STRING=2.7.900a
SRCS=   cpio.c cmdline.c err.c line_reader.c matching.c pathmatch.c
WARNS?= 6
DPADD=  ${LIBARCHIVE} ${LIBZ} ${LIBBZ2}
CFLAGS+= -DBSDCPIO_VERSION_STRING=\"${BSDCPIO_VERSION_STRING}\"
CFLAGS+= -DPLATFORM_CONFIG_H=\"config_freebsd.h\"
CFLAGS+= -I../libarchive_fe -I${.CURDIR}
LDADD+= -larchive -lz -lbz2 -lmd -lcrypto

.if ${MK_GNU_CPIO} != "yes"
SYMLINKS=bsdcpio ${BINDIR}/cpio
MLINKS= bsdcpio.1 cpio.1
.endif

.PHONY: check test

check test: $(PROG) bsdcpio.1.gz
    cd ${.CURDIR}/test && make clean test

.include <bsd.prog.mk>
