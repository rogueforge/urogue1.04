/*
 * Last Modified: 05/25/86
 */
#include	<stdio.h>
#include 	"tunable.h"
#include	<pwd.h>

#define	MAXAUTH		10

static struct  real_pass {
	char	rp_pass[20];
	char	rp_pkey[2];
} 
rpass;

char	password[20],passkey[2];
int	auth_or[MAXAUTH];
char	found;

main(argc, argv)
char **argv;
{
	register struct passwd *pw;
	struct passwd *getpwnam();
	struct passwd *getpwuid();
	char *getpass(), *crypt();
	char *getlogin();
	char *gets();
	char *user,ubuf[80];
	char *opt;
	FILE *perm,*pass;
	int  fdperm,fdpass, fdmotd, fdnews, fdscore;
	int	 i, j;
	int  iflag,pflag,aflag,dflag,cflag,sflag;

	if (argc < 2 || argv[1][0] != '-') {
		fprintf(stderr, "usage: %s -{capids}\n", *argv);
		exit(1);
	}

	user = ubuf;

	pw = getpwuid(getuid());
	if (pw == NULL) {
		fprintf(stderr, "Say, who are you?\n");
		exit(1);
	}

	if (pw->pw_uid != 0)
		if (pw->pw_uid != UID || strcmp(pw->pw_name, LOGIN) != 0) {
			fprintf(stderr, "You are not empowered to run this program\n");
			exit(1);
		}

	opt = argv[1];
	opt++;

	switch(*opt) {
	case	'i':
		iflag = 1;
		break;
	case	'p':
		pflag = 1;
		break;
	case	'a':
		aflag = 1;
		break;
	case	's':
		sflag = 1;
		break;
	case	'd':
		dflag = 1;
		break;
	case	'c':
		cflag = 1;
		break;
	default:
		fprintf(stderr, "usage: %s -{capids}\n", *argv);
		exit(1);
	}

	if ((perm = fopen(PERMOK, "r")) == NULL && !iflag) {
		perror(PERMOK);
		exit(1);
	}

	if ((pass = fopen(PASSCTL,"r")) == NULL && !iflag) {
		perror(PASSCTL);
		exit(1);
	}

	if(!iflag) {
		fdperm = fileno(perm);
		fdpass = fileno(pass);
		encread(rpass.rp_pkey, sizeof(rpass.rp_pkey), fdpass);
		encread(rpass.rp_pass, sizeof(rpass.rp_pass), fdpass);
		encread(auth_or, sizeof(auth_or), fdperm);
	}
	else {
		if (perm == NULL) 
			if ((fdperm = creat(PERMOK, 0644)) < 0) {
				perror(PERMOK);
				exit(1);
			}
			else
				printf("File %s created.\n", PERMOK);

		if (pass == NULL) 
			if ((fdperm = creat(PASSCTL, 0644)) < 0) {
				perror(PASSCTL);
				exit(1);
			}
			else
				printf("File %s created.\n", PASSCTL);

		if (fopen(NEWS, "r") == NULL) 
			if ((fdnews = creat(NEWS, 0644)) < 0 ) {
				perror(NEWS);
				exit(1);
			}
			else {
				printf("File %s created.\n", NEWS);
				close(fdnews);
			}

		if (fopen(MOTD, "r") == NULL) 
			if ((fdmotd = creat(MOTD, 0644)) < 0 ) {
				perror(MOTD);
				exit(1);
			}
			else {
				printf("File %s created.\n", MOTD);
				close(fdmotd);
			}

		if (fopen(SCOREDIR, "r") == NULL) 
			if ((fdscore = creat(SCOREDIR, 0644)) < 0 ) {
				perror(SCOREDIR);
				exit(1);
			}
			else {
				printf("File %s created.\n", SCOREDIR);
				close(fdscore);
			}
		else {
			printf("Type 'y' and press RETURN to reset %s: ", SCOREDIR);
			if (*(gets(password)) == 'y') {
				if ((fdscore = creat(SCOREDIR, 0644)) < 0) {
					perror(SCOREDIR);
					exit(1);
				}
				close(fdscore);
				printf("File %s reset.\n", SCOREDIR);
			}
			else
				printf("File %s untouched.\n", SCOREDIR);
		}

		for(i = 0; i < MAXAUTH; i++)
			auth_or[i] = -10;
	}

	if(iflag) { 
		cflag = pflag = aflag = 1; 
		sflag = dflag = 0;
	}

	if(sflag) {
		printf("Type 'y' and press RETURN to reset %s: ", SCOREDIR);
		if (*(gets(password)) == 'y') {
			if ((fdscore = creat(SCOREDIR, 0644)) < 0) {
				perror(SCOREDIR);
				exit(1);
			}
			close(fdscore);
			printf("File %s reset.\n", SCOREDIR);
		}
		else
			printf("File %s untouched.\n", SCOREDIR);
	}

	if(aflag) {
		printf("Users currently authorized:\n");
		for(i = 0; i < MAXAUTH; i++)
			if(auth_or[i] >= 0) {
				pw = getpwuid(auth_or[i]);
				printf("%s\n", pw->pw_name);
			}

		do {
			printf("Enter user to be authorized: ");
			gets(user);
			pw = getpwnam(user);
			if (pw == NULL) {
				printf("User %s not found!\n", user);
				continue;
			}
			j = pw->pw_uid;
			for(i = 0; i < MAXAUTH; i++)
				if((auth_or[i] < 0) || (auth_or[i] == j))
					break;

			if(auth_or[i] == j) {
				printf("User %s: previously authorized!\n",
				user);
				printf("Authorize another one? ");
				continue;
			}
			if(auth_or[i] >= 0) {
				printf("No MORE SLOTS!\n");
				break;
			}
			auth_or[i] = j;
			printf("User authorized: %s\n", user);
			printf("Authorize another one? ");
		} 
		while (*(gets(password)) == 'y');
	}
	if(dflag) {
		printf("Users currently authorized:\n");
		for(i = 0; i < MAXAUTH; i++)
			if(auth_or[i] >= 0) {
				pw = getpwuid(auth_or[i]);
				printf("%s\n", pw->pw_name);
			}

		do {
			found = 0;
			printf("Enter user to be denied: ");
			gets(user);
			pw = getpwnam(user);
			j = pw->pw_uid;
			for(i = 0; i < MAXAUTH; i++)
				if(auth_or[i] == j) {
					printf("Deleting: %s\n", user);
					found = 1;
					auth_or[i] = -10;
					break;
				}
			if(!found)
				printf("No such user found!\n");
			printf("Another deletion? ");
		} 
		while(*(gets(password)) == 'y');
	}
	if(pflag) {
		strncpy(password,getpass("Enter password:"),20);
		strncpy(rpass.rp_pkey,getpass("Enter key:"),2);
		strncpy(rpass.rp_pass,crypt(password,rpass.rp_pkey),20);
	}

	if((cflag) || (pflag)) {
		printf("Check of array:\n");
		for(i = 0; i < MAXAUTH; i++)
			if(auth_or[i] >= 0) {
				pw = getpwuid(auth_or[i]);
				printf("%s\n", pw->pw_name);
			}
	}
	if(pflag || aflag || dflag) {
		if(perm != NULL)
			fclose(perm);
		else
			close(fdperm);
		if(pass != NULL)
			fclose(pass);
		else
			close(fdpass);
		if((perm = fopen(PERMOK,"w")) == NULL ||
		    (pass = fopen(PASSCTL,"w")) == NULL) {
			printf("This is futile!\n");
			exit(0);
		}
		fseek(perm,0L,0);
		fseek(pass,0L,0);
		encwrite(rpass.rp_pkey, sizeof(rpass.rp_pkey), pass);
		encwrite(rpass.rp_pass, sizeof(rpass.rp_pass), pass);
		encwrite((char *) auth_or, sizeof(auth_or), perm);
	}
}

/*****************************************************************
 *
 *           encwrite, encread: encoded read/write routines
 *
 * The encwrite/encread routines implement a synchronous stream Vernam
 * cipher using a linear congruential random number generator to
 * simulate a one-time pad.  The random seed is encoded and stored in
 * the file using data diffusion (putword,getword).
 *
 * Usage:
 *	encwrite (start, size, outf);
 *	char *start; int size; FILE *outf;
 *
 *	encread (start, size, infd);
 *	char *start; int size; int infd;
 *
 * HISTORY
 * 03-Mar-85  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Modified for UltraRogue.  Allowed multiple encwrites to a file by
 *	removing lseek calls and computing checksum while reading.
 *
 * 20-Dec-82  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Created as a replacement for the encwrite/encread in
 *	Rogue 5.2, which are very easily broken.
 *
 *****************************************************************/

# include "stdio.h"

/* Constants for key generation */
# define OFFSET		667818
# define MODULUS	894871
# define MULT		2399
# define ENCHAR		(((seed= ((seed*MULT+OFFSET)%MODULUS)) >> 10) & 0xff)

/* Constants for checksumming */
# define INITCK		1232531
# define CKMOD		6506347

# define min(A,B)  	(((A)<(B)?(A):(B)))

struct wb { 
	int w1, w2, w3, w4; 
};

/*****************************************************************
 * Encwrite: write a buffer to a file using data encryption
 *****************************************************************/

encwrite (start, size, outf)
register char *start;
int size;
register FILE *outf;
{ 
	register int cksum = INITCK, seed, ch;

	srand (time (0) + getpid () + getuid ());	/* Build a random seed */
	seed = (rand () & 0x7fff);

	putword (seed, outf);				/* Write the random seed */
	putword (size, outf);				/* Write the file length */

# ifdef DEBUG
	fprintf (stderr, "Encwrite: size %ld, seed %ld.\n", size, seed);
# endif

	/* Now checksum, encrypt, and write out the buffer. */
	while (size--)
	{ 
		ch = *start++ & 0xff;			/* Get the next char */
		cksum = ((cksum << 8) + ch) % CKMOD;	/* Checksum clear text */
		putc (ch ^ ENCHAR, outf);			/* Write ciphertext */
	}

	putword (cksum, outf);			/* Write out the checksum */
	fflush (outf);				/* Flush the output */

# ifdef DEBUG
	fprintf (stderr, "Checksum is %ld.\n", cksum);
# endif
}

/*****************************************************************
 * Encread: read a block of encrypted text from a file descriptor
 *****************************************************************/

encread (start, size, infd)
register char *start;
int size;
int infd;
{ 
	register int cksum = INITCK, seed, length, cnt, ch;
	int stored, getword();

	seed = getword (infd); 
	stored = getword (infd);  

# ifdef DEBUG
	fprintf (stderr, "Encread: size %ld, seed %ld, stored %ld.\n", 
	size, seed, stored);
# endif

	if ((length = read (infd, start, (int) min (size, stored))) > 0)
	{ 
		for (cnt = length; cnt--; ) 
		{ 
			ch = (*start++ ^= ENCHAR) & 0xff;
			cksum = ((cksum << 8) + ch) % CKMOD;
		}

		if ((length == stored) && (getword (infd) != cksum))
		{
# ifdef DEBUG
			fprintf (stderr, "Computed checksum %ld is wrong.\n", cksum);
# else
			fprintf (stderr, "Sorry, file has been touched.\n");
# endif
			while (length--) *--start = '\0';		/* Zero the buffer */
		}
	}

	return (length);
}

/*****************************************************************
 * putword: Write out an encoded int word
 *****************************************************************/

putword (word, file)
int word;
FILE *file;
{ 
	struct wb w;

	w.w1 = rand ();  
	w.w2 = rand ();  
	w.w3 = rand ();  
	w.w4 = w.w1 ^ w.w2 ^ w.w3 ^ word;

	return (fwrite ((char *) &w, sizeof (struct wb), 1, file));
}

/*****************************************************************
 * getword: Read in an encoded int word
 *****************************************************************/

getword (fd)
int fd;
{ 
	struct wb w;

	if (read (fd, (char *) &w, sizeof (struct wb)) == sizeof (struct wb))
		return (w.w1 ^ w.w2 ^ w.w3 ^ w.w4);
	else
		return (0);
}
