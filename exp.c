
/* This is copyrighted software but you may use/distribute it     *
 * in accordance with the conditions set out in the file COPYING  */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "defs.h"

/* requires a path which has already been converted to unix format */

int 
expr (char *foos, char *tos, int tlen, int opts)
{
  uid_t myid, mygid;
  struct stat fst;
  int retval = 0;
  int vl, i;
  char *vv, *ff, *ww, *tt;
  int cud = 1;
  DIR *d;
  struct dirent *dent;
  int state, f, w, walk, count = 0;
  int base;
  int statok;

  myid = getuid ();
  mygid = getgid ();
  if (!opts)
    statok = 1;
  else
    statok = 0;

  vl = CEM (strlen (foos) + 4, 16);

#ifdef DEBUG
  fprintf (stderr, "about to allocate core\n");
#endif
  vv = (char *) malloc (vl * sizeof (char));
#ifdef DEBUG
  fprintf (stderr, "allocated core (%d bytes)\n", vl);
#endif


  if (vv != NULL)
    {
      i = 0;
      strcpy (vv, foos);
      tos[0] = '\0';
      tos[1] = '\0';

      while (vv[i] != '\0')
	{
	  if (vv[i] == '/')
	    cud = 0;
	  i++;
	}

      if (cud)
	{
	  d = opendir (".");
	  ww = vv;
	  base = 0;
	}
      else
	{
	  i = strlen (vv);
	  while (vv[i] != '/')
	    i--;
	  vv[i] = '\0';
	  ww = (&vv[i + 1]);

#ifdef DEBUG
	  fprintf (stderr, "base of expresion <%s>\n", vv);
#endif
	  base = strlen (vv) + 1;
	  d = opendir (vv);
	}

      if (d != NULL)
	{
#ifdef DEBUG
	  if (!cud)
	    fprintf (stderr, "Could open dir <%s>\n", vv);
	  else
	    fprintf (stderr, "Could open dir <.>\n");
	  fprintf (stderr, "Looking for match to <%s>\n", ww);
#endif

	  while (NULL != (dent = readdir (d)))
	    {
#ifdef DEBUG
	      fprintf (stderr, "Found dir entry with name %s\n", dent->d_name);
#endif
	      /* evaluate "regex" here */
	      state = 0;
	      f = 0;
	      w = 0;
	      walk = 1;
	      ff = dent->d_name;

	      while (walk)
		{
		  switch (state)
		    {
		    case 0:
		      if ((ff[f] != '\0') && (toupper (ff[f])) == (toupper (ww[w])))
			{
			  f++;
			  w++;
			}
		      else if (ww[w] == '*')
			{
			  state = 1;
			}
		      else if (ww[w] == '?')
			{
			  w++;
			  if (ff[f] == '\0')
			    {
			      while (ww[w] == '?' || ww[w] == '*' || ww[w] == '.')
				w++;
			      if (ww[w] != '\0')
				walk = 0;
			    }
			  else
			    {
			      if (ff[f] != '.')
				f++;
			    }
			}
		      else
			{
			  if (!((ff[f] == '\0') && (ww[w] == '\0')))
			    walk = 0;
			}
		      break;
		    case 1:
		      if (ff[f] == '.' || ff[f] == '\0')
			{
			  state = 0;
			  while (!(ww[w] == '.' || ww[w] == '\0'))
			    w++;
			  if (ff[f] == '\0')
			    {
			      while (ww[w] == '?' || ww[w] == '*' || ww[w] == '.')
				w++;
			      if (ww[w] != '\0')
				walk = 0;
			    }
			}
		      else
			{
			  f++;
			}
		      break;
		    }
		  if ((ff[f] == '\0') && (ww[w] == '\0') && walk)
		    {
#ifdef DEBUG
		      fprintf (stderr, "MATCH in <%s> to <%s>\n", ff, ww);
#endif
		      if ((count + (i = (strlen (ff) + base))) < tlen)
			{
			  if (base)
			    {
			      strcpy (tos + count, vv);
			      tos[count + base - 1] = '/';
			    }
			  strcpy (tos + (count + base), ff);
			  i++;
			}
		      else
			{
			  if (cmdvc < CMDVM)
			    {
#ifdef DEBUG
			      fprintf (stderr, "trying to realloc core from %d", cmdvc);
#endif
			      tt = realloc (cmdv, cmdvc * 2);
			      if (tt != NULL)
				{
				  tlen += cmdvc;
				  cmdvc *= 2;
				  tos = tt + (tos - cmdv);
				  cmdv = tt;
#ifdef DEBUG
				  fprintf (stderr, " to %d ... ok\n", cmdvc);
#endif
				  if (base)
				    {
				      strcpy (tos + count, vv);
				      tos[count + base - 1] = '/';
				    }
				  strcpy (tos + (count + base), ff);
				  i++;
				}
			      else
				{
#ifdef DEBUG
				  fprintf (stderr, "oooopps ... failed\n");
#endif
				  retval = 2;
				  i = 0;
				}
			    }
			  else
			    {
			      i = 0;
			      retval = 2;
			    }
#ifdef DEBUG
			  fprintf (stderr, "Exceeded Count \n");
#endif
			}

		      if (opts)
			{
			  statok = 0;
			  if (!stat (tos + count, &fst))
			    {
#ifdef DEBUG
			      fprintf (stderr, "Could stat file : ");
#endif
			      statok = 1;
			      if (opts & OOVIE)
				if (tos[count] == '.')
				  statok = 0;
			      if (opts & OOMIN)
				if (fst.st_uid != myid)
				  statok = 0;
			      if (opts & OOACC)
				if (!(
				       (07 & fst.st_mode) ||
				       ((070 & fst.st_mode) && (fst.st_gid == mygid)) ||
				       ((0700 & fst.st_mode) && (fst.st_uid == myid))
				    ))
				  {
#ifdef DEBUG
				    fprintf (stderr, "File inaccessible : ");
#endif
				    statok = 0;
				  }
			      if (opts & OOPLA)
				if ((fst.st_mode & S_IFMT) != S_IFREG)
				  {
#ifdef DEBUG
				    fprintf (stderr, "File not regular : ");
#endif
				    statok = 0;
				  }
			      if (opts & OOREA)
				if (!(
				       (04 & fst.st_mode) ||
				       ((040 & fst.st_mode) && (fst.st_gid == mygid)) ||
				       ((0400 & fst.st_mode) && (fst.st_uid == myid))
				    ))
				  {
				    statok = 0;
#ifdef DEBUG
				    fprintf (stderr, "File not readable : ");
#endif
				  }

			    }
#ifdef DEBUG
			  else
			    {
			      perror (" ");
			      fprintf (stderr, "File not statable : ");
			    }
#endif
#ifdef DEBUG
			  if (!statok)
			    fprintf (stderr, "Rejecting %s\n", ff);
#endif
			}
		      if (statok)
			{
			  count += i;
			}
		      walk = 0;
		    }

		}

	    }

	  closedir (d);
	  tos[count] = '\0';
	}

      else
	{
#ifdef DEBUG
	  fprintf (stderr, "Could NOT open dir %s\n", vv);
#endif
	}

      free (vv);
    }
  else
    {
      tos[0] = '\0';
      tos[1] = '\0';
    }

  return retval;
}

int 
expcount (char *ptr, int i)
{
  while (i < cmdvc)
    {
      if (ptr[i] == '\0')
	{
	  if (ptr[i + 1] == '\0')
	    {
	      return i;
	    }
	}
      i++;
    }
  return cmdvc;
}

int 
expwild (char *ptr, int i)
{
  while (i >= 0)
    {
      if (ptr[i] == '*' || ptr[i] == '?')
	{
	  return 1;
	}
      i--;
    }
  return 0;
}


#ifdef OODEBUG
int 
main ()
{
  int i;
  char fr[60];
  char buf[512];

  fprintf (stderr, "Enter expression : ");
  scanf ("%s", fr);

  if (expwild (fr, 60))
    fprintf (stderr, "This contains wild cards\n");

  expr (fr, buf, 512);
  i = 0;

  while (buf[i] != '\0')
    {
      fprintf (stderr, "element at %d is %s\n", i, buf + i);
      i += (strlen (buf + i) + 1);
    }

  fprintf (stderr, "Expcount returned %d\n", expcount (buf, 0));

  return 0;
}
#endif
