////////////////////////////////////////////////////////////////////////////
//
// TRIGCALC		By J. Flynn 5/23/98
//
// Computes a generalized linedef type number based on the answers to 
// questions about what it does.
//
// 2000-10-21 Udo Munk: Ported to UNIX and fixed up
//	
////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>

#define GenFloorBase		0x6000
#define GenCeilingBase		0x4000
#define GenDoorBase		0x3c00
#define GenLockedBase		0x3800
#define GenLiftBase		0x3400
#define GenStairsBase		0x3000
#define GenCrusherBase		0x2F80

#define TriggerType		0x0007
#define TriggerTypeShift	0

// define names for the TriggerType field of the general linedefs
typedef enum
{
	WalkOnce,
	WalkMany,
	SwitchOnce,
	SwitchMany,
	GunOnce,
	GunMany,
	PushOnce,
	PushMany,
} triggertype_e;

// define names for the Speed field of the general linedefs
typedef enum
{
	SpeedSlow,
	SpeedNormal,
	SpeedFast,
	SpeedTurbo,
} motionspeed_e;

// define masks and shifts for the floor type fields
#define FloorCrush		0x1000
#define FloorChange		0x0c00
#define FloorTarget		0x0380
#define FloorDirection		0x0040
#define FloorModel		0x0020
#define FloorSpeed		0x0018

#define FloorCrushShift		12
#define FloorChangeShift	10
#define FloorTargetShift	 7
#define FloorDirectionShift	 6
#define FloorModelShift		 5
#define FloorSpeedShift		 3

// define names for the Target field of the general floor
typedef enum
{
	FtoHnF, FtoLnF, FtoNnF, FtoLnC, FtoC, FbyST, Fby24, Fby32
} floortarget_e;

// define names for the Changer Type field of the general floor
typedef enum
{
	FNoChg, FChgZero, FChgTxt, FChgTyp
} floorchange_e;

// define names for the Change Model field of the general floor
typedef enum
{
	FTriggerModel, FNumericModel
} floormodel_t;

// define masks and shifts for the ceiling type fields
#define CeilingCrush		0x1000
#define CeilingChange		0x0c00
#define CeilingTarget		0x0380
#define CeilingDirection	0x0040
#define CeilingModel		0x0020
#define CeilingSpeed		0x0018

#define CeilingCrushShift	12
#define CeilingChangeShift	10
#define CeilingTargetShift	 7
#define CeilingDirectionShift	 6
#define CeilingModelShift	 5
#define CeilingSpeedShift	 3

// define names for the Target field of the general ceiling
typedef enum
{
	CtoHnC, CtoLnC, CtoNnC, CtoHnF, CtoF, CbyST, Cby24, Cby32
} ceilingtarget_e;

// define names for the Changer Type field of the general ceiling
typedef enum
{
	CNoChg, CChgZero, CChgTxt, CChgTyp
} ceilingchange_e;

// define names for the Change Model field of the general ceiling
typedef enum
{
	CTriggerModel, CNumericModel
} ceilingmodel_t;

// define masks and shifts for the lift type fields
#define LiftTarget		0x0300
#define LiftDelay		0x00c0
#define LiftMonster		0x0020
#define LiftSpeed		0x0018

#define LiftTargetShift		8
#define LiftDelayShift		6
#define LiftMonsterShift	5
#define LiftSpeedShift		3

// define names for the Target field of the general ceiling
typedef enum
{
	F2LnF, F2NnF, F2LnC, LnF2HnF
} lifttarget_e;

// define masks and shifts for the stairs type fields
#define StairIgnore		0x0200
#define StairDirection		0x0100
#define StairStep		0x00c0
#define StairMonster		0x0020
#define StairSpeed		0x0018

#define StairIgnoreShift	9
#define StairDirectionShift	8
#define StairStepShift		6
#define StairMonsterShift	5
#define StairSpeedShift		3

// define masks and shifts for the crusher type fields
#define CrusherSilent		0x0040
#define CrusherMonster		0x0020
#define CrusherSpeed		0x0018

#define CrusherSilentShift	6
#define CrusherMonsterShift	5
#define CrusherSpeedShift	3

// define masks and shifts for the door type fields
#define DoorDelay		0x0300
#define DoorMonster		0x0080
#define DoorKind		0x0060
#define DoorSpeed		0x0018

#define DoorDelayShift		8
#define DoorMonsterShift	7
#define DoorKindShift		5
#define DoorSpeedShift		3

// define names for the door Kind field of the general ceiling
typedef enum
{
	OdCDoor, ODoor, CdODoor, CDoor
} doorkind_e;

// define masks and shifts for the locked door type fields
#define LockedNKeys		0x0200
#define LockedKey		0x01c0
#define LockedKind		0x0020
#define LockedSpeed		0x0018

#define LockedNKeysShift	9
#define LockedKeyShift		6
#define LockedKindShift		5
#define LockedSpeedShift	3

// define names for the locked door Kind field of the general ceiling
typedef enum
{
	AnyKey, RCard, BCard, YCard, RSkull, BSkull, YSkull, AllKeys
} keykind_e;

//--------------------------------------------------------------------

static char *Triggers[] =
{"W1", "WR", "S1", "SR", "G1", "GR", "D1", "DR"};

static char *FloorTargets[] =
{"F->HnF", "F->LnF", "F->NnF", "F->LnC", "F->C", "FbysT", "Fby24", "Fby32"};

static char *Directions[] =
{"Dn", "Up"};

static char *Speeds[] =
{"S", "N", "F", "T"};

static char *Changers[] =
{"", "c0", "cT", "cS"};

static char *Models[] =
{"t", "n"};

static char *Crushers[] =
{"", "Cr"};

static char *CeilingTargets[] =
{"C->HnC", "C->LnC", "C->NnC", "C->LnF", "C->F", "CbysT", "Cby24", "Cby32"};

static char *Doors1[] =
{"OpnD", "Opn", "ClsD", "Cls"};

static char *Doors2[] =
{"Cls", "", "Opn", ""};

static char *Delays[] =
{"1", "4", "9", "30"};

static char *LockedDelays[] =
{"4"};

static char *Locks[] =
{"Any", "RC", "BC", "YC", "RS", "BS", "YS", "All6",
 "Any", "RK", "BK", "YK", "RK", "BK", "YK", "All3"};

static char *LiftTargets[] =
{"F->LnF", "F->NnF", "F->LnC", "LnF-HnF"};

static char *LiftDelays[] =
{"1", "3", "5", "10"};

static char *Steps[] =
{"s4", "s8", "s16", "s24"};

int getone(void)
{
	struct termios	old, new;
	int		ret;

	tcgetattr(fileno(stdin), &old);
	new = old;

	new.c_lflag &= ~ICANON;
	new.c_iflag &= ~(IXON | IXANY | IXOFF);
	new.c_iflag &= ~(IGNCR | ICRNL | INLCR);
	new.c_cc[VMIN] = 1;
	new.c_cc[VTIME] = 0;

	tcsetattr(fileno(stdin), TCSADRAIN, &new);
	tcflush(fileno(stdin), TCIFLUSH);

	ret = getchar();

	tcsetattr(fileno(stdin), TCSADRAIN, &old);

	return ret;
}

char *ParseGeneralizedLinedef(int type)
{
	static char linedes[32];

	strcpy(linedes, "");
	if (type >= GenFloorBase)
	{
		// put in trigger type
		strcat(linedes, Triggers[type & TriggerType]);		
		if (!(type & FloorChange) && (type & FloorModel))
			strcat(linedes, "m");
		strcat(linedes, " ");

		// put in target description
		strcat(linedes, FloorTargets[(type & FloorTarget) >>
		       FloorTargetShift]);
		strcat(linedes, " ");

		// put in direction and speed
		strcat(linedes, Directions[(type & FloorDirection) >> 
		       FloorDirectionShift]);
		strcat(linedes, Speeds[(type & FloorSpeed) >> FloorSpeedShift]);

		// if a changer, put in change type and model
		if (type & FloorChange)
		{
			strcat(linedes, " ");
			strcat(linedes, Changers[(type & FloorChange) >> 
			       FloorChangeShift]);
			strcat(linedes, Models[(type & FloorModel) >> 
			       FloorModelShift]);
		}
		// if a crusher, put in crusher indicator
		if (type & FloorCrush)
		{
			strcat(linedes, " ");
			strcat(linedes, Crushers[(type & FloorCrush) >> 
			       FloorCrushShift]);
		}
	}
	else if (type >= GenCeilingBase)
	{
		// put in trigger type
		strcat(linedes, Triggers[type & TriggerType]);		
		if (!(type & CeilingChange) && (type & CeilingModel))
			strcat(linedes, "m");
		strcat(linedes, " ");

		// put in target description
		strcat(linedes, CeilingTargets[(type & CeilingTarget) >>
		       CeilingTargetShift]);
		strcat(linedes, " ");

		// put in direction and speed
		strcat(linedes, Directions[(type & CeilingDirection) >>
		       CeilingDirectionShift]);
		strcat(linedes, Speeds[(type & CeilingSpeed) >>
		       CeilingSpeedShift]);

		// if a changer, put in change type and model
		if (type & CeilingChange)
		{
			strcat(linedes, " ");
			strcat(linedes, Changers[(type & CeilingChange) >>
			       CeilingChangeShift]);
			strcat(linedes, Models[(type & CeilingModel) >>
			       CeilingModelShift]);
		}
		// if a crusher, put in crusher indicator
		if (type & CeilingCrush)
		{
			strcat(linedes, " ");
			strcat(linedes, Crushers[(type & CeilingCrush) >>
			       CeilingCrushShift]);
		}
	}
	else if (type >= GenDoorBase)
	{
		int k;

		// put in trigger type
		strcat(linedes, Triggers[type & TriggerType]);		
		if (type & DoorMonster)
			strcat(linedes, "m");
		strcat(linedes, " ");

		// put in door kind, and any delay
		k = (type & DoorKind) >> DoorKindShift;
		strcat(linedes, Doors1[k]);
		if (!(k & 1))
			strcat(linedes, Delays[(type & DoorDelay) >>
			       DoorDelayShift]);
		strcat(linedes, Doors2[k]);
		strcat(linedes, " ");

		// put in speed
		strcat(linedes, Speeds[(type & DoorSpeed) >> DoorSpeedShift]);
	}
	else if (type >= GenLockedBase)
	{
		int k, n;

		// put in trigger type
		strcat(linedes, Triggers[type & TriggerType]);		
		strcat(linedes, " ");

		// put in door kind, and any delay
		k = (type & LockedKind) >> LockedKindShift;
		strcat(linedes, Doors1[k]);
		if (!(k & 1))
			strcat(linedes, LockedDelays[0]);
		strcat(linedes, Doors2[k]);
		strcat(linedes, " ");

		// put in speed
		strcat(linedes, Speeds[(type & LockedSpeed) >>
		       LockedSpeedShift]);
		strcat(linedes, " ");

		// put in unlocking key type
		k = (type & LockedKey) >> LockedKeyShift;
		n = (type & LockedNKeys) >> LockedNKeysShift;
		strcat(linedes, Locks[n * 8 + k]);
	}
	else if (type >= GenLiftBase)
	{
		// put in trigger type
		strcat(linedes, Triggers[type & TriggerType]);		
		if (type & LiftMonster)
			strcat(linedes, "m");
		strcat(linedes, " Lft ");

		// put in lift target and delay
		strcat(linedes, LiftTargets[(type & LiftTarget) >>
		       LiftTargetShift]);
		strcat(linedes, " ");
		strcat(linedes, "D");
		strcat(linedes, LiftDelays[(type & LiftDelay) >>
		       LiftDelayShift]);

		// put in lift speed
		strcat(linedes, Speeds[(type & LiftSpeed) >> LiftSpeedShift]);
	}
	else if (type >= GenStairsBase)
	{
		// put in trigger type
		strcat(linedes, Triggers[type & TriggerType]);		
		if (type & StairMonster)
			strcat(linedes, "m");
		strcat(linedes, " Stair");

		// put in direction
		strcat(linedes, Directions[(type & StairDirection) >>
		       StairDirectionShift]);

		// put in stepsize
		strcat(linedes, Steps[(type & StairStep) >> StairStepShift]);

		// put in speed
		strcat(linedes, Speeds[(type & StairSpeed) >> StairSpeedShift]);

		if (type & StairIgnore)
			strcat(linedes, " Ign");
	}
	else if (type >= GenCrusherBase)
	{
		// put in trigger type
		strcat(linedes, Triggers[type & TriggerType]);		
		if (type & CrusherMonster)
			strcat(linedes, "m");
		strcat(linedes, " Crusher ");

		// put in speed
		strcat(linedes, Speeds[(type & CrusherSpeed) >>
		       CrusherSpeedShift]);

		// put in silent property
		if (type & CrusherSilent)
			strcat(linedes, " Silent");
	}
	else return NULL;

	return linedes;
}

int main(int argc, char **argv)
{
	int ch;
	int trig = 0;
	int forc = 0;
	int set = 0;

top:
	ch = 0;
	printf("Floor, Ceil, Door, Keydoor, Lift, Stair, or cRrusher?"
	       " [F/c/d/k/l/s/r]: ");
	fflush(stdout);
	ch = getone();
	printf("\n");

	switch (tolower(ch))
	{
		default:
		case 'f':
			forc = 1;
			trig = GenFloorBase - GenCeilingBase;
			// fall thru
		case 'c':
			trig += GenCeilingBase;
			ch = 0;
			printf("Crushing [N/y]: ");
			fflush(stdout);
			ch = getone();
			printf("\n");
			if (tolower(ch) == 'y') trig += (1 << FloorCrushShift);

			ch = 0;
			printf("Speed [0=Slow, 1=Normal, 2=Fast, 3=Turbo]: ");
			fflush(stdout);
			ch = getone() - '0';
			printf("\n");
			if (0 > ch) ch = 0; if (ch > 3) ch = 3;
			trig += (ch << FloorSpeedShift);

			ch = 0;
			printf("Type of change [0=none, 1=zero type"
			       " 2=text only 3=text & type]: ");
			fflush(stdout);
			ch = getone() - '0';
			printf("\n");
			if (0 > ch) ch = 0; if (ch > 3) ch = 3;
			trig += (ch << FloorChangeShift);

			if (ch)
			{
				ch = 0;
				printf("Model for change"
				       " [T(rigger)/n(umeric)]: ");
				fflush(stdout);
				ch = getone();
				printf("\n");
				if (tolower(ch) == 'n')
					trig += (1 << FloorModelShift);
			}
			else
			{
				ch = 0;
				printf("Allow monster activation [N/y]: ");
				fflush(stdout);
				ch = getone();
				printf("\n");
				if (tolower(ch) == 'y')
					trig += (1 << FloorModelShift);
			}

			ch = 0;
			if (forc)
				printf("Floor Target [0:HnF 1:LnF, 2:NnF,"
				       " 3:LnC, 4:C 5:sT 6:24 7:32]: ");
			else
				printf("Ceiling Target [0:HnC 1:LnC, 2:NnC,"
				       " 3:HnF, 4:F 5:sT 6:24 7:32]: ");
			fflush(stdout);
			ch = getone() - '0';
			printf("\n");
			if (0 > ch) ch = 0; if (ch > 7) ch = 7;
			trig += (ch << FloorTargetShift);

			ch = 0;
			printf("Direction [U/d]: ");
			fflush(stdout);
			ch = getone();
			printf("\n");
			if (tolower(ch) != 'd')
				trig += (1 << FloorDirectionShift);

			set=1;
			break;

		case 'd':
			trig = GenDoorBase;

			ch = 0;
			printf("Kind of Door [0=Open-Close, 1=Open,"
			       " 2=Close-Open, 3=Close]: ");
			fflush(stdout);
			ch = getone() - '0';
			printf("\n");
			if (0 > ch) ch = 0; if (ch > 3) ch = 3;
			trig += (ch << DoorKindShift);

			ch = 0;
			printf("Speed of Door [0=Slow, 1=Normal, 2=Fast,"
			       " 3=Turbo]: ");
			fflush(stdout);
			ch = getone() - '0';
			printf("\n");
			if (0 > ch) ch = 0; if (ch > 3) ch = 3;
			trig += (ch << DoorSpeedShift);

			ch = 0;
			printf("Allow monster activation [N/y]: ");
			fflush(stdout);
			ch = getone();
			printf("\n");
			if (tolower(ch) == 'y')
				trig += (1 << DoorMonsterShift);

			ch = 0;
			printf("Door Delay [0=1sec, 1=4sec, 2=9sec,"
			       " 3=30sec]: ");
			fflush(stdout);
			ch = getone() - '0';
			printf("\n");
			if (0 > ch) ch = 0; if (ch > 3) ch = 3;
			trig += (ch << DoorDelayShift);

			set = 1;
			break;

		case 'k':
			trig = GenLockedBase;

			ch = 0;
			printf("Kind of Door [0=Open-Close, 1=Open]: ");
			fflush(stdout);
			ch = getone() - '0';
			printf("\n");
			if (0 > ch) ch = 0; if (ch > 1) ch = 1;
			trig += (ch << LockedKindShift);

			ch = 0;
			printf("Speed of Door [0=Slow, 1=Normal, 2=Fast,"
			       " 3=Turbo]: ");
			fflush(stdout);
			ch = getone() - '0';
			printf("\n");
			if (0 > ch) ch = 0; if (ch > 3) ch = 3;
			trig += (ch << LockedSpeedShift);

			ch = 0;
			printf("Door Key [0=Any, 1=RC, 2=BC, 3=YC, 4=RS,"
			       " 5=BS, 6=YS, 7=All]: ");
			fflush(stdout);
			ch = getone() - '0';
			printf("\n");
			if (0 > ch) ch = 0; if (ch > 7) ch = 7;
			trig += (ch << LockedKeyShift);

			ch = 0;
			printf("Are skull and card keys the same,"
			       " for this door? [Y/n]: ");
			fflush(stdout);
			ch = getone();
			printf("\n");
			if (tolower(ch) != 'n')
				trig += (1 << LockedNKeysShift);

			set = 1;
			break;

		case 'l':
			trig = GenLiftBase;

			ch = 0;
			printf("Target [0:LnF 1:NnF, 2:LnC, 3:Perpetual]: ");
			fflush(stdout);
			ch = getone() - '0';
			printf("\n");
			if (0 > ch) ch = 0; if (ch > 3) ch = 3;
			trig += (ch << LiftTargetShift);

			ch = 0;
			printf("Lift Delay [0=1 sec, 1=3 sec, 2=5 sec,"
			       " 3=10 sec]: ");
			fflush(stdout);
			ch = getone() - '0';
			printf("\n");
			if (0 > ch) ch = 0; if (ch > 3) ch = 3;
			trig += (ch << LiftDelayShift);

			ch = 0;
			printf("Allow monster activation [N/y]: ");
			fflush(stdout);
			ch = getone();
			printf("\n");
			if (tolower(ch) == 'y')
				trig += (1 << LiftMonsterShift);

			ch = 0;
			printf("Speed of Lift [0=Slow, 1=Normal, 2=Fast,"
			       " 3=Turbo]: ");
			fflush(stdout);
			ch = getone() - '0';
			printf("\n");
			if (0 > ch) ch = 0; if (ch > 3) ch = 3;
			trig += (ch << LiftSpeedShift);

			set = 1;
			break;

		case 's':
			trig = GenStairsBase;

			ch = 0;
			printf("Direction [U/d]: ");
			fflush(stdout);
			ch = getone();
			printf("\n");
			if (tolower(ch) != 'd')
				trig += (1 << StairDirectionShift);

			ch = 0;
			printf("Step Size [0=4, 1=8, 2=16, 3=24]: ");
			fflush(stdout);
			ch = getone() - '0';
			printf("\n");
			if (0 > ch) ch = 0; if (ch > 3) ch = 3;
			trig += (ch << StairStepShift);

			set = 1;
			ch = 0;
			printf("Ignore texture difference? [N/y]: ");
			fflush(stdout);
			ch = getone();
			printf("\n");
			if (tolower(ch) == 'y')
				trig += (1 << StairIgnoreShift);

			ch = 0;
			printf("Allow monster activation [N/y]: ");
			fflush(stdout);
			ch = getone();
			printf("\n");
			if (tolower(ch) == 'y')
				trig += (1 << StairMonsterShift);

			ch = 0;
			printf("Speed of Stair [0=Slow, 1=Normal, 2=Fast,"
			       " 3=Turbo]: ");
			fflush(stdout);
			ch = getone() - '0';
			printf("\n");
			if (0 > ch) ch = 0; if (ch > 3) ch = 3;
			trig += (ch << StairSpeedShift);

			set = 1;
			break;

		case 'r':
			trig = GenCrusherBase;

			set = 1;
			ch = 0;
			printf("Silent Crusher? [N/y]: ");
			fflush(stdout);
			ch = getone();
			printf("\n");
			if (tolower(ch) == 'y')
				trig += (1 << CrusherSilentShift);

			ch = 0;
			printf("Allow monster activation [N/y]: ");
			fflush(stdout);
			ch = getone();
			printf("\n");
			if (tolower(ch) == 'y')
				trig += (1 << CrusherMonsterShift);

			ch = 0;
			printf("Speed of Crusher [0=Slow, 1=Normal,"
			       " 2=Fast, 3=Turbo]: ");
			fflush(stdout);
			ch = getone() - '0';
			printf("\n");
			if (0 > ch) ch = 0; if (ch > 3) ch = 3;
			trig += (ch << CrusherSpeedShift);

			set = 1;

			break;
	}

	if (set)
	{
		char *p;

		ch = 0;
		printf("Trigger Type [0=W1, 1=WR, 2=S1, 3=SR, 4=G1,"
		       " 5=GR, 6=D1, 7=DR]: ");
		fflush(stdout);
		ch = getone() - '0';
		printf("\n");
		if (0 > ch) ch = 0; if (ch > 7) ch = 7;
		trig += (ch << TriggerTypeShift);

		printf("\nLinedef special type is: %d (dec)"
		       " %xH (hex)\n", trig, trig);

		p = ParseGeneralizedLinedef(trig);
		printf("\nFunctional description: %s,"
		       " is this correct? [Y/n/Q(uit)]: ", p);
		fflush(stdout);
		ch = getone();
		printf("\n");
		if (tolower(ch) == 'n')
			goto top;
	}
	exit(0);
}
