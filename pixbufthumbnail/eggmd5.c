/* Egg Libraries: eggmd5.c
 * 
 * Copyright (C) 2004 James M. Cape <jcape@ignore-your.tv>
 * Copyright (C) 2002 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */

#include <string.h>

#include "eggmd5.h"


typedef struct _Md5Context Md5Context;

static void get_md5	  (const gchar	*str,
			   guchar	 digest[16]);
static void md5_init      (Md5Context   *context);
static void md5_update    (Md5Context   *context,
			   guchar const *buf,
			   guint         len);
static void md5_final     (guchar        digest[16],
			   Md5Context   *context);
static void md5_transform (guint32       buf[4],
			   guint32 const in[16]);


/* ************************************************************************** *
 *  PUBLIC API                                                                *
 * ************************************************************************** */

GType
egg_md5_digest_get_type (void)
{
  GType type = G_TYPE_INVALID;
	
  if (type == G_TYPE_INVALID)
    {
      type = g_boxed_type_register_static ("EggMd5Digest",
					   (GBoxedCopyFunc) egg_md5_digest_dup,
					   g_free);
    }

  return type;
}


/**
 * egg_str_get_md5_digest:
 * @str: the string to create a digest of.
 * 
 * Creates a binary MD5 digest of the contents of @str.
 * 
 * Returns: a new binary MD5 digest. It should be freed with g_free() when no
 *  longer needed.
 * 
 * Since: 2.6
 **/
EggMd5Digest *
egg_str_get_md5_digest (const gchar *str)
{
  EggMd5Digest *retval;

  g_return_val_if_fail (str != NULL, NULL);

  retval = g_new (EggMd5Digest, 1);

  get_md5 (str, retval->digest);

  return retval;
}


/**
 * egg_str_get_md5_str:
 * @str: the string to create a digest of.
 * 
 * Creates an character array MD5 digest of the contents of @str.
 * 
 * Returns: a newly-allocated character array which should be freed with
 *  g_free() when no longer needed.
 * 
 * Since: 2.6
 **/
gchar *
egg_str_get_md5_str (const gchar *str)
{
  EggMd5Digest digest;

  g_return_val_if_fail (str != NULL, NULL);

  get_md5 (str, digest.digest);

  return egg_md5_digest_to_str (&digest);
}


/**
 * egg_md5_digest_to_str:
 * @digest: the binary MD5 digest to convert.
 * 
 * Converts the binary @digest to an ASCII character array digest. The result
 * can be used as an ordinary C string.
 * 
 * Returns: a newly-allocated character array which should be freed with
 *  g_free() when no longer needed.
 *
 * Since: 2.6
 **/
gchar *
egg_md5_digest_to_str (const EggMd5Digest *digest)
{
  static gchar hex_digits[] = "0123456789abcdef";
  guchar *retval;
  gint i;

  g_return_val_if_fail (digest != NULL, NULL);

  retval = g_new (guchar, 33);

  for (i = 0; i < 16; i++)
    {
      retval[2 * i] = hex_digits[digest->digest[i] >> 4];
      retval[2 * i + 1] = hex_digits[digest->digest[i] & 0xf];
    }

  retval[32] = 0;

  return retval;
}


/**
 * egg_md5_str_to_digest:
 * @str_digest: the character array digest to convert.
 * 
 * Converts the @str_digest character array digest to a binary digest.
 * 
 * Returns: a newly allocated binary digest. It should be freed with
 *  g_free() when no longer needed.
 *
 * Since: 2.6
 **/
EggMd5Digest *
egg_md5_str_to_digest (const gchar *str_digest)
{
  EggMd5Digest *retval;
  guint i;

  g_return_val_if_fail (str_digest != NULL, NULL);
  g_return_val_if_fail (strlen (str_digest) == 32, NULL);

  retval = g_new (EggMd5Digest, 1);

  for (i = 0; i < 16; i++)
    {
      retval->digest[i] =
        g_ascii_xdigit_value (str_digest[2 * i]) << 4 |
        g_ascii_xdigit_value (str_digest[2 * i + 1]);
    }

  return retval;
}


/**
 * egg_md5_digest_dup:
 * @src: the digest to copy.
 * 
 * Duplicates the contents of the @src binary MD5 digest.
 * 
 * Returns: a new binary MD5 digest. It should be freed with g_free() when no
 *  longer needed.
 * 
 * Since: 2.6
 **/
EggMd5Digest *
egg_md5_digest_dup (const EggMd5Digest *src)
{
  return g_memdup (src, sizeof (EggMd5Digest));
}


/**
 * egg_md5_digest_hash:
 * @digest: the #EggMd5Digest to hash.
 * 
 * Gets the numeric hash of @digest, for use in #GHashTable and #GCache.
 * 
 * Returns: an unsigned integer hash of the digest.
 * 
 * Since: 2.6
 **/
guint
egg_md5_digest_hash (gconstpointer digest)
{
  return *((guint *) digest);
}


/**
 * egg_md5_digest_equal:
 * @digest1: the first #EggMd5Digest to compare.
 * @digest2: the second #EggMd5Digest to compare.
 * 
 * Tests the equality of @digest1 and @digest2, useful for #GHashTable and
 * #GCashe.
 * 
 * Returns: %TRUE if both digests are equal, %FALSE otherwise.
 * 
 * Since: 1.0
 **/
gboolean
egg_md5_digest_equal (gconstpointer digest1, gconstpointer digest2)
{
  guint *d1, *d2;
  guint i;

  /* Both NULL or same digest */
  if (digest1 == digest2)
    return TRUE;

  /* One is NULL and the other isn't */
  if (digest1 == NULL || digest2 == NULL)
    return FALSE;

  d1 = (guint *) digest1;
  d2 = (guint *) digest2;

  for (i = 0; i < (16 / sizeof (guint)); i++)
    {
      if (*d1 != *d2)
	return FALSE;

      d1 += i;
      d2 += i;
    }

  return TRUE;
}


/* ************************************************************************** *
 *  MD5 Digests                                                               *
 * ************************************************************************** */

/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * ThumbMD5Context structure, pass it to thumb_md5_init, call
 * thumb_md5_update as needed on buffers full of bytes, and then call
 * thumb_md5_final, which will fill a supplied 32-byte array with the
 * digest in ascii form. 
 *
 */

struct _Md5Context
{
  guint32 buf[4];
  guint32 bits[2];
  guchar in[64];
};


static void
get_md5 (const char *string, unsigned char digest[16])
{
  Md5Context md5_context;
  
  md5_init (&md5_context);
  md5_update (&md5_context, string, strlen (string));
  md5_final (digest, &md5_context);
}

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
#define byteReverse(buf, len)	/* Nothing */
#else

/*
 * Note: this code is harmless on little-endian machines.
 */
static void
byteReverse(unsigned char *buf, unsigned longs)
{
    guint32 t;
    do {
	t = (guint32) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
	    ((unsigned) buf[1] << 8 | buf[0]);
	*(guint32 *) buf = t;
	buf += 4;
    } while (--longs);
}

#endif

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
static void 
md5_init (Md5Context *ctx)
{
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;

    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
static void 
md5_update (Md5Context   *ctx,
	    unsigned char const *buf,
	    unsigned	  len)
{
    guint32 t;

    /* Update bitcount */

    t = ctx->bits[0];
    if ((ctx->bits[0] = t + ((guint32) len << 3)) < t)
	ctx->bits[1]++;		/* Carry from low to high */
    ctx->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */

    if (t) {
	unsigned char *p = (unsigned char *) ctx->in + t;

	t = 64 - t;
	if (len < t) {
	    memcpy (p, buf, len);
	    return;
	}
	memcpy (p, buf, t);
	byteReverse (ctx->in, 16);
	md5_transform (ctx->buf, (guint32 *) ctx->in);
	buf += t;
	len -= t;
    }

    /* Process data in 64-byte chunks */

    while (len >= 64) {
	memcpy (ctx->in, buf, 64);
	byteReverse (ctx->in, 16);
	md5_transform (ctx->buf, (guint32 *) ctx->in);
	buf += 64;
	len -= 64;
    }

    /* Handle any remaining bytes of data. */

    memcpy(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern 
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
static void 
md5_final (unsigned char digest[16],
	   Md5Context *ctx)
{
    unsigned count;
    unsigned char *p;

    /* Compute number of bytes mod 64 */
    count = (ctx->bits[0] >> 3) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    p = ctx->in + count;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    count = 64 - 1 - count;

    /* Pad out to 56 mod 64 */
    if (count < 8) {
	/* Two lots of padding:  Pad the first block to 64 bytes */
	memset (p, 0, count);
	byteReverse (ctx->in, 16);
	md5_transform (ctx->buf, (guint32 *) ctx->in);

	/* Now fill the next block with 56 bytes */
	memset(ctx->in, 0, 56);
    } else {
	/* Pad block to 56 bytes */
	memset(p, 0, count - 8);
    }
    byteReverse(ctx->in, 14);

    /* Append length in bits and transform */
    ((guint32 *) ctx->in)[14] = ctx->bits[0];
    ((guint32 *) ctx->in)[15] = ctx->bits[1];

    md5_transform (ctx->buf, (guint32 *) ctx->in);
    byteReverse ((unsigned char *) ctx->buf, 4);
    memcpy (digest, ctx->buf, 16);
    memset (ctx, 0, sizeof(ctx));	/* In case it's sensitive */
}


/* The four core functions - F1 is optimized somewhat */

#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1 (z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define md5_step(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  Md5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
static void 
md5_transform (guint32	   buf[4],
	       guint32 const in[16])
{
    register guint32 a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    md5_step(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    md5_step(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    md5_step(F1, c, d, a, b, in[2] + 0x242070db, 17);
    md5_step(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    md5_step(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    md5_step(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    md5_step(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    md5_step(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    md5_step(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    md5_step(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    md5_step(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    md5_step(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    md5_step(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    md5_step(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    md5_step(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    md5_step(F1, b, c, d, a, in[15] + 0x49b40821, 22);
		
    md5_step(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    md5_step(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    md5_step(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    md5_step(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    md5_step(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    md5_step(F2, d, a, b, c, in[10] + 0x02441453, 9);
    md5_step(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    md5_step(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    md5_step(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    md5_step(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    md5_step(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    md5_step(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    md5_step(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    md5_step(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    md5_step(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    md5_step(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);
		
    md5_step(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    md5_step(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    md5_step(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    md5_step(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    md5_step(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    md5_step(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    md5_step(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    md5_step(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    md5_step(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    md5_step(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    md5_step(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    md5_step(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    md5_step(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    md5_step(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    md5_step(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    md5_step(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);
		
    md5_step(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    md5_step(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    md5_step(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    md5_step(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    md5_step(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    md5_step(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    md5_step(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    md5_step(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    md5_step(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    md5_step(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    md5_step(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    md5_step(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    md5_step(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    md5_step(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    md5_step(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    md5_step(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}
