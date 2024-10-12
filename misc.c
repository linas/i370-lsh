/* This is copyrighted software but you may use/distribute it     *
 * in accordance with the conditions set out in the file COPYING  */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>

#ifdef USE_TERMCAP
#include <termcap.h>
#endif

#include "defs.h"


#ifdef USE_TERMCAP
int 
bootcap (char *termstr)
{
  if (tgetent (NULL, termstr) <= 0)
    {
      plerr (PLTER);
      return 1;
    }

  if (rsstr != NULL)
    {
      free (rsstr);
    }
  rsstr = tgetstr ("rs", NULL);
  if (rsstr == NULL)
    {
      rsstr = tgetstr ("cl", NULL);
    }

  if (sestr != NULL)
    {
      free (sestr);
    }
  sestr = tgetstr ("se", NULL);

  if (sostr != NULL)
    {
      free (sostr);
    }
  sostr = tgetstr ("so", NULL);

  if (tgetflag ("hs") && tgetflag ("es"))
    {
      if (fsstr)
	free (fsstr);
      fsstr = tgetstr ("fs", NULL);

      if (tsstr)
	free (tsstr);
      fsstr = tgetstr ("ts", NULL);
    }

  if (!tgetflag ("ks"))
    {
      if (k1str != NULL)
	{
	  free (k1str);
	}
      k1str = tgetstr ("k1", NULL);

      if (k3str != NULL)
	{
	  free (k3str);
	}
      k3str = tgetstr ("k3", NULL);

      if (krstr != NULL)
	{
	  free (krstr);
	}
      krstr = tgetstr ("kr", NULL);

      if (kustr != NULL)
	{
	  free (kustr);
	}
      kustr = tgetstr ("ku", NULL);

      if (klstr != NULL)
	{
	  free (klstr);
	}
      klstr = tgetstr ("kl", NULL);

      if (kdstr != NULL)
	{
	  free (kdstr);
	}
      kdstr = tgetstr ("kd", NULL);
    }

  if (hostr != NULL)
    {
      free (hostr);
    }
  hostr = tgetstr ("cl", NULL);
  if (hostr == NULL)
    {
      hostr = tgetstr ("ho", NULL);
    }

  return 0;
}
#endif


int 
checkopt (int tc, int tk, int *tks)
{
  int retval;
  switch (symtab[tks[tc]].id)
    {
    case LEM:
    case LSEP:
    case LAT:
    case LBAN:
    case LTO:
    case LFRO:
    case LPIP:
      retval = 0;
      break;
    case LSWI:
      if (symtab[tks[tc]].len == 2)
	{
	  retval = symstream[symtab[tks[tc]].ptr + 1];
	}
      else
	{
	  retval = 128;
	}
      break;
    default:
      retval = (-1);
      break;
    }
  return retval;
}

int 
mangle (char *src, char *mask, char *dest, int destspace)
{
  int srcptr = 0, destptr = 0, maskptr = 0;

  srcptr = strlen (src);
  while (srcptr && src[srcptr - 1] != '/')
    {
      srcptr--;
    }

#ifdef DEBUG
  fprintf (stderr, "Chewed off dir part: source <%s> at <%d>\n", src, srcptr);
#endif

  maskptr = strlen (mask);
  while (maskptr && mask[maskptr - 1] != '/')
    {
      maskptr--;
    }

#ifdef DEBUG
  fprintf (stderr, "Chewed of dir part: mask <%s> at <%d>\n", mask, maskptr);
#endif

  if (maskptr)
    {
      if (maskptr < destspace)
	{
	  strncpy (dest, mask, maskptr);
	  destptr = maskptr;

#ifdef DEBUG
	  fprintf (stderr, "Added the dir part <%s>\n", dest);
#endif

	}
      else
	{
	  return 1;		/* Our destination is too small */
	}
    }

#ifdef DEBUG
  fprintf (stderr, "About to mangle sourcename through mask\n");
#endif

  while (destptr < destspace && mask[maskptr] != '\0')
    {
      switch (mask[maskptr])
	{
	case '*':
	  if (src[srcptr] != '\0')
	    {
	      if (src[srcptr] == '.')
		{
		  maskptr++;
		  while (mask[maskptr] != '\0' && mask[maskptr] != '.')
		    {
		      maskptr++;
		    }
		}
	      else
		{
		  dest[destptr] = src[srcptr];
		  destptr++;
		  srcptr++;
		}
	    }
	  else
	    {
	      maskptr++;
	      if (mask[maskptr] == '.' && src[srcptr] == '\0')
		{
		  if (mask[maskptr + 1] == '\0' || mask[maskptr + 1] == '*')
		    {
		      maskptr++;
		    }
		}
	    }
	  break;
	case '?':
	  if (src[srcptr] != '\0')
	    {
	      dest[destptr] = src[srcptr];
	      destptr++;
	      srcptr++;
	    }
	  maskptr++;
	  break;
	default:
	  dest[destptr] = mask[maskptr];
	  maskptr++;
	  destptr++;
	  if (src[srcptr] != '\0')
	    srcptr++;
	  break;
	}
    }

  if (destptr < destspace)
    {
      dest[destptr] = '\0';
    }
  else
    {
      return 1;
    }

  return 0;
}

int 
countmail ()
{
  int count = 0;
  char *ep;
  struct stat mailfile;


  ep = getenv ("MAIL");
  if (ep)
    {
      if (!stat (ep, &mailfile))
	{
#ifdef DEBUG
	  fprintf (stderr, "checking %s for mail\n", ep);
#endif
	  count = mailfile.st_size;
	}
    }
  else
    {
      strcpy (cmdv, "/var/spool/mail/");
      if (NULL != (ep = getlogin ()))
	{
	  strcat (cmdv, ep);
	  if (!stat (cmdv, &mailfile))
	    {
#ifdef DEBUG
	      fprintf (stderr, "checking %s for mail\n", cmdv);
#endif
	      count = mailfile.st_size;
	    }
	}
    }
#ifdef DEBUG
  fprintf (stderr, "[%d] bytes mail\n", count);
#endif
  return count;
}

int 
dcp (char *dest, char *src, char *mo)
{
  char cpbuf[CPBUF];
  FILE *de, *sr;
  int count;

  struct stat sdes;
  struct stat ssrc;

  if (stat (src, &ssrc))
    {
#ifdef DEBUG
      fprintf (stderr, "Could not stat file <%s>\n", src);
#endif
      return 1;
    }
  else
    {
      if (!stat (dest, &sdes))
	{
	  if (ssrc.st_ino == sdes.st_ino)
	    {
#ifdef DEBUG
	      fprintf (stderr, "Can not copy file on to of itself \n");
#endif
	      return 1;
	    }
	}
#ifdef DEBUG
      fprintf (stderr, "Called dcp()...\n");
#endif
      de = fopen (dest, mo);
      if (de != NULL)
	{
	  sr = fopen (src, "r");
	  if (sr != NULL)
	    {
	      while ((count = fread (cpbuf, sizeof (char), CPBUF, sr)) > 0)
		{
#ifdef DEBUG
		  fprintf (stderr, "Writing...");
#endif
		  if (count > fwrite (cpbuf, sizeof (char), count, de))
		    {
		      plerr (PLINS);
		    }
		}
#ifdef DEBUG
	      fprintf (stderr, "DONE Writing...\n");
#endif
	      fclose (sr);
	    }
	  else
	    {
#ifdef DEBUG
	      fprintf (stderr, "Open of <%s> failed\n", src);
#endif
	      return 1;
	    }
	  fclose (de);
	}
      else
	{
#ifdef DEBUG
	  fprintf (stderr, "Open of <%s> failed\n", dest);
#endif
	  return 1;
	}
    }
  return 0;
}

int 
always ()
{
  char *tptr;
  int i;
  char tmpstr[32];

/* set the termcap strings */
  rsstr = NULL;
  sostr = NULL;
  sestr = NULL;
  hostr = NULL;

  k1str = NULL;
  k3str = NULL;
  kustr = NULL;
  krstr = NULL;
  klstr = NULL;
  kdstr = NULL;

  fsstr = NULL;
  tsstr = NULL;

/* set default flags      */
  testtrue = 0;
  running = 1;
  calldep = 0;
  quitstat = 0;
  doodle = 0;
  doexec = 0;
  dobatch = 0;
  niceval = DEFNICE;
  timeout = DEFTMOUT;
  junior = 0;

  llines = DEFLINES;
  ccols = DEFCOLS;

  time (&wtmpy);
  time (&maily);

  goptall = 0;
  goptpage = 0;
  goptwide = 0;
  goptcomp = 0;
  goptbrie = 0;
  goptconf = 0;
  goptprom = 0;

  nomap = 0;
  nochange = 0;
  noset = 0;
  noalias = 0;
  nobatch = 0;
  nopath = 0;
  notrap = 0;


  cmdvc = CMDV;
  cmdv = (char *) malloc (sizeof (char) * CMDV);
  if (cmdv == NULL)
    {
      plerr (PLMAE);
      plerr (PLINK);
#ifdef DEBUG
      fprintf (stderr, "Wow -- malloc failed -- resorting to desperate measures\n");
#endif
      exit (127);
    }

  cmdcc = CMDC;
  cmdc = (char **) malloc (sizeof (char *) * CMDC);
  if (cmdc == NULL)
    {
      plerr (PLMAE);
      plerr (PLINK);
#ifdef DEBUG
      fprintf (stderr, "Wow -- malloc failed -- resorting to desperate measures\n");
#endif
      exit (127);
    }

/* no drive yet defined   */
  drivecur = (-1);

/* set up shell levels    */
  tptr = getenv (STRSHLVL);
  if (tptr != NULL)
    {
      sprintf (tmpstr, "%d", atoi (tptr) + 1);
      setenv (STRSHLVL, tmpstr, 1);
    }
  else
    {
      setenv (STRSHLVL, "1", 1);
    }

/* set up a normal prompt */
  tptr = getenv (STRPROMPT);
  if (tptr != NULL)
    {
      if (strlen (tptr) < PROZ)
	strcpy (proz, tptr);
      else
	strcpy (proz, DEFPROMPT);
    }
  else
    {
      tptr = getenv (STRALTPROMPT);
      if (tptr != NULL)
	{
	  if (strlen (tptr) < PROZ)
	    strcpy (proz, tptr);
	  else
	    strcpy (proz, DEFPROMPT);
	}
      else
	{
	  tptr = getenv ("PS1");
	  if (tptr != NULL)
	    {
	      if (strlen (tptr) < PROZ)
		{
		  strcpy (proz, tptr);
		  i = 0;
		  while (proz[i] != '\0')
		    {
		      switch (proz[i])
			{
			case '`':
			case '\\':
			case '"':
			case '\'':
			  proz[i] = '$';
			  break;
			}
		      i++;
		    }
		}
	      else
		{
		  strcpy (proz, DEFPROMPT);
		}
	    }
	  else
	    {
	      strcpy (proz, DEFPROMPT);
#ifdef DEBUG
	      fprintf (stderr, "Are we are root ?\n");
#endif
	      if (!getuid ())
		strcat (proz, "# ");
	    }
	}
    }

/* have the alias chain set up correctly */
  head = NULL;
  return 0;
}

int 
onquit ()
{
/* what to do when exiting */
  cleanalias ();
  free (cmdv);
  free (cmdc);
  return 0;
}

/* run a "batch" file */

int 
cmdrun (char *fname, FILE * out)
{
  FILE *fp;
  int retval = 0;
  int isatermi = 0;
  char inppp[128];

#ifdef DEBUG
  fprintf (stderr, "About to open %s\n", fname);
#endif

  calldep++;

  if (NULL != (fp = fopen (fname, "r")))
    {
#ifdef DEBUG
      fprintf (stderr, "Could open %s\n", fname);
#endif

      if ((isatermi = isatty (fileno (fp))))
	{
	  printprompt (drivecur, proz, out);
	}

      while ((NULL != fgets (inppp, 128, fp)) && running)
	{
#ifdef DEBUG
	  fprintf (stderr, "Running command %s\n", inppp);
#endif
	  lex (inppp, 0);
	  stmts (0, tokencount, tokenstream, fp, out);
/* hmmm, should I do a printprompt if isatty ? */
	  if (isatermi)
	    {
	      if (tokencount)
		fputc ('\n', out);
	      printprompt (drivecur, proz, out);
	    }
	}
      fclose (fp);
    }
  else
    {
      retval = 1;
    }

  calldep--;
  return retval;
}

int 
introspect ()
{
  char *ptr;
  int i;

#ifdef USE_TERMCAP
  char *tptr;
  char tmp[10];
#endif

#ifdef DEBUG
  fprintf (stderr, "called introspect\n");
#endif

  ptr = getenv (STRNOFREEDOM);
  if (ptr != NULL)
    {
      nomap = 1;
      nochange = 1;
      noset = 1;
      noalias = 1;
      nobatch = 1;
      nopath = 1;
      notrap = 1;
    }
  else
    {
      ptr = getenv (STRNICE);
      if (ptr != NULL)
	{
	  niceval = atoi (ptr);
	}

      ptr = getenv (STRNOMAP);
      if (ptr != NULL)
	nomap = 1;
      else
	nomap = 0;

      ptr = getenv (STRNOCHANGE);
      if (ptr != NULL)
	nochange = 1;
      else
	nochange = 0;

      ptr = getenv (STRNOSET);
      if (ptr != NULL)
	noset = 1;

      ptr = getenv (STRNOALIAS);
      if (ptr != NULL)
	noalias = 1;
      else
	noalias = 0;

      ptr = getenv (STRNOBATCH);
      if (ptr != NULL)
	nobatch = 1;
      else
	nobatch = 0;

      ptr = getenv (STRNOPATH);
      if (ptr != NULL)
	nopath = 1;
      else
	nopath = 0;

      ptr = getenv (STRNOTRAP);
      if (ptr != NULL)
	notrap = 1;
      else
	notrap = 0;

    }

  goptall = 0;
  goptpage = 0;
  goptwide = 0;
  goptbrie = 0;
  goptcomp = 0;
  ptr = getenv (STRDIR);
  if (ptr != NULL)
    {
      i = 0;
      while (ptr[i] != '\0')
	{
	  switch (ptr[i])
	    {
	    case 'A':
	    case 'a':
	      goptall = 1;
	      break;
	    case 'P':
	    case 'p':
	      goptpage = 1;
	      break;
	    case 'W':
	    case 'w':
	      goptwide = 1;
	      break;
	    case 'C':
	    case 'c':
	      goptcomp = 1;
	      break;
	    case 'B':
	    case 'b':
	      goptbrie = 1;
	      break;
	    }
	  i++;
	}
    }

  goptconf = 0;
  ptr = getenv (STRREN);
  if (ptr != NULL)
    {
      i = 0;
      while (ptr[i] != '\0')
	{
	  switch (ptr[i])
	    {
	    case 'C':
	    case 'c':
	    case 'I':
	    case 'i':
	    case 'P':
	    case 'p':
	      goptconf = 1;
	      break;
	    }
	  i++;
	}
    }

  goptprom = 0;
  ptr = getenv (STRDEL);
  if (ptr != NULL)
    {
      i = 0;
      while (ptr[i] != '\0')
	{
	  switch (ptr[i])
	    {
	    case 'C':
	    case 'c':
	    case 'I':
	    case 'i':
	    case 'P':
	    case 'p':
	      goptprom = 1;
	      break;
	    }
	  i++;
	}
    }

  ptr = getenv (STRUMASK);
  if (ptr != NULL)
    {
#ifdef DEBUG
      fprintf (stderr, "ptr[0]-'0'=%d,ptr[1]-'0'=%d,ptr[2]-'0'=%d\n", ptr[0] - '0', ptr[1] - '0', ptr[2] - '0');
#endif
      i = (64 * (ptr[0] - '0')) + (8 * (ptr[1] - '0')) + (ptr[2] - '0');
#ifdef DEBUG
      fprintf (stderr, "Going to set umask to %o\n", i);
#endif
      umask (i);
    }

  timeout = DEFTMOUT;
  ptr = getenv (STRTMOUT);
  if (ptr != NULL)
    {
      timeout = atol (ptr);
#ifdef DEBUG
      fprintf (stderr, "Timeout is %ld\n", timeout);
#endif
    }

  mistr = getenv ("MAIL");

  lshout = getenv (STRLSHOUT);
  if (lshout == NULL)
    {
      lshout = STRDEFLSHOUT;
    }

  funny = 0;
  ptr = getenv (STRFUNNY);
  if (ptr != NULL)
    {
      funny = atoi (ptr);
#ifdef DEBUG
      fprintf (stderr, "Funny is %d\n", funny);
#endif
      srand (funny + mailbytes + quitstat + 1);
    }

  prevchar = DEFPREVCHAR;
  ptr = getenv (STRPREVCHAR);
  if (ptr != NULL)
    {
      prevchar = ptr[0];
    }

  ptr = getenv (STRTESTTRUE);
  if (ptr != NULL)
    {
      testtrue = atoi (ptr);
#ifdef DEBUG
      fprintf (stderr, "True is %d\n", testtrue);
#endif
    }
  else
    {
      testtrue = 0;
    }

#ifdef USE_TERMCAP
  tptr = getenv (STRTERM);
  if (tptr != NULL)
    {
      if (bootcap (tptr))
	{
	  tptr = NULL;
	}
    }
  else
    {
      if (isatty (STDIN_FILENO))
	{
	  tptr = ttyname (STDIN_FILENO);
	  if (tptr != NULL)
	    {
	      if (strstr (tptr, "ttyp") != NULL)
		{
		  if (bootcap ("dumb"))
		    {
		      tptr = NULL;
		    }
		  setenv ("TERM", "dumb", 1);
		}
	      else
		{
		  if (bootcap ("linux"))
		    {
		      tptr = NULL;
		    }
		  setenv ("TERM", "linux", 1);
		}
	    }
	}
    }
#endif

  ptr = getenv (STRLINES);
  if (ptr != NULL)
    {
      llines = atoi (ptr);
#ifdef DEBUG
      fprintf (stderr, "llines is %d\n", llines);
#endif
    }
#ifdef USE_TERMCAP
  else
    {
      if (tptr != NULL)
	{
	  llines = tgetnum ("li");
	  if (llines == (-1))
	    llines = DEFLINES;
	  else
	    {
	      sprintf (tmp, "%d", llines);
	      setenv (STRLINES, tmp, 1);
	    }
	}
    }
#endif

  ptr = getenv (STRCOLS);
  if (ptr != NULL)
    {
      ccols = atoi (ptr);
#ifdef DEBUG
      fprintf (stderr, "ccols is %d\n", ccols);
#endif
    }
#ifdef USE_TERMCAP
  else
    {
      if (tptr != NULL)
	{
	  ccols = tgetnum ("co");
	  if (ccols == (-1))
	    ccols = DEFCOLS;
	  else
	    {
	      sprintf (tmp, "%d", ccols);
	      setenv (STRCOLS, tmp, 1);
	    }
	}
    }
#endif

  ptr = getenv (STRPROMPT);
  if (ptr != NULL)
    {
      if (strlen (ptr) < PROZ)
	strcpy (proz, ptr);
    }
  else
    {
      ptr = getenv (STRALTPROMPT);
      if (ptr != NULL)
	{
	  if (strlen (ptr) < PROZ)
	    strcpy (proz, ptr);
	}
    }

  return 0;
}

int 
plerr (int pcode)
{
  int fun = 0;

  if (funny)
    {
      fun = rand () % funny;
      if (fun)
	fun = 0;
      else
	fun = 1;
    }

  switch (pcode)
    {
    case PLBAD:
      fprintf (stderr, "Bad Command or file name");
      if (fun)
	fprintf (stderr, " - Go stand in the corner !");
      break;
    case PLTMP:
      fprintf (stderr, "Too many parameters");
      if (fun)
	fprintf (stderr, " make me nervous.");
      break;
    case PLRPM:
      fprintf (stderr, "Required parameter missing");
      if (fun)
	fprintf (stderr, " in action - send in search party !");
      break;
    case PLFNF:
      fprintf (stderr, "File not found");
      if (fun)
	fprintf (stderr, " - and I won't fake it.");
      break;
    case PLPNF:
      fprintf (stderr, "Path not found");
      if (fun)
	fprintf (stderr, " - I am lost.");
      break;
    case PLIND:
      fprintf (stderr, "Invalid directory");
      break;
    case PLMAE:
      fprintf (stderr, "Memory allocation error");
      break;
    case PLIDS:
      fprintf (stderr, "Invalid drive specification");
      break;
    case PLUCD:
      fprintf (stderr, "Unable to create directory");
      break;
    case PLACD:
      fprintf (stderr, "Access denied");
      if (fun)
	fprintf (stderr, " - Nana nanahh.");
      break;
    case PLIDN:
      fprintf (stderr, "Invalid path, not directory, \nor directory not empty");
      break;
    case PLPFN:
      fprintf (stderr, "Parameter format not correct");
      break;
    case PLOES:
      fprintf (stderr, "Out of environment space");
      break;
    case PLATR:
      fprintf (stderr, "Attempt to remove current directory");
      break;
    case PLSER:
      fprintf (stderr, "Syntax error");
      if (fun)
	fprintf (stderr, " - Clean up your grammar !");
      break;
    case PLPAK:
      if (fun)
	fprintf (stderr, "Enter a 12-digit prime number");
      else
	fprintf (stderr, "Press any key");
      fprintf (stderr, " to continue . . .");
      break;
    case PLSRE:
      fprintf (stderr, "System resource exhausted");
      if (fun)
	fprintf (stderr, ", tired and underpaid.");
      break;
    case PLINS:
      fprintf (stderr, "Insufficient disk space");
      break;
    case PLRTF:
      fprintf (stderr, "No online help implemented");
      if (fun)
	fprintf (stderr, " - go use a real shell !");
      break;
    case PLINK:
      fprintf (stderr, "General failure");
      if (fun)
	fprintf (stderr, " ? Failure, General failure, Sir !");
      break;
    case PLISW:
      fprintf (stderr, "Invalid switch");
      break;
    case PLFEX:
      fprintf (stderr, "File exists");
      break;
    case PLTER:
      fprintf (stderr, "Could not make sense of your terminal");
      break;

    }
  fputc ('\n', stderr);
  return 0;
}

#ifdef OODEBUG

int 
main ()
{
  dcp ("b", "a", "w");
  dcp ("c", "a", "a");
  dcp ("c", "b", "a");
  return 0;
}

#endif
