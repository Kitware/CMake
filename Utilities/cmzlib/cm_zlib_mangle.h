#ifndef cm_zlib_mangle_h
#define cm_zlib_mangle_h

/* Mangle symbols like Z_PREFIX, but with a different prefix.  */
#define _dist_code            cm_zlib__dist_code
#define _length_code          cm_zlib__length_code
#define _tr_align             cm_zlib__tr_align
#define _tr_flush_bits        cm_zlib__tr_flush_bits
#define _tr_flush_block       cm_zlib__tr_flush_block
#define _tr_init              cm_zlib__tr_init
#define _tr_stored_block      cm_zlib__tr_stored_block
#define _tr_tally             cm_zlib__tr_tally
#define adler32               cm_zlib_adler32
#define adler32_combine       cm_zlib_adler32_combine
#define adler32_combine64     cm_zlib_adler32_combine64
#define adler32_z             cm_zlib_adler32_z
#ifndef Z_SOLO
#  define compress              cm_zlib_compress
#  define compress2             cm_zlib_compress2
#  define compressBound         cm_zlib_compressBound
#endif
#define crc32                 z_crc32
#define crc32_combine         cm_zlib_crc32_combine
#define crc32_combine64       cm_zlib_crc32_combine64
#define crc32_z               cm_zlib_crc32_z
#define deflate               cm_zlib_deflate
#define deflateBound          cm_zlib_deflateBound
#define deflateCopy           cm_zlib_deflateCopy
#define deflateEnd            cm_zlib_deflateEnd
#define deflateGetDictionary  cm_zlib_deflateGetDictionary
#define deflateInit           cm_zlib_deflateInit
#define deflateInit2          cm_zlib_deflateInit2
#define deflateInit2_         cm_zlib_deflateInit2_
#define deflateInit_          cm_zlib_deflateInit_
#define deflateParams         cm_zlib_deflateParams
#define deflatePending        cm_zlib_deflatePending
#define deflatePrime          cm_zlib_deflatePrime
#define deflateReset          cm_zlib_deflateReset
#define deflateResetKeep      cm_zlib_deflateResetKeep
#define deflateSetDictionary  cm_zlib_deflateSetDictionary
#define deflateSetHeader      cm_zlib_deflateSetHeader
#define deflateTune           cm_zlib_deflateTune
#define deflate_copyright     cm_zlib_deflate_copyright
#define get_crc_table         cm_zlib_get_crc_table
#ifndef Z_SOLO
#  define gcm_zlib_error              cm_zlib_gcm_zlib_error
#  define gcm_zlib_intmax             cm_zlib_gcm_zlib_intmax
#  define gcm_zlib_strwinerror        cm_zlib_gcm_zlib_strwinerror
#  define gzbuffer              cm_zlib_gzbuffer
#  define gzclearerr            cm_zlib_gzclearerr
#  define gzclose               cm_zlib_gzclose
#  define gzclose_r             cm_zlib_gzclose_r
#  define gzclose_w             cm_zlib_gzclose_w
#  define gzdirect              cm_zlib_gzdirect
#  define gzdopen               cm_zlib_gzdopen
#  define gzeof                 cm_zlib_gzeof
#  define gzerror               cm_zlib_gzerror
#  define gzflush               cm_zlib_gzflush
#  define gzfread               cm_zlib_gzfread
#  define gzfwrite              cm_zlib_gzfwrite
#  define gzgetc                cm_zlib_gzgetc
#  define gzgetc_               cm_zlib_gzgetc_
#  define gzgets                cm_zlib_gzgets
#  define gzoffset              cm_zlib_gzoffset
#  define gzoffset64            cm_zlib_gzoffset64
#  define gzopen                cm_zlib_gzopen
#  define gzopen64              cm_zlib_gzopen64
#  ifdef _WIN32
#    define gzopen_w              cm_zlib_gzopen_w
#  endif
#  define gzprintf              cm_zlib_gzprintf
#  define gzputc                cm_zlib_gzputc
#  define gzputs                cm_zlib_gzputs
#  define gzread                cm_zlib_gzread
#  define gzrewind              cm_zlib_gzrewind
#  define gzseek                cm_zlib_gzseek
#  define gzseek64              cm_zlib_gzseek64
#  define gzsetparams           cm_zlib_gzsetparams
#  define gztell                cm_zlib_gztell
#  define gztell64              cm_zlib_gztell64
#  define gzungetc              cm_zlib_gzungetc
#  define gzvprintf             cm_zlib_gzvprintf
#  define gzwrite               cm_zlib_gzwrite
#endif
#define inflate               cm_zlib_inflate
#define inflateBack           cm_zlib_inflateBack
#define inflateBackEnd        cm_zlib_inflateBackEnd
#define inflateBackInit       cm_zlib_inflateBackInit
#define inflateBackInit_      cm_zlib_inflateBackInit_
#define inflateCodesUsed      cm_zlib_inflateCodesUsed
#define inflateCopy           cm_zlib_inflateCopy
#define inflateEnd            cm_zlib_inflateEnd
#define inflateGetDictionary  cm_zlib_inflateGetDictionary
#define inflateGetHeader      cm_zlib_inflateGetHeader
#define inflateInit           cm_zlib_inflateInit
#define inflateInit2          cm_zlib_inflateInit2
#define inflateInit2_         cm_zlib_inflateInit2_
#define inflateInit_          cm_zlib_inflateInit_
#define inflateMark           cm_zlib_inflateMark
#define inflatePrime          cm_zlib_inflatePrime
#define inflateReset          cm_zlib_inflateReset
#define inflateReset2         cm_zlib_inflateReset2
#define inflateResetKeep      cm_zlib_inflateResetKeep
#define inflateSetDictionary  cm_zlib_inflateSetDictionary
#define inflateSync           cm_zlib_inflateSync
#define inflateSyncPoint      cm_zlib_inflateSyncPoint
#define inflateUndermine      cm_zlib_inflateUndermine
#define inflateValidate       cm_zlib_inflateValidate
#define inflate_copyright     cm_zlib_inflate_copyright
#define inflate_fast          cm_zlib_inflate_fast
#define inflate_table         cm_zlib_inflate_table
#ifndef Z_SOLO
#  define uncompress            cm_zlib_uncompress
#  define uncompress2           cm_zlib_uncompress2
#endif
#define zError                cm_zlib_zError
#ifndef Z_SOLO
#  define zcalloc               cm_zlib_zcalloc
#  define zcfree                cm_zlib_zcfree
#endif
#define zlibCompileFlags      cm_zlib_zlibCompileFlags
#define zlibVersion           cm_zlib_zlibVersion

/* Mangle symbols not covered by Z_PREFIX.  */
#define crc32_combine_gen64  cm_zlib_crc32_combine_gen64
#define crc32_combine_gen    cm_zlib_crc32_combine_gen
#define crc32_combine_op     cm_zlib_crc32_combine_op
#define gz_error             cm_zlib_gz_error
#define z_crc32              cm_zlib_z_crc32
#define z_errmsg             cm_zlib_z_errmsg

#endif
