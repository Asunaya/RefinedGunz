#pragma once

class Rules
{
public:
	Rules();

	void OnGameCreate();

	int MaxHP() const { return HP; }
	int MaxAP() const { return AP; }
	bool CanFlip() const { return !NoFlip; }
	bool CanUseElements() const { return !NoElements; }
	bool CanTipslash() const { return !NoTipslash; }
	bool IsVanillaMode() const { return VanillaMode; }
	bool MassiveEffectEnabled() const { return !NoMassiveEffect; }
	bool GetShowHitboxes() const { return ShowHitboxes; }
	bool IsSwordsOnly() const { return NoGuns; }
	bool IsAntilead() const { return Antilead; }

private:
	int HP;
	int AP;
	bool NoFlip;
	bool NoElements;
	bool NoTipslash;
	bool NoGuns;
	bool NoMassiveThrow;
	bool NoMassiveEffect;
	bool ShowHitboxes;
	bool VanillaMode;
	bool Antilead = true;

	bool GetRoomTag(const char* szString);
};

extern Rules g_Rules;

static inline Rules& GetRules()
{
	return g_Rules;
}