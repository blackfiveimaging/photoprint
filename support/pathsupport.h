#ifndef PATHSUPPORT_H
#define PATHSUPPORT_H

// Wrapper for either getenv("HOME") or g_get_home_dir().

const char *get_homedir();


// substitue_homedir(const char *path)
//
// Accepts a source path argument.  If the source path begins with $HOME" or "~"
// then the user's home directory is substituted.
// Whether or not the directory is substituted, the returned value is a new string
// which may be free()d when finished with.

char *substitute_homedir(const char *path);

#endif
