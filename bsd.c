/*
 * "$Id: bsd.c,v 1.1 2009/01/22 10:46:58 anikolov Exp $"
 *
 *   Free/Net/OpenBSD package gateway for the ESP Package Manager (EPM).
 *
 *   Copyright 1999-2007 by Easy Software Products.
 *   Copyright 2009-2011 by Dr.Web.
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
 *   make_bsd()        - Make a Free/Net/OpenBSD software distribution package.
 *   make_subpackage() - Create a subpackage...
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


/*
 * 'make_bsd()' - Make a Free/Net/OpenBSD software distribution package.
 */

int					/* O - 0 = success, 1 = fail */
make_bsd(const char     *prodname,	/* I - Product short name */
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

    snprintf(filename, sizeof(filename), "%s/%s.tbz.tgz", directory, name);

    if ((tarfile = tar_open(filename, 1)) == NULL)
      return (1);

   /*
    * Archive the main package and subpackages...
    */

    if (tar_package(tarfile, "tbz", prodname, directory, platname, dist, NULL))
    {
      tar_close(tarfile);
      return (1);
    }

    for (i = 0; i < dist->num_subpackages; i ++)
    {
      if (tar_package(tarfile, "tbz", prodname, directory, platname, dist,
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
    * Remove .tbz files since they are now in a .tbz.tgz file...
    */

    unlink_package("tbz", prodname, directory, platname, dist, NULL);

    for (i = 0; i < dist->num_subpackages; i ++)
      unlink_package("tbz", prodname, directory, platname, dist,
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
  FILE		*fp;			/* Spec file */
  char		prodfull[1024];		/* Full subpackage name */
  char		name[1024];		/* Full product name */
  char		commentname[1024];	/* pkg comment filename */
  char		descrname[1024];	/* pkg descr filename */
  char		plistname[1024];	/* pkg plist filename */
  char		preinstallname[1024];	/* pre-install script filename */
  char		postinstallname[1024];	/* post-install script filename */
  char		preremovename[1024];	/* pre-remove script filename */
  char		postremovename[1024];	/* post-remove script filename */
  char		preinstall_opt[1024];	/* pkg_create option for pre-install */
  char		postinstall_opt[1024];	/* pkg_create option for post-install */
  char		preremove_opt[1024];	/* pkg_create option for pre-remove */
  char		postremove_opt[1024];	/* pkg_create option for post-remove */
  char		origin[1024];		/* Package origin */
  char		filename[1024];		/* Destination filename */
  char		*old_user,		/* Old owner UID */
		*old_group;		/* Old group ID */
  int		old_mode;		/* Old permissions */
  file_t	*file;			/* Current distribution file */
  command_t	*c;			/* Current command */
  depend_t	*d;			/* Current dependency */
  struct passwd	*pwd;			/* Pointer to user record */
  struct group	*grp;			/* Pointer to group record */
  char		current[1024];		/* Current directory */


  getcwd(current, sizeof(current));

  if (subpackage)
    snprintf(prodfull, sizeof(prodfull), "%s-%s", prodname, subpackage);
  else
    strlcpy(prodfull, prodname, sizeof(prodfull));

  snprintf(origin, sizeof(origin), "security/%s",  prodfull);

  if (Verbosity)
    printf("Creating %s *BSD pkg distribution...\n", prodfull);

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
  * Write the descr file for pkg...
  */

  if (Verbosity)
    printf("Creating %s.descr file...\n", prodfull);

  snprintf(descrname, sizeof(descrname), "%s/%s.descr", directory, prodfull);

  if ((fp = fopen(descrname, "w")) == NULL)
  {
    fprintf(stderr, "epm: Unable to create descr file \"%s\" - %s\n", descrname,
            strerror(errno));
    return (1);
  }

  for (i = 0; i < dist->num_descriptions; i ++)
    if (dist->descriptions[i].subpackage == subpackage)
      break;

  if (i < dist->num_descriptions)
    fprintf(fp, "%s\n", dist->descriptions[i].description);

  fclose(fp);

 /*
  * Write the comment file for pkg...
  */

  if (Verbosity)
    printf("Creating %s.comment file...\n", prodfull);

  snprintf(commentname, sizeof(commentname), "%s/%s.comment", directory,
           prodfull);

  if ((fp = fopen(commentname, "w")) == NULL)
  {
    fprintf(stderr, "epm: Unable to create comment file \"%s\" - %s\n", commentname,
            strerror(errno));
    return (1);
  }

  fprintf(fp, "%s", dist->product);
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
  fputc('\n', fp);

  fclose(fp);

 /*
  * Write the plist file for pkg...
  */

  if (Verbosity)
    printf("Creating %s.plist file...\n", prodfull);

  snprintf(plistname, sizeof(plistname), "%s/%s.plist", directory, prodfull);

  if ((fp = fopen(plistname, "w")) == NULL)
  {
    fprintf(stderr, "epm: Unable to create plist file \"%s\" - %s\n", plistname,
            strerror(errno));
    return (1);
  }

 /*
  * Declare our (non-standard) package name explicitly.
  */

  /* Shorten version string to 5 symbols (x.y.z). */
  char ver[6];
  strncpy(ver, dist->version, 5);

  fprintf(fp, "@name %s-%s\n", prodfull, ver);

 /*
  * FreeBSD and NetBSD support both "source directory" and "preserve files"
  * options, OpenBSD does not...
  */

#ifdef __FreeBSD__
  fprintf(fp, "@srcdir %s/%s/%s.buildroot\n", current, directory, prodfull);
  fputs("@option preserve\n", fp);
#elif defined(__NetBSD__)
  fprintf(fp, "@src %s/%s/%s.buildroot\n", current, directory, prodfull);
  fputs("@option preserve\n", fp);
#endif /* __FreeBSD__ */

  for (i = dist->num_depends, d = dist->depends; i > 0; i --, d ++)
  {
    if (d->subpackage != subpackage)
      continue;

    if (d->type == DEPEND_REQUIRES)
#ifdef __OpenBSD__
      fprintf(fp, "@depend %s", d->product);
#else
      fprintf(fp, "@pkgdep %s", d->product);
#endif /* __OpenBSD__ */
    else
#ifdef __FreeBSD__
     /*
      * FreeBSD uses @conflicts...
      */
      fprintf(fp, "@conflicts %s", d->product);
#elif defined(__OpenBSD__)
      fprintf(fp, "@conflict %s", d->product);
#else
      fprintf(fp, "@pkgcfl %s", d->product);
#endif /* __FreeBSD__ */
    if (d->vernumber[0] > 0)
      fprintf(fp, "-%s\n", d->version[0]);
    else
      putc('\n', fp);
  }

  for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
    if (tolower(file->type) == 'd' && file->subpackage == subpackage)
    {
     /*
      * We create and update directories as postinstall commands to
      * avoid a bug in the FreeBSD pkg_delete command.
      */

      fprintf(fp, "@exec mkdir -p %s\n", file->dst);
      if (strcmp(file->user, "root") != 0 || strcmp(file->group, "root") != 0)
        fprintf(fp, "@exec chown %s:%s %s\n", file->user, file->group,
                file->dst);
      fprintf(fp, "@exec chmod %04o %s\n", file->mode, file->dst);
    }

  for (i = dist->num_files, file = dist->files, old_mode = 0, old_user = "",
           old_group = "";
       i > 0;
       i --, file ++)
  {
   /*
    * The FreeBSD pkg_delete command (at least) doesn't like creating
    * and deleting directories.  I don't know if other BSD's have the
    * same problem, but for now just put the directory stuff in a
    * postinstall script...
    */

    if (tolower(file->type) == 'd' || file->subpackage != subpackage)
      continue;

    if (file->mode != old_mode)
      fprintf(fp, "@mode %04o\n", old_mode = file->mode);
    if (strcmp(file->user, old_user))
      fprintf(fp, "@owner %s\n", old_user = file->user);
    if (strcmp(file->group, old_group))
    {
      if (!strcmp(file->group, "root"))
          fprintf(fp, "@group %s\n", "wheel");
      else
          fprintf(fp, "@group %s\n", file->group);

      old_group = file->group;
    }

    switch (tolower(file->type))
    {
      case 'i' :
          qprintf(fp, "usr/local/etc/rc.d/%s.sh\n", file->dst);
          break;
      case 'c' :
      case 'f' :
      case 'l' :
#ifdef __OpenBSD__
          qprintf(fp, "@file %s\n", file->dst + 1);
	  if (tolower(file->type) == 'c')
            qprintf(fp, "@sample %s\n", file->dst + 1);
#else
          qprintf(fp, "%s\n", file->dst + 1);
#endif /* __OpenBSD__ */
          break;
    }
  }

 /*
  * Need to list directories to remove in reverse order after
  * everything else...
  */

  for (i = dist->num_files, file = dist->files + i - 1;
       i > 0;
       i --, file --)
    if (tolower(file->type) == 'd' && file->subpackage == subpackage)
      qprintf(fp, "@dirrm %s\n", file->dst + 1);

  fclose(fp);

 /*
  * Write the pre/post install/remove scripts for pkg...
  */

  snprintf(preinstallname, sizeof(preinstallname), "%s/preinst", directory);
  snprintf(postinstallname, sizeof(postinstallname), "%s/postinst", directory);
  snprintf(preremovename, sizeof(preremovename), "%s/prerm", directory);
  snprintf(postremovename, sizeof(postremovename), "%s/postrm", directory);

  preinstall_opt[0]=0;
  postinstall_opt[0]=0;
  preremove_opt[0]=0;
  postremove_opt[0]=0;

 /*
  * Remove files so that appending would re-create them below.
  */
  unlink(preinstallname);
  unlink(postinstallname);
  unlink(preremovename);
  unlink(postremovename);

  for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
    if (c->subpackage == subpackage)
      switch (c->type)
      {
        case COMMAND_PRE_INSTALL :
            if (Verbosity)
              printf("Writing to file %s...\n", preinstallname);
            if ((fp = fopen(preinstallname, "a")) == NULL)
            {
              fprintf(stderr, "epm: Unable to create pre-install script file \"%s\" - %s\n", preinstallname,
                      strerror(errno));
              return (1);
            }
            fprintf(fp, "%s\n", c->command);
            fclose(fp);
            snprintf(preinstall_opt, sizeof(preinstall_opt), "-i %s ", preinstallname);
            break;
	case COMMAND_POST_INSTALL :
            if (Verbosity)
              printf("Writing to file %s...\n", postinstallname);
            if ((fp = fopen(postinstallname, "a")) == NULL)
            {
              fprintf(stderr, "epm: Unable to create post-install script file \"%s\" - %s\n", postinstallname,
                      strerror(errno));
              return (1);
            }
            fprintf(fp, "%s\n", c->command);
            fclose(fp);
            snprintf(postinstall_opt, sizeof(postinstall_opt), "-I %s ", postinstallname);
	    break;
	case COMMAND_PRE_REMOVE :
            if (Verbosity)
              printf("Writing to file %s...\n", preremovename);
            if ((fp = fopen(preremovename, "a")) == NULL)
            {
              fprintf(stderr, "epm: Unable to create pre-remove script file \"%s\" - %s\n", preremovename,
                      strerror(errno));
              return (1);
            }
            fprintf(fp, "%s\n", c->command);
            fclose(fp);
            snprintf(preremove_opt, sizeof(preremove_opt), "-k %s ", preremovename);
	    break;
	case COMMAND_POST_REMOVE :
            if (Verbosity)
              printf("Writing to file %s...\n", postremovename);
            if ((fp = fopen(postremovename, "a")) == NULL)
            {
              fprintf(stderr, "epm: Unable to create post-remove script file \"%s\" - %s\n", postremovename,
                      strerror(errno));
              return (1);
            }
            fprintf(fp, "%s\n", c->command);
            fclose(fp);
            snprintf(postremove_opt, sizeof(postremove_opt), "-K %s ", postremovename);
            break;
      }

 /*
  * Copy the files over...
  */

  if (Verbosity)
    puts("Copying temporary distribution files...");

  for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
  {
    if (file->subpackage != subpackage)
      continue;

   /*
    * Find the username and groupname IDs...
    */

    pwd = getpwnam(file->user);
    grp = getgrnam(file->group);

    endpwent();
    endgrent();

   /*
    * Copy the file or make the directory or make the symlink as needed...
    */

    switch (tolower(file->type))
    {
      case 'c' :
      case 'f' :
          snprintf(filename, sizeof(filename), "%s/%s.buildroot%s",
	           directory, prodfull, file->dst);

	  if (Verbosity > 1)
	    printf("%s -> %s...\n", file->src, filename);

	  if (copy_file(filename, file->src, file->mode, pwd ? pwd->pw_uid : 0,
			grp ? grp->gr_gid : 0))
	    return (1);
          break;
      case 'i' :
          snprintf(filename, sizeof(filename),
                   "%s/%s.buildroot/usr/local/etc/rc.d/%s.sh", directory,
                   prodfull, file->dst);

	  if (Verbosity > 1)
	    printf("%s -> %s...\n", file->src, filename);

	  if (copy_file(filename, file->src, file->mode, pwd ? pwd->pw_uid : 0,
			grp ? grp->gr_gid : 0))
	    return (1);
          break;
      case 'd' :
          snprintf(filename, sizeof(filename), "%s/%s.buildroot%s",
	           directory, prodfull, file->dst);

	  if (Verbosity > 1)
	    printf("Directory %s...\n", filename);

          make_directory(filename, file->mode, pwd ? pwd->pw_uid : 0,
			 grp ? grp->gr_gid : 0);
          break;
      case 'l' :
          snprintf(filename, sizeof(filename), "%s/%s.buildroot%s",
	           directory, prodfull, file->dst);

	  if (Verbosity > 1)
	    printf("%s -> %s...\n", file->src, filename);

          make_link(filename, file->src);
          break;
    }
  }

 /*
  * Build the distribution...
  */

  if (Verbosity)
    printf("Building %s *BSD pkg binary distribution...\n", prodfull);

#ifdef __OpenBSD__
  if (run_command(NULL, "pkg_create -p / -B %s/%s.buildroot "
                        "-c %s "
			"-d %s "
                        "-f %s "
			"%s.tgz",
                  directory, prodfull,
		  commentname,
		  descrname,
		  plistname,
		  name))
    return (1);

  if (run_command(NULL, "mv %s.tgz %s", name, directory))
    return (1);
#elif defined(__FreeBSD__)
  char pkg_create[512];
  sprintf(pkg_create,
          "/usr/sbin/pkg_create -p / "
          "-c %s "
          "-d %s "
          "-f %s "
          "-o %s "
          "%s %s %s %s "
          "%s/%s.tbz",
          commentname,
          descrname,
          plistname,
          origin,
          preinstall_opt,
          postinstall_opt,
          preremove_opt,
          postremove_opt,
          directory, name);
  puts(pkg_create);
  if (run_command(NULL, pkg_create))
    return (1);
#else
  if (run_command(NULL, "pkg_create -p / "
                        "-c %s "
			"-d %s "
                        "-f %s "
			"%s/%s.tgz",
		  commentname,
		  descrname,
		  plistname,
		  directory, name))
    return (1);
#endif /* __OpenBSD__ */

 /*
  * Remove temporary files...
  */

  if (!KeepFiles)
  {
    if (Verbosity)
      puts("Removing temporary distribution files...");

    run_command(NULL, "/bin/rm -rf %s/%s.buildroot", directory, prodfull);

    unlink(plistname);
    unlink(commentname);
    unlink(descrname);
    unlink(preinstallname);
    unlink(postinstallname);
    unlink(preremovename);
    unlink(postremovename);
  }

  return (0);
}


/*
 * End of "$Id: bsd.c,v 1.1 2009/01/22 10:46:58 anikolov Exp $".
 */
