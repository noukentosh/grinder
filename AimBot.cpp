#pragma once
struct AimBot {
    ConfigLoader* cl;
    XDisplay* display;
    Level* level;
    LocalPlayer* localPlayer;
    std::vector<Player*>* players;
    Player* target = nullptr;

    AimBot(ConfigLoader* cl, XDisplay* display, Level* level, LocalPlayer* localPlayer, std::vector<Player*>* players) {
        this->cl = cl;
        this->display = display;
        this->level = level;
        this->localPlayer = localPlayer;
        this->players = players;
    }

    void aimAssist(int counter) {
        if (!active()) { releaseTarget(); return; }
        if (target == nullptr) assignTarget();
        if (target == nullptr) return;
        if (!target->visible) return;
        if (target->distance2DToLocalPlayer < util::metersToGameUnits(cl->AIMBOT_MIN_DISTANCE)) return;
        if (target->distance2DToLocalPlayer > util::metersToGameUnits(cl->AIMBOT_MAX_DISTANCE)) return;
        moveMouse();
    }

    void moveMouse() {
        //calculate smoothing    
        float EXTRA_SMOOTH = cl->AIMBOT_SMOOTH_EXTRA_BY_DISTANCE / target->distanceToLocalPlayer;
        float TOTAL_SMOOTH = cl->AIMBOT_SMOOTH + EXTRA_SMOOTH;
        //Aimbot calcs
        const FloatVector2D aimbotDelta = target->aimbotDesiredAnglesIncrement
            .multiply(100)
            .divide(TOTAL_SMOOTH);
        const double aimYawIncrement = aimbotDelta.y * -1;
        const double aimPitchIncrement = aimbotDelta.x;
        //combine
        const double totalPitchIncrement = aimPitchIncrement;
        const double totalYawIncrement = aimYawIncrement;
        //turn into integers
        int totalPitchIncrementInt = roundHalfEven(atLeast_1_AwayFromZero(totalPitchIncrement));
        int totalYawIncrementInt = roundHalfEven(atLeast_1_AwayFromZero(totalYawIncrement));

        if (target->aimbotDesiredAnglesIncrement.magnitude() > cl->AIMBOT_FOV) return;

        //deadzone - are we close enough yet?
        if (fabs(target->aimbotDesiredAnglesIncrement.x) < cl->AIMBOT_DEADZONE) totalPitchIncrementInt = 0;
        if (fabs(target->aimbotDesiredAnglesIncrement.y) < cl->AIMBOT_DEADZONE) totalYawIncrementInt = 0;
        if (totalPitchIncrementInt == 0 && totalYawIncrementInt == 0) return;
        //move mouse
        display->moveMouseRelative(totalPitchIncrementInt, totalYawIncrementInt);
    }

    bool active() {
        bool aimbotIsOn = cl->FEATURE_AIMBOT_ON;
        bool combatReady = localPlayer->isCombatReady();
        int weaponId = localPlayer->weaponIndex;
        bool weaponDiscarded = localPlayer->weaponDiscarded;

        //only these weapons will use aimbot
        bool weaponCanBeAimbotted = (
            weaponId == WEAPON_R301 ||
            weaponId == WEAPON_FLATLINE ||
            weaponId == WEAPON_HAVOC ||
            weaponId == WEAPON_SPITFIRE ||
            weaponId == WEAPON_RAMPAGE ||
            weaponId == WEAPON_HEMLOCK ||
            weaponId == WEAPON_VOLT ||
            weaponId == WEAPON_LSTAR ||
            weaponId == WEAPON_DEVOTION ||
            weaponId == WEAPON_PROWLER ||
            weaponId == WEAPON_ALTERNATOR ||
            weaponId == WEAPON_R99 ||
            weaponId == WEAPON_CAR ||
            weaponId == WEAPON_P2020 ||
            weaponId == WEAPON_MOZAMBIQUE ||
            weaponId == WEAPON_EVA8 ||
            weaponId == WEAPON_RE45);

        bool activatedByAttackingAndIsAttacking = cl->AIMBOT_ACTIVATED_BY_ATTACK && localPlayer->inAttack;
        bool activatedByADSAndIsADSing = cl->AIMBOT_ACTIVATED_BY_ADS && localPlayer->inZoom;
        bool activatedByButtonAndButtonIsDown = cl->AIMBOT_ACTIVATED_BY_BUTTON != "" && display->keyDown(cl->AIMBOT_ACTIVATED_BY_BUTTON);
        bool active = aimbotIsOn
            && combatReady
            && !weaponDiscarded
            && weaponCanBeAimbotted
            && (activatedByAttackingAndIsAttacking
                || activatedByADSAndIsADSing
                || activatedByButtonAndButtonIsDown);
        return active;
    }

    void assignTarget() {
        for (int i = 0;i < players->size();i++) {
            Player* p = players->at(i);
            if (!p->isCombatReady())continue;
            if (!p->enemy) continue;
            if (!p->visible) continue;
            if (p->aimedAt) continue;
            if (fabs(p->aimbotDesiredAnglesIncrement.x) > cl->AIMBOT_FOV) continue;
            if (fabs(p->aimbotDesiredAnglesIncrement.y) > cl->AIMBOT_FOV) continue;
            if (target == nullptr || p->aimbotScore > target->aimbotScore) {
                target = p;
                target->aimbotLocked = true;
            }
        }
    }

    void releaseTarget() {
        if (target != nullptr && target->isValid())
            target->aimbotLocked = false;
        target = nullptr;
    }

    void resetLockFlag() {
        for (int i = 0;i < players->size();i++) {
            Player* p = players->at(i);
            if (!p->isCombatReady()) continue;
            p->aimbotLocked = false;
        }
        if (target != nullptr)
            target->aimbotLocked = true;
    }

    int roundHalfEven(float x) {
        return (x >= 0.0)
            ? static_cast<int>(std::round(x))
            : static_cast<int>(std::round(-x)) * -1;
    }

    float atLeast_1_AwayFromZero(float num) {
        if (num > 0) return std::max(num, 1.0f);
        return std::min(num, -1.0f);
    }
};
