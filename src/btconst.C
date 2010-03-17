/*-------------------------------------------------------------------------*\
  <btconst.C> -- Bt Builder constants

  Date      Programmer  Description
  11/20/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "valuelookup.h"

char *spellTypes[] =
{
 "Heal",
 "Resurrect",
 "Cure Poison",
 "Cure Insanity",
 "Youth",
 "Dispossess",
 "Stone to Flesh",
 "Cure Paralysis",
 "Restore Lost Levels",
 "Damage",
 "Kill",
 "Poison",
 "Cause Insanity",
 "Age",
 "Possess",
 "Flesh to Stone",
 "Paralyze",
 "Drain Level",
 "Summon Monster",
 "Summon Illusion",
 "Dispel Illusion",
 "Scry Sight",
 "Light",
 "Armor Class Bonus",
 "Hit Bonus",
 "Trap Destruction",
 "Secret Door Detection",
 "Phase Door",
 "Dispel Magic",
 "Regenerate Bard Songs",
 "Damage By Level",
 "Compass"
};
ArrayLookup spellTypeLookup(BT_SPELLTYPES, spellTypes);

char *effectiveRanges[] =
{
 "Normal",
 "Double"
};

char *areaEffect[] =
{
 "N/A",
 "Single Target",
 "Group Target",
 "All Monster Groups"
};
ArrayLookup areaLookup(BT_AREAEFFECT, areaEffect);

char *durations[] =
{
 "One Turn",
 "Short",
 "Medium",
 "Long",
 "Combat",
 "Permanent",
 "Continuous",
 "Indefinite"
};
ArrayLookup durationLookup(BT_DURATION, durations);

char *extraDamage[] =
{
 "Nothing special",
 "Poison",
 "Level drain",
 "Insanity",
 "Aging",
 "Possession",
 "Stoned",
 "Paralysis",
 "Critical hit",
 "Item-zot",
 "Point Phaze"
};
ArrayLookup extraDamageLookup(BT_MONSTEREXTRADAMAGE, extraDamage);

char *rangedTypes[] =
{
 "None",
 "Physical: 1 Foe",
 "Physical: Group",
 "Magic"
};

char *combatActions[] =
{
 "Attack",
 "Move and Attack",
 "Dependent attack",
 "Special attack",
 "Random",
 "Nothing"
};

char *itemTypes[] =
{
 "miscellaneous item",
 "hand weapon",
 "shield",
 "armor",
 "helm",
 "gloves",
 "instrument",
 "figurine",
 "ring",
 "wand",
 "bow",
 "arrow",
 "thrown weapon"
};

char *conditionalCommands[] =
{
 "anyone is carrying $I",
 "everyone is carrying $I",
 "last input = \"$$\"",
 "answer to \"$$\" is yes",
 "local flag $F is set",
 "local flag $F is clear",
 "group is facing $D",
 "monster named $A is in party",
 "class $C is in party",
 "it is daytime",
 "counter > $#",
 "counter = $#",
 "party won last combat",
 "global flag $F is set",
 "global flag $F is clear",
 "random percent <= $G",
 "race $R is in party",
 "can take $# gold from party"
};

char *specialCommands[] =
{
 "nothing",
 "stop",
 "spin party",
 "darkness zone",
 "silence zone",
 "anti-magic zone",
 "get input",
 "set local flag $F",
 "clear local flag $F",
 "take $I",
 "give $I",
 "sell $I for $# gold",
 "cast $X for $# gold",
 "print \"$$\"",
 "regenerate $# spell points",
 "take $# spell points",
 "heal $# hit points",
 "damage $# hit points and $T",
 "regenerate $# bard songs",
 "take $# bard songs",
 "move back one space",
 "move one square forward",
 "teleport to $L dir:$D in $N",
 "exit to guild",
 "exit to review",
 "exit to garths",
 "trap flag",
 "draw picture $P",
 "clear special at $L",
 "make $S at $L",
 "print \"$$\" under picture",
 "have $M join party",
 "have monsters leave party",
 "turn monsters hostile",
 "play sound $!",
 "set counter to $#",
 "add $# to counter",
 "set encounter: $M",
 "set encounter: $G $M",
 "begin combat",
 "chest holding $I,trap $T,damage $#,message $$",
 "set global flag $F",
 "clear global flag $F",
 "give party $# gold",
 "give party $# experience",
 "press any key",
 "have monster named $A leave party",
 "turn monster named $A hostile",
 "turn party to face $D",
 "clear this special",
 "change this special to $S",
 "force a random encounter",
 "clear text window",
 "take $# gold from party",
 "display $$ as full screen picture",
 "do special $S in $# minutes",
 "cancel timed special",
 "goto line number $J",
 "subtract $# from counter",
 "relative teleport by $L dir: $D in $N",
 "tele-activate to $L dir: $D in $N"
};

char *directions[] =
{
 "north",
 "east",
 "south",
 "west"
};

char *mapTypes[] =
{
 "Dungeon 1",
 "Dungeon 2",
 "City",
 "Wilderness"
};

char *statAbbrev[] =
{
 "St",
 "IQ",
 "Dx",
 "Cn",
 "Lk"
};
ArrayLookup statLookup(BT_STATS, statAbbrev);

char *restriction[] =
{
 "none",
 "combat",
 "noncombat"
};
ArrayLookup restrictionLookup(BT_RESTRICTIONS, restriction);

