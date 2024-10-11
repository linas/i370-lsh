/* This is copyrighted software but you may use/distribute it     *
 * in accordance with the conditions set out in the file COPYING  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include "defs.h"

#define DEBUG

int
main (int argc, char **argv)
{
  int i, j;
  char cin[INBUF];
  char *ep;

#ifdef DEBUG
  printf ("Hey, /bin/sh says hello world!\n");
#endif 

/* Do essential initialization         */
  always ();
  pathinit ();
  syminit ();

  cin[0] = '\n';
  cin[1] = '\0';

/* Always have SOME drive mapped       */
  if (getcwd (cmdv, cmdvc) != NULL)
    {
      if (map (2, "/"))
	{
	  plerr (PLINK);
	  quitstat = 1;
	}
      else
	{
	  if (!chdir (cmdv))
	    {
	      if (setlo (2, cmdv))
		{
		  plerr (PLPNF);
		}
	    }
	  else
	    {
	      plerr (PLPNF);
	    }
	}
    }
  else
    {
      plerr (PLPNF);
      if (map (2, "/"))
	{
	  plerr (PLINK);
	  quitstat = 1;
	}
    }

/* If we are a login shell, set up shell level and trap TERM */
  if (argv[0][0] == '-' || getppid () == 1)
    {
      setenv (STRSHLVL, "1", 1);

      signal (SIGTERM, SIG_IGN);

/* check for mail                      */
      mailbytes = countmail ();
      if (mailbytes && isatty (STDOUT_FILENO))
	{
	  printf ("You have Mail [%d bytes]\n", mailbytes);
	}

    }

/* We should initialize if we start with a '-', are a child     */
/* of init or have been told by the variable in the environment */
  if (argv[0][0] == '-' || (getenv (STRNOUNINIT) != NULL) || getppid () == 1)
    {

#ifdef DEBUG
      fprintf (stderr, "We want to be initialized\n");
#endif

      sprintf (cmdv, "%d", getuid ());
      setenv ("UID", cmdv, 1);

      sprintf (cmdv, "%d", getgid ());
      setenv ("GID", cmdv, 1);

/* run the global autoexec first       */
      cmdrun ("/etc/autoexec", stdout);

/* run the autoexec for that group (redundant)   */
/*    sprintf(cmdv,"/etc/autoexec.%d",getgid()); */
/*    cmdrun(cmdv,stdout);                       */

/* check out any environment variables */
      introspect ();

/* Trap ^C                             */
      signal (SIGINT, SIG_IGN);

/* run the users autoexec here -       */
      ep = getenv ("HOME");
      if (ep != NULL)
	{

/* use an absolute path, relying on previous progs to set up HOME variable */
#ifdef DEBUG
	  fprintf (stderr, "Home dir is %s\n", ep);
#endif
	  strcpy (cmdv, ep);
#ifdef DEBUG
	  fprintf (stderr, "last character is <%c> -- is that a /?\n", cmdv[strlen (cmdv) - 1]);
#endif
	  if (cmdv[strlen (cmdv) - 1] == '/')
	    {
	      strcat (cmdv, "autoexec");
	    }
	  else
	    {
	      strcat (cmdv, "/autoexec");
	    }
#ifdef DEBUG
	  fprintf (stderr, "About to run %s\n", cmdv);
#endif
	  cmdrun (cmdv, stdout);

/* Now run the .autoexec file */
	  strcpy (cmdv, ep);
#ifdef DEBUG
	  fprintf (stderr, "last character is <%c> -- is that a /?\n", cmdv[strlen (cmdv) - 1]);
#endif
	  if (cmdv[strlen (cmdv) - 1] == '/')
	    {
	      strcat (cmdv, ".autoexec");
	    }
	  else
	    {
	      strcat (cmdv, "/.autoexec");
	    }
	  cmdrun (cmdv, stdout);

	}
      else
	{
/* if no HOME variable exits, then run the file in the current directory */
/* Could this somehow be a risk along the lines of .exrc ?               */
	  if (argv[0][0] == '-')
	    {
	      cmdrun ("autoexec", stdout);
	      cmdrun (".autoexec", stdout);
	    }
	}
    }
  else
    {
/* we are not a login shell, nor do we want to be initialized */
/* just check out any environment variables                   */
      introspect ();
    }

#ifdef DEBUG
  fprintf (stderr, "Got %d arguments\n", argc);
#endif

/* Note : Above initialization independant   */
/* of cmdline arguments                      */
  if (argc > 1)
    {
      ep = argv[1];
      if (ep[0] == '-')
	{
	  switch (ep[1])
	    {
	    case 'C':
	    case 'c':
	      running = 0;
	    case 'R':
	    case 'r':
	      if (argc > 2)
		{
		  j = 0;
		  for (i = 2; i < argc; i++)
		    {
		      if (strlen (argv[i]) < INBUF - j)
			{
			  strcpy (cin + j, argv[i]);
			  j += strlen (argv[i]);
			  cin[j] = ' ';
			  j++;
			}
		      else
			{
			  i = argc;
			  fputc ('\a', stdout);
			}
		    }
		  cin[j] = '\0';
#ifdef DEBUG
		  fprintf (stderr, "Will be running <%s> from the commandline\n", cin);
#endif
		  lex (cin, 0);
		  stmts (0, tokencount, tokenstream, stdin, stdout);
		}
	      break;
	    case 'K':
	    case 'k':
	      if (argc > 2)
		{
		  cmdrun (argv[2], stdout);
		}
	      break;
	    case 'H':
	    case 'h':
	    case '-':
	    case '?':
	      running = 0;
	      fprintf (stderr, "Usage: lsh [filename] [-c commands] [-h] [-k filename] [-r commands]\n");
	      break;
	    default:
	      plerr (PLPFN);
	      break;
	    }
	}
      else
	{
#ifdef DEBUG
	  fprintf (stderr, "Running file <%s>\n", ep);
#endif
	  cmdrun (ep, stdout);
	  running = 0;
	}
    }

  while (running)
    {

#ifdef DEBUG
      fprintf (stderr, "[%d] At top of main loop\n", getpid ());
#endif

      if (tokencount)
	fprintf (stdout, "\n");
      printprompt (drivecur, proz, stdout);

      if (timeout)
	{
	  alarm (timeout);
	}

      if (readl (cin, INBUF))
	{
	  running = 0;
	  fprintf (stdout, "\n");
	}
      else
	{
	  if (timeout)
	    {
	      alarm (0);
	    }
	  lex (cin, 0);
	  stmts (0, tokencount, tokenstream, stdin, stdout);
	}

#ifdef DEBUG
      fprintf (stderr, "[%d] At end of main loop... running [%d]\n", getpid (), running);
#endif
    }

#ifdef DIAGNOSE
  pathdump (stdout);
  symdump ();
  tokendump ();
#endif

  onquit ();

  return quitstat;
}
