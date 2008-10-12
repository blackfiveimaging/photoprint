/* Egg Libraries: eggmd5.h
 * 
 * Copyright (c) 2004 James M. Cape <jcape@ignore-your.tv>
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

#ifndef __EGG_MD5_H__
#define __EGG_MD5_H__ 1

#include <glib-object.h>

G_BEGIN_DECLS

#define EGG_TYPE_MD5_DIGEST	(egg_md5_digest_get_type ())

typedef struct _EggMd5Digest EggMd5Digest;
struct _EggMd5Digest
{
  guchar  digest[16];
};

GType egg_md5_digest_get_type (void) G_GNUC_CONST;


EggMd5Digest *egg_str_get_md5_digest (const gchar	 *contents);
gchar	     *egg_str_get_md5_str    (const gchar	 *contents);

EggMd5Digest *egg_md5_str_to_digest  (const gchar	 *str_digest);
gchar	     *egg_md5_digest_to_str  (const EggMd5Digest *md5_digest);

EggMd5Digest *egg_md5_digest_dup     (const EggMd5Digest *src);
#define	      egg_md5_digest_free    g_free

guint	      egg_md5_digest_hash   (gconstpointer	  digest);
gboolean      egg_md5_digest_equal  (gconstpointer	  digest1,
				     gconstpointer	  digest2);

G_END_DECLS

#endif /* !__EGG_MD5_H__ */
