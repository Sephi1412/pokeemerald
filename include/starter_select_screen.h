#ifndef GUARD_STARTER_SELECT_SCREEN_H
#define GUARD_STARTER_SELECT_SCREEN_H

bool8 StartTutorialMenu_CB2();
void StarterSelectScreen_CB2(MainCallback returnCallback);
void C2_StarterSelectScreen(void);
void CB2_InitTutorialMenu();
static void CB2_Tutorial();
static void LoadTutorialBgs();

void InitMyTextWindows();
void AddText(const u8* text);
void CreateTextBox(u8 windowId, u8 font, const u8* colourTable, u8 left, u8 top);
void CreateTextBoxType(u8 windowId, u8 font, const u8* colourTable, u8 top, u8 left);

bool8 StartOption_CB2();

bool8 MainState_StarterBeginFadeInOut();
bool8 MainState_StarterWaitFadeOutAndExit(MainCallback returnCallback);



#endif //GUARD_STARTER_SELECT_SCREEN_H