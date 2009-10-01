/*
 * "$Id: file.c,v 1.1.1.1.2.11 2009/10/01 15:02:40 bsavelev Exp $"
 *
 *   File functions for the ESP Package Manager (EPM).
 *
 *   Copyright 1999-2005 by Easy Software Products.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 * Contents:
 *
 *   copy_file()      - Copy a file...
 *   make_directory() - Make a directory.
 *   make_link()      - Make a symbolic link.
 *   strip_execs()    - Strip symbols from executable files in the
 *                      distribution.
 *   unlink_package() - Delete a package file.
 */

/*
 * Include necessary headers...
 */

#include "epm.h"


/*
 * 'copy_file()' - Copy a file...
 */

int					/* O - 0 on success, -1 on failure */
copy_file(const char *dst,		/* I - Destination file */
          const char *src,		/* I - Source file */
          mode_t     mode,		/* I - Permissions */
	  uid_t      owner,		/* I - Owner ID */
	  gid_t      group)		/* I - Group ID */
{
  FILE		*dstfile,		/* Destination file */
		*srcfile;		/* Source file */
  char		buffer[8192];		/* Copy buffer */
  char		*slash;			/* Pointer to trailing slash */
  size_t	bytes;			/* Number of bytes read/written */


 /*
  * Check that the destination directory exists...
  */

  strcpy(buffer, dst);
  if ((slash = strrchr(buffer, '/')) != NULL)
    *slash = '\0';

  if (access(buffer, F_OK))
    make_directory(buffer, 0755, owner, group);

 /*
  * Open files...
  */

  unlink(dst);

  if ((dstfile = fopen(dst, "wb")) == NULL)
  {
    fprintf(stderr, "epm: Unable to create \"%s\" -\n     %s\n", dst,
            strerror(errno));
    return (-1);
  }

  if ((srcfile = fopen(src, "rb")) == NULL)
  {
    fclose(dstfile);
    unlink(dst);

    fprintf(stderr, "epm: Unable to open \"%s\" -\n     %s\n", src,
            strerror(errno));
    return (-1);
  }

 /*
  * Copy from src to dst...
  */

  while ((bytes = fread(buffer, 1, sizeof(buffer), srcfile)) > 0)
    if (fwrite(buffer, 1, bytes, dstfile) < bytes)
    {
      fprintf(stderr, "epm: Unable to write to \"%s\" -\n     %s\n", dst,
              strerror(errno));

      fclose(srcfile);
      fclose(dstfile);
      unlink(dst);

      return (-1);
    }

 /*
  * Close files, change permissions, and return...
  */

  fclose(srcfile);
  fclose(dstfile);

  chmod(dst, mode);
  chown(dst, owner, group);

  return (0);
}


/*
 * 'make_directory()' - Make a directory.
 */

int					/* O - 0 = success, -1 = error */
make_directory(const char *directory,	/* I - Directory */
               mode_t     mode,		/* I - Permissions */
	       uid_t      owner,	/* I - Owner ID */
	       gid_t      group)	/* I - Group ID */
{
  char	buffer[8192],			/* Filename buffer */
	*bufptr;			/* Pointer into buffer */


  for (bufptr = buffer; *directory;)
  {
    if (*directory == '/' && bufptr > buffer)
    {
      *bufptr = '\0';

      if (access(buffer, F_OK))
      {
	mkdir(buffer, 0777);
	chmod(buffer, mode | 0700);
	chown(buffer, owner, group);
      }
    }

    *bufptr++ = *directory++;
  }

  *bufptr = '\0';

  if (access(buffer, F_OK))
  {
    mkdir(buffer, 0777);
    chmod(buffer, mode | 0700);
    chown(buffer, owner, group);
  }

  return (0);
}


/*
 * 'make_link()' - Make a symbolic link.
 */

int					/* O - 0 = success, -1 = error */
make_link(const char *dst,		/* I - Destination file */
          const char *src)		/* I - Link */
{
  char	buffer[8192],			/* Copy buffer */
	*slash;				/* Pointer to trailing slash */


 /*
  * Check that the destination directory exists...
  */

  strcpy(buffer, dst);
  if ((slash = strrchr(buffer, '/')) != NULL)
    *slash = '\0';

  if (access(buffer, F_OK))
    make_directory(buffer, 0755, 0, 0);

 /* 
  * Make the symlink...
  */

  return (symlink(src, dst));
}


/*
 * 'strip_execs()' - Strip symbols from executable files in the distribution.
 */

void
strip_execs(dist_t *dist)		/* I - Distribution to strip... */
{
  int		i;			/* Looping var */
  file_t	*file;			/* Software file */
  FILE		*fp;			/* File pointer */
  char		header[4];		/* File header... */
  char		*dir_name,
		*file_name;
  dist_t	*dist_tmp;
  const char	*subpkg = "debug";
  const char delim[32] = "/\0";
  char *subpkg_name;


//create temp dist for debug package files
  if (DebugPackage)
    dist_tmp=new_dist();
 /*
  * Loop through the distribution files and strip any executable
  * files.
  */

  for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
    if (tolower(file->type) == 'f' &&
        strstr(file->options, "nostrip()") == NULL)
    {
     /*
      * OK, this file has executable permissions; see if it is a
      * script...
      */

      if ((fp = fopen(file->src, "rb")) != NULL)
      {
       /*
        * Read the first 3 bytes of the file...
	*/

        if (fread(header, 1, sizeof(header) - 1, fp) == 0)
	  continue;

	header[sizeof(header) - 1] = '\0';

	fclose(fp);

       /*
        * Check for "#!/" or "#" + whitespace at the beginning of the file...
	*/

        if (!strncmp(header, "#!/", 3) ||
	    (header[0] == '#' && isspace(header[1] & 255)))
	  continue;
      }
      else
      {
       /*
        * File could not be opened; error out...
	*/

        fprintf(stderr, "epm: Unable to open file \"%s\" for destination "
	                "\"%s\" -\n     %s\n",
	        file->src, file->dst, strerror(errno));

        exit(1);
      }

     /*
      * Strip executables...
      */

     int res=0;
     res = run_command(NULL, "/bin/sh -c '/usr/bin/file %s | egrep \"ELF.*not stripped\"'", file->src);
     if (!res)
     {
      if (Verbosity > 1)
       fprintf(stdout, "Copying %s\n",file->src);

       //copy file before strip
      char debug_src[512] = "\0";
      strcat(debug_src,TempDir);
      strcat(debug_src,delim);
      strcat(debug_src,file->src);
      dir_name = strdup(debug_src);
      run_command(NULL, "mkdir -p %s", dirname(dir_name));
      run_command(NULL, "cp %s %s", file->src,debug_src);
      strcpy(file->src,debug_src);

      if (Verbosity > 1)
       fprintf(stdout, "Stripping %s\n",file->src);

      if (DebugPackage)
      {
	char appendix[255]=".debug";
	char debug_dst[512] = "\0";
	strcat(debug_dst,file->src);
	strcat(debug_dst,appendix);
	res = run_command(NULL, "objcopy --only-keep-debug %s %s", file->src, debug_dst);
       if (!res) {
	run_command(NULL, EPM_STRIP " %s", file->src);
	run_command(NULL, "objcopy --add-gnu-debuglink=%s %s ", debug_dst, file->src);
	//subpackage
	char dst[1024] = "\0";
	dir_name = strdup(file->dst);
	strcat(dst,dirname(dir_name));
	strcat(dst,delim);
	file_name = strdup(file->src);
	strcat(dst,strdup(basename(file_name)));
	strcat(dst,appendix);
	subpkg_name=find_subpackage(dist_tmp, subpkg);
	file_t *new_file = add_file(dist_tmp, subpkg_name);
	new_file->type = file->type;
	new_file->mode = file->mode;
	strcpy(new_file->src, debug_dst);
	strcpy(new_file->dst, dst);
	strcpy(new_file->user, file->user);
	strcpy(new_file->group, file->group);
	strcpy(new_file->options, file->options);
       }
      }
      else
      {
	run_command(NULL, EPM_STRIP " %s", file->src);
      }
     }
    }
  if (DebugPackage)
    for (i = dist_tmp->num_files, file = dist_tmp->files; i > 0; i --, file ++)
    {
	subpkg_name=find_subpackage(dist, subpkg);
	file_t *new_file = add_file(dist, subpkg_name);
	new_file->type = file->type;
	new_file->mode = file->mode;
	strcpy(new_file->src, file->src);
	strcpy(new_file->dst, file->dst);
	strcpy(new_file->user, file->user);
	strcpy(new_file->group, file->group);
	strcpy(new_file->options, file->options);	
    }
}


/*
 * 'unlink_package()' - Delete a package file.
 */

int					/* O - 0 on success, -1 on failure */
unlink_package(const char *ext,		/* I - Package filename extension */
               const char *prodname,	/* I - Product short name */
	       const char *directory,	/* I - Directory for distribution files */
               const char *platname,	/* I - Platform name */
               dist_t     *dist,	/* I - Distribution information */
	       const char *subpackage)	/* I - Subpackage */
{
  char		prodfull[255],		/* Full name of product */
		name[1024],		/* Full product name */
		filename[1024];		/* File to archive */


 /*
  * Figure out the full name of the distribution...
  */

  if (subpackage)
    snprintf(prodfull, sizeof(prodfull), "%s-%s", prodname, subpackage);
  else
    strlcpy(prodfull, prodname, sizeof(prodfull));

 /*
  * Then the subdirectory name...
  */

if (!strcmp(ext,"rpm")) {
  if (dist->release[0])
    snprintf(name, sizeof(name), "%s-%s-%s", prodfull, dist->version,
             dist->release);
  else
    snprintf(name, sizeof(name), "%s-%s", prodfull, dist->version);

  if (platname[0])
  {
    strlcat(name, ".", sizeof(name));
    strlcat(name, platname, sizeof(name));
  }

  strlcat(name, ".", sizeof(name));
  strlcat(name, ext, sizeof(name));
} else if (!strcmp(ext,"deb")) {
  if (dist->release[0])
    snprintf(name, sizeof(name), "%s_%s-%s", prodfull, dist->version,
             dist->release);
  else
    snprintf(name, sizeof(name), "%s_%s", prodfull, dist->version);

  if (platname[0])
  {
    strlcat(name, "_", sizeof(name));
    strlcat(name, platname, sizeof(name));
  }

  strlcat(name, ".", sizeof(name));
  strlcat(name, ext, sizeof(name));
} else {
  if (dist->release[0])
    snprintf(name, sizeof(name), "%s-%s-%s", prodfull, dist->version,
             dist->release);
  else
    snprintf(name, sizeof(name), "%s-%s", prodfull, dist->version);

  if (platname[0])
  {
    strlcat(name, "-", sizeof(name));
    strlcat(name, platname, sizeof(name));
  }

  strlcat(name, ".", sizeof(name));
  strlcat(name, ext, sizeof(name));
}

 /*
  * Remove the package file...
  */

  snprintf(filename, sizeof(filename), "%s/%s", directory, name);

  if (unlink(filename))
  {
    fprintf(stderr, "epm: Unable to remove \"%s\" - %s\n", filename,
            strerror(errno));
    return (-1);
  }
  else
    return (0);
}


/*
 * End of "$Id: file.c,v 1.1.1.1.2.11 2009/10/01 15:02:40 bsavelev Exp $".
 */
