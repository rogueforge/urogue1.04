/*
 * Last Modified: 11/03/86
 */
#include <stdio.h>
#include "rogue.h"

static char *progname;

int multi = FALSE;

main(argc, argv)
int argc;
char *argv[];
{
	register int x;

	if(argc > 2)
		multi = TRUE;

	progname = *argv;

	for (x = 1; x < argc; x++) {
		if (argv[x][0] != '-')
			break;
		switch (argv[x][1]) {
			case 'a':
				pr_armour();
				break;
			case 'm':
				pr_monsters();
				break;
			case 'i':
			case 'A':
				multi = TRUE;
				pr_armour();
				pr_scrolls();
				pr_rings();
				pr_potions();
				pr_food();
				pr_sticks();
				pr_weapons();
				break;
			case 'w':
				pr_weapons();
				break;
			case 's':
				pr_scrolls();
				break;
			case 'r':
				pr_rings();
				break;
			case 'p':
				pr_potions();
				break;
			case 'f':
				pr_food();
				break;
			case 'S':
				pr_sticks();
				break;
			default:
				usage();
		}
	}

	if (argc <= 1) {
		usage();
		exit(1);
	}
}

usage()
{
	fprintf(stderr, "usage: %s -afsSprmw [ -A ]\n", progname);
	fprintf(stderr, "\t-a\tArmor\n");
	fprintf(stderr, "\t-f\tFood\n");
	fprintf(stderr, "\t-s\tScrolls\n");
	fprintf(stderr, "\t-S\tSticks\n");
	fprintf(stderr, "\t-p\tPotions\n");
	fprintf(stderr, "\t-r\tRings\n");
	fprintf(stderr, "\t-m\tMonsters\n");
	fprintf(stderr, "\t-w\tWeapons\n");
	fprintf(stderr, "\t-A\tPrint all of the above (except monsters).\n");
}

extern struct init_armor armors[];

pr_armour()
{
    register int i;

    printf("%25s   %%  AC Weight Worth\n", "Armor");

    for (i=0; i<maxarmors; i++) {

		printf("%25.25s ", armors[i].a_name);
		printf("%3d ", armors[i].a_prob);
		printf(" %2d ", armors[i].a_class);
		printf(" %3d ", armors[i].a_wght);
		printf(" %5d ", armors[i].a_worth);

		printf("\n");
    }
	if(multi)
		putchar('\n');
}

pr_scrolls()
{
    register int i;
    float q;

    printf("%25s   %%   Curse Bless Worth\n", "Scrolls");
    for (i=0; i<maxscrolls; i++) {

		printf("%25.25s  ", s_magic[i].mi_name);
		q = s_magic[i].mi_prob  / 10.0;
		printf("%-3.1f	", q);
		printf(" %3d ", s_magic[i].mi_curse);
		printf(" %3d ", s_magic[i].mi_bless);
		printf(" %5d ", s_magic[i].mi_worth);
	
		printf("\n");
    }
	if(multi)
		putchar('\n');
}

pr_potions()
{
    register int i;
    float q;

    printf("%25s   %%   Curse Bless Worth\n", "Potions");
    for (i=0; i<maxpotions; i++) {

		printf("%25.25s  ", p_magic[i].mi_name);
		q = p_magic[i].mi_prob  / 10.0;
		printf("%-3.1f	", q);
		printf(" %3d ", p_magic[i].mi_curse);
		printf(" %3d ", p_magic[i].mi_bless);
		printf(" %5d ", p_magic[i].mi_worth);
	
		printf("\n");
    }
	if(multi)
		putchar('\n');
}

pr_rings()
{
    register int i;
    float q;

    printf("%25s   %%   Curse Bless Worth\n", "Rings");
    for (i=0; i<maxrings; i++) {

		printf("%25.25s  ", r_magic[i].mi_name);
		q = r_magic[i].mi_prob  / 10.0;
		printf("%-3.1f	", q);
		printf(" %3d ", r_magic[i].mi_curse);
		printf(" %3d ", r_magic[i].mi_bless);
		printf(" %5d ", r_magic[i].mi_worth);
	
		printf("\n");
    }
	if(multi)
		putchar('\n');
}

pr_food()
{
    register int i;
    float q;

    printf("%25s   %%   Curse Bless Worth\n", "Food");
    for (i=0; i<maxfoods; i++) {

		printf("%25.25s  ", fd_data[i].mi_name);
		q = fd_data[i].mi_prob  / 10.0;
		printf("%-3.1f	", q);
		printf(" %3d ", fd_data[i].mi_curse);
		printf(" %3d ", fd_data[i].mi_bless);
		printf(" %5d ", fd_data[i].mi_worth);
	
		printf("\n");
	}
	if(multi)
		putchar('\n');
}

pr_sticks()
{
    register int i;
    float q;

    printf("%25s   %%   Curse Bless Worth\n", "Sticks");
    for (i=0; i<maxsticks; i++) {

		printf("%25.25s  ", ws_magic[i].mi_name);
		q = ws_magic[i].mi_prob  / 10.0;
		printf("%-3.1f	", q);
		printf(" %3d ", ws_magic[i].mi_curse);
		printf(" %3d ", ws_magic[i].mi_bless);
		printf(" %5d ", ws_magic[i].mi_worth);
	
		printf("\n");
    }
	if(multi)
		putchar('\n');
}

extern struct monster monsters[];

pr_monsters()
{

		/*
		CARRY_PROB,	NORMAL,	WANDER,	SYMBOL,	INTELLIGENCE,
		{ATTRIBUTE_FLAGS},
		CREATURE_SUMMONED, NUMBER_SUMMONED,
		ADDED_EXPERIENCE/HIT_POINT,
		{str,	exp_pts, exp_level, armor_class, hit_points,
		"damage"}},
		*/

    register int i;
    register int flag, j, k;

    printf(
	"Number%30s Letter  Int Str  Lvl    Exp(X) Tres  AC Flags\n%64s%12s\n",
		"Monster", "HP", "Damage");

    for (i=1; i<nummonst+2; i++) {
		printf("%4d) %30.30s   '%c' %5s %2d %c",
			i,
		    monsters[i].m_name,	
		    monsters[i].m_appear,
		    monsters[i].m_intel,
		    monsters[i].m_stats.s_str,	
		    (monsters[i].m_wander ? 'W' : ' ')
		    );
	
		printf(" %2d ", monsters[i].m_stats.s_lvl);
		printf(" %5d", monsters[i].m_stats.s_exp);
	
		if (monsters[i].m_add_exp != 0)
		    printf("(%2d)", monsters[i].m_add_exp);
		else 
		    printf("    ");
	
		if (monsters[i].m_carry != 0)
		    printf("%3d%%", monsters[i].m_carry);
		else 
		    printf("    ");
	
		printf("   %2d ", monsters[i].m_stats.s_arm);
	
		find_flags(i);
	
		/* Second line */
		printf("\n%64.5s %10s",
		    monsters[i].m_stats.s_hpt,	
		    monsters[i].m_stats.s_dmg
		    );
	
		if (monsters[i].m_typesum != 0)
		    printf("%-20d(%s)", 
		    	monsters[i].m_numsum, 
		    	monsters[i].m_typesum);
	
		printf("\n");
    }
	if(multi)
		putchar('\n');
}

find_flags(i)
register int i;
{
    register int k, flag;

    for (k=0; k<16; k++) {
			if ( (flag=monsters[i].m_flags[k]) != 0)
			{
					switch(flag) {
					case ISBLIND:		printf(" BLIND");			break;
					case ISINWALL:		printf(" INWALL");			break;
					case ISRUN:			printf(" RUN");				break;
					case ISFLEE:		printf(" FLEE");			break;
					case ISINVIS:		printf(" INVIS");			break;
					case ISMEAN:		printf(" M");				break;
					case ISGREED:		printf(" GREEDY");			break;
					case CANSHOOT:		printf(" SHOOT");			break;
					case ISHELD:		printf(" HELD");			break;
					case ISHUH:			printf(" HUH");				break;
					case ISREGEN:		printf(" REGEN");			break;
					case CANHUH:		printf(" HUH");				break;
					case CANSEE:		printf(" SEE");				break;
					case HASFIRE:		printf(" FIRE");			break;
					case ISSLOW:		printf(" SLOW");			break;
					case ISHASTE:		printf(" HASTE");			break;
					case ISCLEAR:		printf(" CLEAR");			break;
					case CANINWALL:		printf(" INWALL");			break;
					case ISDISGUISE:	printf(" DISGUISE");		break;
					case CANBLINK:		printf(" BLINK");			break;
					case CANSNORE:		printf(" SNORE");			break;
					case HALFDAMAGE:	printf(" HALFDAMAGE");		break;
					case CANSUCK:		printf(" SUCK");			break;
					case CANRUST:		printf(" RUST");			break;
					case CANPOISON:		printf(" POISON");			break;
					case CANDRAIN:		printf(" DRAIN");			break;
					case ISUNIQUE:		printf(" UNIQUE");			break;
					case STEALGOLD:		printf(" STEALGOLD");		break;
					case STEALMAGIC:	printf(" STEALMAGIC");		break;
					case CANDISEASE:	printf(" DISEASE");			break;
					case HASDISEASE:	printf(" DISEASE");			break;
					case CANSUFFOCATE:	printf(" SUFFOCATE");		break;
					case DIDSUFFOCATE:	printf(" DIDSUFFOCATE");	break;
					case BOLTDIVIDE:	printf(" BOLTDIVIDE");		break;
					case BLOWDIVIDE:	printf(" BLOWDIVIDE");		break;
					case NOCOLD:		printf(" NOCOLD");			break;
					case TOUCHFEAR:		printf(" TOUCHFEAR");		break;
					case BMAGICHIT:		printf(" M_+_HIT");			break;
					case NOFIRE:		printf(" NOFIRE");			break;
					case NOBOLT:		printf(" NOBOLT");			break;
					case CARRYGOLD:		printf(" CARRYGOLD");		break;
					case CANITCH:		printf(" ITCH");			break;
					case HASITCH:		printf(" ITCH");			break;
					case DIDDRAIN:		printf(" DIDDRAIN");		break;
					case WASTURNED:		printf(" WASTURNED");		break;
					case CANSELL:		printf(" SELL");			break;
					case CANBLIND:		printf(" BLIND");			break;
					case CANBBURN:		printf(" BBURN");			break;
					case ISCHARMED:		printf(" CHARMED");			break;
					case CANSPEAK:		printf(" SPEAK");			break;
					case CANFLY:		printf(" FLY");				break;
					case ISFRIENDLY:	printf(" FRIENDLY");		break;
					case CANHEAR:		printf(" HEAR");			break;
					case ISDEAF:		printf(" DEAF");			break;
					case CANSCENT:		printf(" SCENT");			break;
					case ISUNSMELL:		printf(" UNSMELL");			break;
					case WILLRUST:		printf(" RUST");			break;
					case WILLROT:		printf(" ROT");				break;
					case SUPEREAT:		printf(" SUPEREAT");		break;
					case PERMBLIND:		printf(" PERMBLIND");		break;
					case MAGICHIT:		printf(" M_HIT");			break;
					case CANINFEST:		printf(" INFEST");			break;
					case HASINFEST:		printf(" INFEST");			break;
					case NOMOVE:		printf(" NOMOVE");			break;
					case CANSHRIEK:		printf(" SHRIEK");			break;
					case CANDRAW:		printf(" DRAW");			break;
					case CANSMELL:		printf(" SMELL");			break;
					case CANPARALYZE:	printf(" PARALYZE");		break;
					case CANROT:		printf(" ROT");				break;
					case ISSCAVENGE:	printf(" SCAVENGE");		break;
					case DOROT:			printf(" ROT");				break;
					case CANSTINK:		printf(" STINK");			break;
					case HASSTINK:		printf(" STINK");			break;
					case ISSHADOW:		printf(" SHADOW");			break;
					case CANCHILL:		printf(" CHILL");			break;
					case CANHUG:		printf(" HUG");				break;
					case CANSURPRISE:	printf(" SURPRISE");		break;
					case CANFRIGHTEN:	printf(" FRIGHTEN");		break;
					case CANSUMMON:		printf(" SUMMON");			break;
					case TOUCHSTONE:	printf(" TOUCHSTONE");		break;
					case LOOKSTONE:		printf(" LOOKSTONE");		break;
					case CANHOLD:		printf(" HOLD");			break;
					case DIDHOLD:		printf(" DIDHOLD");			break;
					case DOUBLEDRAIN:	printf(" DOUBLEDRAIN");		break;
					case ISUNDEAD:		printf(" UNDEAD");			break;
					case BLESSMAP:		printf(" B_MAP");			break;
					case BLESSGOLD:		printf(" B_GOLD");			break;
					case BLESSMONS:		printf(" B_MONS");			break;
					case BLESSMAGIC:	printf(" B_MAGIC");			break;
					case BLESSFOOD:		printf(" B_FOOD");			break;
					case CANBRANDOM:	printf(" B_RANDOM");		break;
					case CANBACID:		printf(" B_ACID");			break;
					case CANBFIRE:		printf(" B_FIRE");			break;
					case CANBBOLT:		printf(" B_BOLT");			break;
					case CANBGAS:		printf(" B_GAS");			break;
					case CANBICE:		printf(" B_ICE");			break;
					case CANBPGAS:		printf(" B_PGAS");			break;
					case CANBSGAS:		printf(" B_SGAS");			break;
					case CANBSLGAS:		printf(" B_SLGAS");			break;
					case CANBFGAS:		printf(" B_FGAS");			break;
					case CANBREATHE:	printf(" BREATHE");			break;
					case STUMBLER:		printf(" STUMBLER");		break;
					case POWEREAT:		printf(" P_EAT");			break;
					case ISELECTRIC:	printf(" ELECTRIC");		break;
					case HASOXYGEN:		printf(" OXYGEN");			break;
					case POWERDEXT:		printf(" P_DEXT");			break;
					case POWERSTR:		printf(" P_STR");			break;
					case POWERWISDOM:	printf(" P_WISDOM");		break;
					case POWERINTEL:	printf(" P_INTEL");			break;
					case POWERCONST:	printf(" P_CONST");			break;
					case SUPERHERO:		printf(" SUPERHERO");		break;
					case ISUNHERO:		printf(" UNHERO");			break;
					case CANCAST:		printf(" CAST");			break;
					case CANTRAMPLE:	printf(" TRAMPLE");			break;
					case CANSWIM:		printf(" SWIM");			break;
					case LOOKSLOW:		printf(" LOOKSLOW");		break;
					case CANWIELD:		printf(" WIELD");			break;
					case CANDARKEN:		printf(" DARKEN");			break;
					case ISFAST:		printf(" FAST");			break;
					case CANBARGAIN:	printf(" BARGAIN");			break;
					case NOMETAL:		printf(" NOMETAL");			break;
					case CANSPORE:		printf(" SPORE");			break;
					case NOSHARP:		printf(" NOSHARP");			break;
					case DRAINWISDOM:	printf(" D_WISDOM");		break;
					case DRAINBRAIN:	printf(" D_BRAIN");			break;
					case ISLARGE:		printf(" LARGE");			break;
					case ISSMALL:		printf(" SMALL");			break;
					case CANSTAB:		printf(" STAB");			break;
					case ISFLOCK:		printf(" FLOCK");			break;
					case ISSWARM:		printf(" SWARM");			break;
					case CANSTICK:		printf(" STICK");			break;
					case CANTANGLE:		printf(" TANGLE");			break;
					case DRAINMAGIC:	printf(" D_ENCHANT");		break;
					case SHOOTNEEDLE:	printf(" NEEDLE");		break;
					case CANZAP:		printf(" ELECTRIC_ZAP");	break;
					case HASARMOR:		printf(" HASARMOR");		break;
					case CANTELEPORT:	printf(" TELEPORT");		break;
					case ISBERSERK:		printf(" BERSERK");		break;
					case ISFAMILIAR:	printf(" FAMILIAR");		break;
					case HASFAMILIAR:	printf(" HASFAMILIAR");		break;
					case SUMMONING:		printf(" SUMMONING");		break;
					case CANREFLECT:	printf(" REFLECT");		break;
					case LOWFRIENDLY:	printf(" L_FRIEND");		break;
					case HIGHFRIENDLY:	printf(" H_FRIEND");		break;
					case MAGICATTRACT:	printf(" M_ATTRACT");		break;
					case ISGOD:		printf(" *GOD*");		break;
					case CANLIGHT:		printf(" LIGHT");		break;
					case MEDFRIENDLY:	printf(" M_FRIEND");		break;
					case LOWCAST:		printf(" L_CAST");		break;
					case MEDCAST:		printf(" M_CAST");		break;
					case HIGHCAST:	printf(" H_CAST");		break;
					case WASSUMMONED:	printf(" SUMMONER");		break;
					case HASSUMMONED:	printf(" HASSUMMONED");		break;
					case CANTRUESEE:	printf(" TRUESEE");		break;
					default:
						printf(" ?????");							break;
					}
			}
    }
}

extern struct init_weps weaps[];

pr_weapons()
{
    register int i;

    printf("%17s  Damage    Throw  Weight Worth Flags\n", "Weapon");

    for (i=1; i<maxweapons; i++) {

		printf("%17.17s ", weaps[i].w_name);
		printf("%5s ", weaps[i].w_dam);
		printf(" %8s ", weaps[i].w_hrl);
		printf(" %5d ", weaps[i].w_wght);
		printf(" %5d ", weaps[i].w_worth);
	
		wfind_flags(i);
	
		if (weaps[i].w_launch != NONE)
		    printf(" by %s", weaps[weaps[i].w_launch].w_name);

		printf("\n");
    }
	if(multi)
		putchar('\n');
}

wfind_flags(i)
register int i;
{
    register int k, flag, mask;
    if ( (flag=weaps[i].w_flags) != 0) {
			for (mask=01,k=0; k<32; k++,mask<<=1)
					switch (flag&mask) {
						case ISCURSED:	printf(" CURSED");		break;
						case ISKNOW:	printf(" KNOW");		break;
						case ISPOST:	printf(" POST");		break;
						case ISMETAL:	printf(" METAL");		break;
						case ISPROT:	printf(" PROT");		break;
						case ISBLESSED:	printf(" BLESS");		break;
						case ISZAPPED:	printf(" ZAP");			break;
						case ISVORPED:	printf(" VORPAL");		break;
						case ISSILVER:	printf(" SILVER");		break;
						case ISPOISON:	printf(" POISON");		break;
						case CANRETURN:	printf(" MISS_RET");	break;
						case ISOWNED:	printf(" ALWAYS_RET");	break;
						case ISLOST:	printf(" DISSAPEARS");	break;
						case ISMISL:	printf(" MISSLE");		break;
						case ISMANY:	printf(" MANY");		break;
						case CANBURN:	printf(" BURN");		break;
						case ISSHARP:	printf(" SHARP");		break;
						case ISTWOH:		printf(" TWOH");			break;
						case ISLITTLE:	printf(" LITTLE");		break;
					}
	}
}
