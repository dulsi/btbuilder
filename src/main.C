#include "btconst.h"
#include "file.h"
#include "monster.h"
#include "spell.h"
#include "item.h"
#include "map.h"

int main(int argc, char *argv[])
{
 BinaryReadFile f(argv[2]);
 try
 {
  if (strcmp(argv[1], "-m") == 0)
  {
   while (true)
   {
    BTMonster mon(f);
    printf("Name: %s\n", mon.getName());
    printf("Illusion: %s   Base armor class: %d\n",
      (mon.isIllusion() ? "Yes" : "No"), mon.getAc());
    printf("Level: %d   Thaumaturgical resistance: %d\n", mon.getLevel(),
      mon.getMagicResistance());
    printf("Starting distance (*10'): %d   Moves per round: %d\n",
      mon.getStartDistance(), mon.getMove());
    printf("Hit points: %dd%d   Combat options: \n", mon.getHp().getNumber(),
      mon.getHp().getType());
    printf("Rate of attacks: %d   Damage Dice: %dd%d\n",
      mon.getRateAttacks(), mon.getMeleeDamage().getNumber(),
      mon.getMeleeDamage().getType());
    printf("Extra damage: %s\n", extraDamage[mon.getMeleeExtra()]);
    printf("Attack message: %s\n", mon.getMeleeMessage());
    printf("Extra ranged attack information -\n");
    printf("   Type: %s", rangedTypes[mon.getRangedType()]);
    switch (mon.getRangedType())
    {
     case BTRANGEDTYPE_FOE:
     case BTRANGEDTYPE_GROUP:
     case BTRANGEDTYPE_MAGIC:
      printf("   Message: %s\n", mon.getRangedMessage());
      printf("   Damage: %dd%d   Extra damage: %s   Range: %d",
        mon.getRangedDamage().getNumber(),  mon.getRangedDamage().getType(),
        extraDamage[mon.getRangedExtra()], mon.getRange());
      break;
     default:
      break;
    }
    printf("\n");
    printf("Upper limit appearing: %d  Gold: %dd%d   Picture: %d\n",
      mon.getMaxAppearing(), mon.getGold().getNumber(),
      mon.getGold().getType(), mon.getPicture());
    printf("\n");
   }
  }
  else if (strcmp(argv[1], "-s") == 0)
  {
   while (true)
   {
    BTSpell mon(f);
    printf("Name: %s\n", mon.getName());
    printf("Code: %s\n", mon.getCode());
    printf("Points: %d   Range: %d   Extra range: %s\n", mon.getSp(),
      mon.getRange() * 10, effectiveRanges[mon.getEffectiveRange()]);
    printf("Type: %s", spellTypes[mon.getType()]);
    switch (mon.getType())
    {
     case BTSPELLTYPE_ARMORBONUS:
     case BTSPELLTYPE_HITBONUS:
      printf("   Bonus: %d", mon.getExtra());
      break;
     case BTSPELLTYPE_REGENBARD:
      printf("   Amount: %d", mon.getExtra());
      break;
     default:
      break;
    }
    printf("\nTarget: %s\n", areaEffect[mon.getArea()]);
    printf("Dice: %dd%d   Duration: %s\n", mon.getDice().getNumber(),
      mon.getDice().getType(), durations[mon.getDuration()]);
    printf("Effect: %s\n", mon.getEffect());
    printf("\n");
   }
  }
  else if (strcmp(argv[1], "-i") == 0)
  {
   while (true)
   {
    BTItem mon(f);
    printf("Item: %s\n", mon.getName());
    printf("Type: %s\n", itemTypes[mon.getType()]);
    printf("Price: %d   User class: \n", mon.getPrice());
    printf("Armor bonus: %d   Hit bonus: \n", mon.getArmorPlus(),
      mon.getHitPlus());
    printf("Damage dice: %dd%d\n", mon.getDamage().getNumber(),
      mon.getDamage().getType());
    printf("Extra special damage: %s   Special damage likelihood: %d%%\n",
      extraDamage[mon.getXSpecial()], mon.getChanceXSpecial());
    printf("Times useable: %d   Spell cast: \n", mon.getTimesUsable());
    printf("Cause: %s\n", mon.getCause());
    printf("Effect: %s\n", mon.getEffect());
    printf("\n");
   }
  }
  else if (strcmp(argv[1], "-p") == 0)
  {
   BTMap mon(f);
  }
 }
 catch (FileException e)
 {
 }
 return 0;
}

