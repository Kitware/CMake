# $FreeBSD: src/usr.bin/tar/Makefile,v 1.40 2008/12/06 07:38:14 kientzle Exp $

.PATH: ${.CURDIR}/../libarchive_fe

PROG=   bsdtar
BSDTAR_VERSION_STRING=2.7.900a
SRCS=   bsdtar.c cmdline.c getdate.c read.c subst.c tree.c util.c write.c
SRCS+=  err.c line_reader.c matching.c pathmatch.c
WARNS?= 5
DPADD=  ${LIBARCHIVE} ${LIBBZ2} ${LIBZ}
LDADD=  -larchive -lbz2 -lz -lmd -lcrypto
CFLAGS+=    -DBSDTAR_VERSION_STRING=\"${BSDTAR_VERSION_STRING}\"
CFLAGS+=    -DPLATFORM_CONFIG_H=\"config_freebsd.h\"
CFLAGS+=    -I"${.CURDIR}" -I"${.CURDIR}/../libarchive_fe"
SYMLINKS=   bsdtar ${BINDIR}/tar
MLINKS= bsdtar.1 tar.1
DEBUG_FLAGS=-g

.PHONY: check test
check test: $(PROG) bsdtar.1.gz
    cd ${.CURDIR}/test && make test

.include <bsd.prog.mk>
