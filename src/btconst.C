/*-------------------------------------------------------------------------*\
  <btconst.C> -- Bt Builder constants

  Date      Programmer  Description
  11/20/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "valuelookup.h"

const char *spellTypes[] =
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
 "Compass",
 "Push",
 "Attack Rate Bonus",
 "Regenerate Spell Points",
 "Save Bonus",
 "Block Random Encounters",
 "Block Spells",
 "Spell Bind",
 "Levitation",
 "Regenerate Skill",
 "Cure Status",
 "Teleport",
 "Melee Damage Bonus",
 "Detect Special",
 "Location"
};
ArrayLookup spellTypeLookup(BT_SPELLTYPES_FULL, spellTypes);

const char *effectTypes[] =
{
 "Magic",
 "Song",
 "Item"
};
ArrayLookup effectTypeLookup(BT_EFFECTTYPE, effectTypes);

const char *effectiveRanges[] =
{
 "Normal",
 "Double"
};

const char *areaEffect[] =
{
 "N/A",
 "Single Target",
 "Group Target",
 "All Monster Groups",
 "Caster Only"
};
ArrayLookup areaLookup(BT_AREAEFFECT, areaEffect);

const char *durationType[] =
{
 "Dice",
 "Combat",
 "Permanent",
 "Continuous",
 "Indefinite"
};
ArrayLookup durationTypeLookup(BT_DURATIONTYPE, durationType);

const char *extraDamage[] =
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
 "Point Phaze",
 "Good possession"
};
ArrayLookup extraDamageLookup(BT_MONSTEREXTRADAMAGE, extraDamage);

const char *rangedTypes[] =
{
 "None",
 "Physical: 1 Foe",
 "Physical: Group",
 "Magic"
};
ArrayLookup rangedTypeLookup(BT_RANGEDTYPES, rangedTypes);

const char *combatActions[] =
{
 "Attack",
 "Move and Attack",
 "Dependent attack",
 "Special attack",
 "Random",
 "Nothing"
};
ArrayLookup combatActionLookup(BT_COMBATACTIONS, combatActions);

/*
 * $A or $M - monster
 * $C - class
 * $D - direction
 * $E - spell type
 * $F - local flag number
 * $G - global flag number
 * $I - item
 * $K - skill
 * $L - location
 * $N - map file
 * $O - relative location
 * $P - picture
 * $R - race
 * $S - special
 * $T - trap special attack
 * $X - spell
 * $# - number
 * $$ - text
 * $! - sound
 */
const char *conditionalCommands[] =
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
 "global flag $G is set",
 "global flag $G is clear",
 "random percent <= $#",
 "race $R is in party",
 "can take $# gold from party",
 "$E is active",
 "anyone passess $K check of $#",
 "anyone has tag $$",
 "selected PC has tag $$",
 "PC is selected",
 "selected PC passess $K check of $#",
 "everyone passess $K check of $#"
};

const char *specialCommands[] =
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
 "teleport to $N at $L dir:$D",
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
 "set encounter: $# $M",
 "begin combat",
 "chest holding $I,trap $T,damage $#,message $$",
 "set global flag $G",
 "clear global flag $G",
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
 "goto $$",
 "subtract $# from counter",
 "relative teleport by $O dir: $D in $N",
 "tele-activate to $N at $L dir: $D",
 "label $$",
 "selected PC gains tag $$",
 "selected PC removes tag $$",
 "run xml file $$",
 "answer to \"$$\" is selected PC",
 "set race to $R",
 "reroll abilities",
 "save the roster"
};

const char *directions[] =
{
 "north",
 "east",
 "south",
 "west"
};
ArrayLookup directionsLookup(BT_DIRECTIONS, directions);

const char *statAbbrev[] =
{
 "St",
 "IQ",
 "Dx",
 "Cn",
 "Lk"
};
ArrayLookup statLookup(BT_STATS, statAbbrev);

const char *genderNames[] =
{
 "male",
 "female",
 "it"
};
ArrayLookup genderLookup(BT_GENDERS, genderNames);
const char *genderPronouns[] =
{
 "him",
 "her",
 "it"
};

int statusValue[] =
{
 BTSTATUS_LEVELDRAIN,
 BTSTATUS_POISONED,
 BTSTATUS_INSANE,
 BTSTATUS_AGED,
 BTSTATUS_POSSESSED,
 BTSTATUS_STONED,
 BTSTATUS_PARALYZED,
 BTSTATUS_GOODPOSSESSED
};
const char *statusNames[] =
{
 "Level drained",
 "Poisoned",
 "Insane",
 "Aged",
 "Possessed",
 "Stoned",
 "Paralyzed",
 "Good possessed"
};
PairLookup statusLookup(7, statusValue, statusNames);

const char *restriction[] =
{
 "none",
 "combat",
 "noncombat"
};
ArrayLookup restrictionLookup(BT_RESTRICTIONS, restriction);

const char *targetOverride[] =
{
 "none",
 "singer",
 "all monsters"
};
ArrayLookup targetOverrideLookup(BT_TARGETOVERRIDES, targetOverride);

const char *mapDisplayMode[] =
{
 "request",
 "no3d",
 "always"
};
ArrayLookup mapDisplayModeLookup(BT_MAPDISPLAYMODES, mapDisplayMode);

const char *toHitBonus[] =
{
 "always",
 "mustUse"
};
ArrayLookup toHitBonusLookup(BT_TOHITBONUS, toHitBonus);
