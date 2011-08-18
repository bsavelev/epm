/*
 * "$Id: pkg.c,v 1.1.1.1.2.2 2009/05/07 15:49:02 bsavelev Exp $"
 *
 *   AT&T package gateway for the ESP Package Manager (EPM).
 *
 *   Copyright 1999-2008 by Easy Software Products.
 *   Copyright 2009-2010 by Dr.Web.
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
 *   make_pkg() - Make an AT&T software distribution package.
 *   make_subpackage() - Create a subpackage...
 *   pkg_path() - Return an absolute path for the prototype file.
 */

/*
 * Include necessary headers...
 */

#include "epm.h"


/*
 * Local functions...
 */

static int	make_subpackage(const char *prodname, const char *directory,
		                const char *platname, dist_t *dist,
			        const char *subpackage);
static const char	*pkg_path(const char *filename, const char *dirname);


/*
 * 'make_pkg()' - Make an AT&T software distribution package.
 */

int					/* O - 0 = success, 1 = fail */
make_pkg(const char     *prodname,	/* I - Product short name */
         const char     *directory,	/* I - Directory for distribution files */
         const char     *platname,	/* I - Platform name */
         dist_t         *dist,		/* I - Distribution information */
	 struct utsname *platform)	/* I - Platform information */
{
  int		i;			/* Looping var */
  tarf_t	*tarfile;		/* Distribution tar file */
  char		name[1024],		/* Product filename */
		filename[1024];		/* Destination filename */


  if (make_subpackage(prodname, directory, platname, dist, NULL))
    return (1);

  for (i = 0; i < dist->num_subpackages; i ++)
    if (make_subpackage(prodname, directory, platname, dist,
                        dist->subpackages[i]))
      return (1);

 /*
  * Build a compressed tar file to hold all of the subpackages...
  */

  if (dist->num_subpackages)
  {
   /*
    * Figure out the full name of the distribution...
    */

    if (dist->release[0])
      snprintf(name, sizeof(name), "%s_%s-%s", prodname, dist->version,
               dist->release);
    else
      snprintf(name, sizeof(name), "%s_%s", prodname, dist->version);

    if (platname[0])
    {
      strlcat(name, "_", sizeof(name));
      strlcat(name, platname, sizeof(name));
    }

   /*
    * Create a compressed tar file...
    */

    snprintf(filename, sizeof(filename), "%s/%s.pkg.gz.tgz", directory, name);

    if ((tarfile = tar_open(filename, 1)) == NULL)
      return (1);

   /*
    * Archive the main package and subpackages...
    */

    if (tar_package(tarfile, "pkg.gz", prodname, directory, platname, dist, NULL))
    {
      tar_close(tarfile);
      return (1);
    }

    for (i = 0; i < dist->num_subpackages; i ++)
    {
      if (tar_package(tarfile, "pkg.gz", prodname, directory, platname, dist,
                      dist->subpackages[i]))
      {
	tar_close(tarfile);
	return (1);
      }
    }
    
    tar_close(tarfile);
  }

 /*
  * Remove temporary files...
  */

  if (!KeepFiles && dist->num_subpackages)
  {
    if (Verbosity)
      puts("Removing temporary distribution files...");

   /*
    * Remove .pkg.gz files since they are now in a .pkg.gz.tgz file...
    */

    unlink_package("pkg.gz", prodname, directory, platname, dist, NULL);

    for (i = 0; i < dist->num_subpackages; i ++)
      unlink_package("pkg.gz", prodname, directory, platname, dist,
                     dist->subpackages[i]);
  }

  return (0);
}

/*
 * 'make_subpackage()' - Create a subpackage...
 */

static int				/* O - 0 = success, 1 = fail */
make_subpackage(
    const char     *prodname,		/* I - Product short name */
    const char     *directory,		/* I - Directory for distribution files */
    const char     *platname,		/* I - Platform name */
    dist_t         *dist,		/* I - Distribution information */
    const char     *subpackage)		/* I - Subpackage name */
{
  int		i;			/* Looping var */
  FILE		*fp;			/* Control file */
  char		prodfull[1024];		/* Full subpackage name */
  char		name[1024];		/* Full product name */
  char		filename[1024],		/* Destination filename */
		preinstall[1024],	/* Pre install script */
		postinstall[1024],	/* Post install script */
		preremove[1024],	/* Pre remove script */
		postremove[1024],	/* Post remove script */
		request[1024];		/* Request script */
  char		current[1024];		/* Current directory */
  file_t	*file;			/* Current distribution file */
  command_t	*c;			/* Current command */
  depend_t	*d;			/* Current dependency */
  time_t	curtime;		/* Current time info */
  struct tm	*curdate;		/* Current date info */
  const char	*runlevels;		/* Run levels */
  const char	*class;			/* File class */


  getcwd(current, sizeof(current));

  if (subpackage)
    snprintf(prodfull, sizeof(prodfull), "%s-%s", prodname, subpackage);
  else
    strlcpy(prodfull, prodname, sizeof(prodfull));

  if (Verbosity)
    puts("Creating PKG distribution...");

  if (dist->release[0])
  {
    if (platname[0])
      snprintf(name, sizeof(name), "%s_%s-%s_%s", prodfull, dist->version,
               dist->release, platname);
    else
      snprintf(name, sizeof(name), "%s_%s-%s", prodfull, dist->version,
               dist->release);
  }
  else if (platname[0])
    snprintf(name, sizeof(name), "%s_%s_%s", prodfull, dist->version, platname);
  else
    snprintf(name, sizeof(name), "%s_%s", prodfull, dist->version);

 /*
  * Write the pkginfo file for pkgmk...
  */

  if (Verbosity)
    puts("Creating package information file...");

  snprintf(filename, sizeof(filename), "%s/%s.pkginfo", directory, prodfull);

  if ((fp = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "epm: Unable to create package information file \"%s\" - %s\n", filename,
            strerror(errno));
    return (1);
  }

  curtime = time(NULL);
  curdate = gmtime(&curtime);

  fprintf(fp, "PKG=%s\n", prodfull);
  fprintf(fp, "NAME=%s", dist->product);
  if (subpackage)
  {
    for (i = 0; i < dist->num_descriptions; i ++)
      if (dist->descriptions[i].subpackage == subpackage)
	break;

    if (i < dist->num_descriptions)
    {
      char	line[1024],		/* First line of description... */
		*ptr;			/* Pointer into line */


      strlcpy(line, dist->descriptions[i].description, sizeof(line));
      if ((ptr = strchr(line, '\n')) != NULL)
        *ptr = '\0';

      fprintf(fp, " - %s", line);
    }
  }
  fputs("\n", fp);
  fprintf(fp, "VERSION=%s\n", dist->version);
  fprintf(fp, "VENDOR=%s\n", dist->vendor);
  fprintf(fp, "PSTAMP=epm%04d%02d%02d%02d%02d%02d\n",
          curdate->tm_year + 1900, curdate->tm_mon + 1, curdate->tm_mday,
	  curdate->tm_hour, curdate->tm_min, curdate->tm_sec);

  fputs("CATEGORY=application\n", fp);

  fclose(fp);

 /*
  * Write the depend file for pkgmk...
  */

  if (Verbosity)
    puts("Creating package dependency file...");

  snprintf(filename, sizeof(filename), "%s/%s.depend", directory, prodfull);

  if ((fp = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "epm: Unable to create package dependency file \"%s\" - %s\n", filename,
            strerror(errno));
    return (1);
  }

  for (i = dist->num_depends, d = dist->depends; i > 0; i --, d ++)
  {
    if (d->subpackage != subpackage)
      continue;

    if (!strcmp(d->product, "_self"))
      continue;
    else if (d->type == DEPEND_REQUIRES)
      fprintf(fp, "P %s\n", d->product);
    else
      fprintf(fp, "I %s\n", d->product);
  }

  fclose(fp);

 /*
  * Write the preinstall file for pkgmk...
  */

  for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
    if (c->subpackage == subpackage && c->type == COMMAND_PRE_INSTALL)
      break;

  if (i)
  {
   /*
    * Write the preinstall file for pkgmk...
    */

    if (Verbosity)
      puts("Creating preinstall script...");

    snprintf(preinstall, sizeof(preinstall), "%s/%s.preinstall", directory,
             prodfull);

    if ((fp = fopen(preinstall, "w")) == NULL)
    {
      fprintf(stderr, "epm: Unable to create script file \"%s\" - %s\n", preinstall,
              strerror(errno));
      return (1);
    }

    fchmod(fileno(fp), 0755);

    fputs("#!/bin/sh\n", fp);
    fputs("# " EPM_VERSION "\n", fp);

    for (; i > 0; i --, c ++)
      if (c->subpackage == subpackage && c->type == COMMAND_PRE_INSTALL)
        fprintf(fp, "%s\n", c->command);

    fclose(fp);
  }
  else
    preinstall[0] = '\0';

 /*
  * Write the postinstall file for pkgmk...
  */

  for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
    if (c->subpackage == subpackage && c->type == COMMAND_POST_INSTALL)
      break;

  if (!i)
    for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
      if (file->subpackage == subpackage && tolower(file->type) == 'i')
        break;

  if (i)
  {
   /*
    * Write the postinstall file for pkgmk...
    */

    if (Verbosity)
      puts("Creating postinstall script...");

    snprintf(postinstall, sizeof(postinstall), "%s/%s.postinstall", directory,
             prodfull);

    if ((fp = fopen(postinstall, "w")) == NULL)
    {
      fprintf(stderr, "epm: Unable to create script file \"%s\" - %s\n", postinstall,
              strerror(errno));
      return (1);
    }

    fchmod(fileno(fp), 0755);

    fputs("#!/bin/sh\n", fp);
    fputs("# " EPM_VERSION "\n", fp);

    for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
      if (c->subpackage == subpackage && c->type == COMMAND_POST_INSTALL)
        fprintf(fp, "%s\n", c->command);

    for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
      if (file->subpackage == subpackage && tolower(file->type) == 'i')
	qprintf(fp, "/etc/init.d/%s start\n", file->dst);

    fclose(fp);
  }
  else
    postinstall[0] = '\0';

 /*
  * Write the preremove script...
  */

  for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
    if (c->subpackage == subpackage && c->type == COMMAND_PRE_REMOVE)
      break;

  if (!i)
    for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
      if (file->subpackage == subpackage && tolower(file->type) == 'i')
        break;

  if (i)
  {
   /*
    * Write the preremove file for pkgmk...
    */

    if (Verbosity)
      puts("Creating preremove script...");

    snprintf(preremove, sizeof(preremove), "%s/%s.preremove", directory, prodfull);

    if ((fp = fopen(preremove, "w")) == NULL)
    {
      fprintf(stderr, "epm: Unable to create script file \"%s\" - %s\n", preremove,
              strerror(errno));
      return (1);
    }

    fchmod(fileno(fp), 0755);

    fputs("#!/bin/sh\n", fp);
    fputs("# " EPM_VERSION "\n", fp);

    for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
      if (file->subpackage == subpackage && tolower(file->type) == 'i')
	qprintf(fp, "/etc/init.d/%s stop\n", file->dst);

    for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
      if (c->subpackage == subpackage && c->type == COMMAND_PRE_REMOVE)
        fprintf(fp, "%s\n", c->command);

    fclose(fp);
  }
  else
    preremove[0] = '\0';

 /*
  * Write the postremove file for pkgmk...
  */

  for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
    if (c->subpackage == subpackage && c->type == COMMAND_POST_REMOVE)
      break;

  if (i)
  {
   /*
    * Write the postremove file for pkgmk...
    */

    if (Verbosity)
      puts("Creating postremove script...");

    snprintf(postremove, sizeof(postremove), "%s/%s.postremove", directory,
             prodfull);

    if ((fp = fopen(postremove, "w")) == NULL)
    {
      fprintf(stderr, "epm: Unable to create script file \"%s\" - %s\n", postremove,
              strerror(errno));
      return (1);
    }

    fchmod(fileno(fp), 0755);

    fputs("#!/bin/sh\n", fp);
    fputs("# " EPM_VERSION "\n", fp);

    for (; i > 0; i --, c ++)
      if (c->subpackage == subpackage && c->type == COMMAND_POST_REMOVE)
        fprintf(fp, "%s\n", c->command);

    fclose(fp);
  }
  else
    postremove[0] = '\0';

 /*
  * Write the request file for pkgmk...
  */

  for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
    if (c->subpackage == subpackage &&
        c->type == COMMAND_LITERAL && !strcmp(c->section, "request"))
      break;

  if (i)
  {
   /*
    * Write the request file for pkgmk...
    */

    if (Verbosity)
      puts("Creating request script...");

    snprintf(request, sizeof(request), "%s/%s.request", directory,
             prodfull);

    if ((fp = fopen(request, "w")) == NULL)
    {
      fprintf(stderr, "epm: Unable to create script file \"%s\" - %s\n",
              request, strerror(errno));
      return (1);
    }

    fchmod(fileno(fp), 0755);

    fputs("#!/bin/sh\n", fp);
    fputs("# " EPM_VERSION "\n", fp);

    for (; i > 0; i --, c ++)
      if (c->subpackage == subpackage &&
          c->type == COMMAND_LITERAL && !strcmp(c->section, "request"))
        fprintf(fp, "%s\n", c->command);

    fclose(fp);
  }
  else
    request[0] = '\0';

 /*
  * Add symlinks for init scripts...
  */

  for (i = 0; i < dist->num_files; i ++)
    if (dist->files[i].subpackage == subpackage &&
        tolower(dist->files[i].type) == 'i')
    {
     /*
      * Make symlinks for all of the selected run levels...
      */

      for (runlevels = get_runlevels(dist->files + i, "023");
           isdigit(*runlevels & 255);
	   runlevels ++)
      {
	file = add_file(dist, dist->files[i].subpackage);
	file->type = 'l';
	file->mode = 0;
	strcpy(file->user, "root");
	strcpy(file->group, "sys");
	snprintf(file->src, sizeof(file->src), "../init.d/%s",
        	 dist->files[i].dst);

        if (*runlevels == '0')
	  snprintf(file->dst, sizeof(file->dst), "/etc/rc0.d/K%02d%s",
        	   get_stop(dist->files + i, 0), dist->files[i].dst);
        else
	  snprintf(file->dst, sizeof(file->dst), "/etc/rc%c.d/S%02d%s",
        	   *runlevels, get_start(dist->files + i, 99),
		   dist->files[i].dst);
      }

     /*
      * Then send the original file to /etc/init.d...
      */

      file = dist->files + i;

      snprintf(filename, sizeof(filename), "/etc/init.d/%s", file->dst);
      strcpy(file->dst, filename);
    }

 /*
  * Write the prototype file for pkgmk...
  */

  if (Verbosity)
    puts("Creating prototype file...");

  snprintf(filename, sizeof(filename), "%s/%s.prototype", directory, prodfull);

  if ((fp = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "epm: Unable to create prototype file \"%s\" - %s\n",
            filename, strerror(errno));
    return (1);
  }

#if 0 /* apparently does not work on Solaris 7... */
  fprintf(fp, "!search %s\n", current);
#endif /* 0 */

  if (dist->num_licenses==1) {
    fprintf(fp, "i copyright=%s\n", pkg_path(dist->licenses[0].src, current));
  }

  fprintf(fp, "i depend=%s/%s.depend\n", pkg_path(directory, current), prodfull);
  fprintf(fp, "i pkginfo=%s/%s.pkginfo\n", pkg_path(directory, current),
          prodfull);

  if (preinstall[0])
    fprintf(fp, "i preinstall=%s\n", pkg_path(preinstall, current));
  if (postinstall[0])
    fprintf(fp, "i postinstall=%s\n", pkg_path(postinstall, current));
  if (preremove[0])
    fprintf(fp, "i preremove=%s\n", pkg_path(preremove, current));
  if (postremove[0])
    fprintf(fp, "i postremove=%s\n", pkg_path(postremove, current));
  if (request[0])
    fprintf(fp, "i request=%s\n", pkg_path(request, current));


  for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
    if (file->subpackage == subpackage)
      switch (tolower(file->type))
      {
      case 'c' :
          qprintf(fp, "e %s %s=%s %04o %s %s\n",
	          file->subpackage ? file->subpackage : "none",
	          file->dst, pkg_path(file->src, current),
		  file->mode, file->user, file->group);
          break;
      case 'd' :
	  qprintf(fp, "d %s %s %04o %s %s\n",
	          file->subpackage ? file->subpackage : "none",
	          file->dst, file->mode, file->user, file->group);
          break;
      case 'f' :
      case 'i' :
          if (strstr(file->options, "manifest()") != NULL)
              class="manifest";
          else
              class=file->subpackage ? file->subpackage : "none";

          qprintf(fp, "f %s %s=%s %04o %s %s\n", class,
                  file->dst, pkg_path(file->src, current),
                  file->mode, file->user, file->group);

          break;
      case 'l' :
          qprintf(fp, "s %s %s=%s\n",
	          file->subpackage ? file->subpackage : "none",
	          file->dst, file->src);
          break;
    }

  fclose(fp);

 /*
  * Build the distribution from the prototype file...
  */

  if (Verbosity)
    puts("Building PKG binary distribution...");

  char pkg_create[512];
  sprintf(pkg_create, "pkgmk -o -f %s/%s.prototype -d %s/%s",
          directory, prodfull, current, directory);
  puts(pkg_create);
  if (run_command(NULL, pkg_create))
    return (1);

 /*
  * Make a package stream file...
  */

  if (Verbosity)
    puts("Copying into package stream file...");

  if (run_command(directory, "pkgtrans -s %s/%s %s.pkg %s",
                  current, directory, name, prodfull))
    return (1);

 /*
  * Compress the package stream file...
  */

  snprintf(filename, sizeof(filename), "%s/%s.pkg.gz", directory, name);
  unlink(filename);

  if (run_command(directory, EPM_GZIP " -v9 %s.pkg", name))
    return (1);

 /*
  * Remove temporary files...
  */

  if (!KeepFiles)
  {
    if (Verbosity)
      puts("Removing temporary distribution files...");

    snprintf(filename, sizeof(filename), "%s/%s.pkginfo", directory, prodfull);
    unlink(filename);
    snprintf(filename, sizeof(filename), "%s/%s.depend", directory, prodfull);
    unlink(filename);
    snprintf(filename, sizeof(filename), "%s/%s.prototype", directory, prodfull);
    unlink(filename);
    if (preinstall[0])
      unlink(preinstall);
    if (postinstall[0])
      unlink(postinstall);
    if (preremove[0])
      unlink(preremove);
    if (postremove[0])
      unlink(postremove);
    if (request[0])
      unlink(request);
  }

  return (0);
}


/*
 * 'pkg_path()' - Return an absolute path for the prototype file.
 */

static const char *			/* O - Absolute filename */
pkg_path(const char *filename,		/* I - Source filename */
         const char *dirname)		/* I - Source directory */
{
  static char	absname[1024];		/* Absolute filename */


  if (filename[0] == '/')
    return (filename);

  snprintf(absname, sizeof(absname), "%s/%s", dirname, filename);
  return (absname);
}


/*
 * End of "$Id: pkg.c,v 1.1.1.1.2.2 2009/05/07 15:49:02 bsavelev Exp $".
 */
