#ifndef cm_zlib_mangle_h
#define cm_zlib_mangle_h

/*

This header file mangles all symbols exported from the zlib library.
It is included in all files while building the zlib library.  Due to
namespace pollution, no zlib headers should be included in .h files in
cm.

The following command was used to obtain the symbol list:

nm libcmzlib.a |grep " [TR] "

*/

#define deflate_copyright cm_zlib_deflate_copyright
#define _length_code cm_zlib__length_code
#define _dist_code cm_zlib__dist_code
#define _tr_align cm_zlib__tr_align
#define _tr_flush_block cm_zlib__tr_flush_block
#define _tr_init cm_zlib__tr_init
#define _tr_stored_block cm_zlib__tr_stored_block
#define _tr_tally cm_zlib__tr_tally
#define adler32 cm_zlib_adler32
#define compress cm_zlib_compress
#define compress2 cm_zlib_compress2
#define crc32 cm_zlib_crc32
#define deflate cm_zlib_deflate
#define deflateCopy cm_zlib_deflateCopy
#define deflateEnd cm_zlib_deflateEnd
#define deflateInit2_ cm_zlib_deflateInit2_
#define deflateInit_ cm_zlib_deflateInit_
#define deflateParams cm_zlib_deflateParams
#define deflateReset cm_zlib_deflateReset
#define deflateSetDictionary cm_zlib_deflateSetDictionary
#define get_crc_table cm_zlib_get_crc_table
#define gzclose cm_zlib_gzclose
#define gzdopen cm_zlib_gzdopen
#define gzeof cm_zlib_gzeof
#define gzerror cm_zlib_gzerror
#define gzflush cm_zlib_gzflush
#define gzgetc cm_zlib_gzgetc
#define gzgets cm_zlib_gzgets
#define gzopen cm_zlib_gzopen
#define gzprintf cm_zlib_gzprintf
#define gzputc cm_zlib_gzputc
#define gzputs cm_zlib_gzputs
#define gzread cm_zlib_gzread
#define gzrewind cm_zlib_gzrewind
#define gzseek cm_zlib_gzseek
#define gzsetparams cm_zlib_gzsetparams
#define gztell cm_zlib_gztell
#define gzwrite cm_zlib_gzwrite
#define inflate cm_zlib_inflate
#define inflateEnd cm_zlib_inflateEnd
#define inflateInit2_ cm_zlib_inflateInit2_
#define inflateInit_ cm_zlib_inflateInit_
#define inflateReset cm_zlib_inflateReset
#define inflateSetDictionary cm_zlib_inflateSetDictionary
#define inflateSync cm_zlib_inflateSync
#define inflateSyncPoint cm_zlib_inflateSyncPoint
#define inflate_blocks cm_zlib_inflate_blocks
#define inflate_blocks_free cm_zlib_inflate_blocks_free
#define inflate_blocks_new cm_zlib_inflate_blocks_new
#define inflate_blocks_reset cm_zlib_inflate_blocks_reset
#define inflate_blocks_sync_point cm_zlib_inflate_blocks_sync_point
#define inflate_codes cm_zlib_inflate_codes
#define inflate_codes_free cm_zlib_inflate_codes_free
#define inflate_codes_new cm_zlib_inflate_codes_new
#define inflate_copyright cm_zlib_inflate_copyright
#define inflate_fast cm_zlib_inflate_fast
#define inflate_flush cm_zlib_inflate_flush
#define inflate_mask cm_zlib_inflate_mask
#define inflate_set_dictionary cm_zlib_inflate_set_dictionary
#define inflate_trees_bits cm_zlib_inflate_trees_bits
#define inflate_trees_dynamic cm_zlib_inflate_trees_dynamic
#define inflate_trees_fixed cm_zlib_inflate_trees_fixed
#define uncompress cm_zlib_uncompress
#define zError cm_zlib_zError
#define zcalloc cm_zlib_zcalloc
#define zcfree cm_zlib_zcfree
#define zlibVersion cm_zlib_zlibVersion
#define z_errmsg cm_zlib_z_errmsg

#endif
