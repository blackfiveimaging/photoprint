#include <string.h>

#include "rotatepixbuf.h"

/* HACK * HACK * HACK
   This function was pulled in from GdkPixBuf CVS as a stop-gap, since the
   rotate function within won't get widespread exposure until Gtk+-2.6 is released.
   The function has been adjusted to cope with _GdkPixbuf being opaque - direct
   accesses have been replaced with the appropriate accessor functions.
   
   If GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE is defined, then the built-in
   function will be used instead of this one.  Probably a year down the
   line, this can be removed and no-one will notice.
*/

/**
 * gdk_pixbuf_rotate_simple:
 * @src: a #GdkPixbuf
 * @angle: the angle to rotate by
 *
 * Rotates a pixbuf by a multiple of 90 degrees, and returns the
 * result in a new pixbuf.
 *
 * Returns: a new pixbuf
 *
 * Since: 2.6
 */
 
#define OFFSET(pb, x, y) ((x) * (gdk_pixbuf_get_n_channels(pb)) + (y) * gdk_pixbuf_get_rowstride(pb))
 
GdkPixbuf *
pv_gdk_pixbuf_rotate_simple (const GdkPixbuf   *src,
			  GdkPixbufRotation angle)
{
  GdkPixbuf *dest;
  guchar *p, *q;
  gint x, y;
  int srcwidth=gdk_pixbuf_get_width(src);
  int srcheight=gdk_pixbuf_get_height(src);

  switch (angle % 360)
    {
    case 0:
      dest = gdk_pixbuf_copy (src);
      break;
    case 90:
      dest = gdk_pixbuf_new (gdk_pixbuf_get_colorspace(src), 
			     gdk_pixbuf_get_has_alpha(src),
			     gdk_pixbuf_get_bits_per_sample(src), 
			     srcheight,srcwidth);
      if (!dest)
	return NULL;

      for (y = 0; y < srcheight; y++) 
	{ 
	  for (x = 0; x < srcwidth; x++) 
	    { 
	      p = gdk_pixbuf_get_pixels(src) + OFFSET (src, x, y); 
	      q = gdk_pixbuf_get_pixels(dest) + OFFSET (dest, y, srcwidth - x - 1); 
	      memcpy (q, p, gdk_pixbuf_get_n_channels(dest));
	    }
	} 
      break;
    case 180:
      dest = gdk_pixbuf_new (gdk_pixbuf_get_colorspace(src), 
			     gdk_pixbuf_get_has_alpha(src),
			     gdk_pixbuf_get_bits_per_sample(src), 
			     srcwidth,srcheight);
      if (!dest)
	return NULL;

      for (y = 0; y < srcheight; y++) 
	{ 
	  for (x = 0; x < srcwidth; x++) 
	    { 
	      p = gdk_pixbuf_get_pixels(src) + OFFSET (src, x, y); 
	      q = gdk_pixbuf_get_pixels(dest) + OFFSET (dest, srcwidth - x - 1, srcheight - y - 1); 
	      memcpy (q, p, gdk_pixbuf_get_n_channels(dest));
	    }
	} 
      break;
    case 270:
      dest = gdk_pixbuf_new (gdk_pixbuf_get_colorspace(src), 
			     gdk_pixbuf_get_has_alpha(src),
			     gdk_pixbuf_get_bits_per_sample(src), 
			     srcheight,srcwidth);
      if (!dest)
	return NULL;

      for (y = 0; y < srcheight; y++) 
	{ 
	  for (x = 0; x < srcwidth; x++) 
	    { 
	      p = gdk_pixbuf_get_pixels(src) + OFFSET (src, x, y); 
	      q = gdk_pixbuf_get_pixels(dest) + OFFSET (dest, srcheight - y - 1, x); 
	      memcpy (q, p, gdk_pixbuf_get_n_channels(dest));
	    }
	} 
      break;
    default:
      dest = NULL;
      g_warning ("gdk_pixbuf_rotate_simple() can only rotate "
		 "by multiples of 90 degrees");
      g_assert_not_reached ();
  } 

  return dest;
}

/* BORROWED FUNCTION ENDS */
