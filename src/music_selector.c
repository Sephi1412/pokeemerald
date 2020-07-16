#include "global.h"
#include "battle.h"
#include "debug.h"
#include "battle_message.h"
#include "main.h"
#include "event_data.h"
#include "menu.h"
#include "menu_helpers.h"
#include "scanline_effect.h"
#include "palette.h"
#include "sprite.h"
#include "item.h"
#include "task.h"
#include "bg.h"
#include "gpu_regs.h"
#include "window.h"
#include "text.h"
#include "text_window.h"
#include "international_string_util.h"
#include "overworld.h"
#include "constants/songs.h"
#include "sound.h"
#include "strings.h"
#include "list_menu.h"
#include "malloc.h"
#include "string_util.h"
#include "util.h"
#include "reset_rtc_screen.h"
#include "reshow_battle_screen.h"
#include "constants/abilities.h"
#include "constants/moves.h"
#include "constants/items.h"
#include "constants/rgb.h"
#include "constants/vars.h"

#define MAX_MODIFY_DIGITS 4
#define MAX_VAR_COUNT 2



struct BattleDebugModifyArrows
{
    u8 arrowSpriteId[2];
    u16 minValue;
    u16 maxValue;
    int currValue;
    u8 currentDigit;
    u8 maxDigits;
    u8 charDigits[MAX_MODIFY_DIGITS];
    void *modifiedValPtr;
    u8 typeOfVal;
};


struct  BattleDebugMenu
{

    u8 mainListWindowId;
    u8 mainListTaskId;
    u8 currentMainListItemId;

    u8 secondaryListWindowId;
    u8 secondaryListTaskId;
    u8 currentSecondaryListItemId;

    u8 actualPos;
    u8 rowIndex;

    u8 activeWindow;

    struct BattleDebugModifyArrows modifyArrows;
    const struct BitfieldInfo *bitfield;
};



struct __attribute__((__packed__)) BitfieldInfo  /*
	Por lo visto está asociado al cambio de estados. Bien podríamos removerlo
*/
{
    u8 bitsCount;
    u8 currBit;
};



enum
{
    LIST_ITEM_WILD, // El comportamiento de PP Es similar a lo que queremos
    LIST_ITEM_TRAINER,
    LIST_ITEM_COUNT
};

enum
{
    ACTIVE_WIN_MAIN,
    ACTIVE_WIN_SECONDARY,
    ACTIVE_WIN_MODIFY
};

enum
{
    VAL_U16,
    FLAGs,   
    VARs,
};

static const u8 sText_EmptyString[] = _("");
static const u8 sText_Main_Var[] = _("Wild Theme");
static const u8 sText_Var404E[] = _("Starter index");
static const u8 sText_Var406A[] = _("Gym Badges");
static const u8 sText_Main_Flags[] = _("Trainer Theme");
static const u8 sText_Flag_0x20[] = _("0x20");
static const u8 sText_Flag_0x21[] = _("0x21");
static u16 value_Var;
static u16 value_Flag;



const u8 gSongsNames[][30] = {
	[1] = _("Hoenn Wild"),
	[2] = _("Kanto Wild"),
	[3] = _("Johto Wild"),
};




struct debug_VAR
{
    u16 var;
};


struct debug_FLAG{
    u16 flag;
};




static const struct debug_VAR gDebugVar[] = 
{
    {VAR_UNUSED_0x404E},
    {VAR_ROUTE109_STATE}

};

static const struct debug_FLAG gDebugFlag[] =
{
    {FLAG_UNUSED_0x020},
    {FLAG_UNUSED_0x021}
};

static const struct ListMenuItem sMainListItems[] =
{
    {sText_Main_Var, LIST_ITEM_WILD},
    {sText_Main_Flags, LIST_ITEM_TRAINER},
};

static const struct ListMenuItem sVarList[] =
{
    {sText_Var404E, 0},
    {sText_Var406A, 1},
};


static const struct ListMenuItem sFlagList[] =
{
    {sText_Var404E, 0},
    {sText_Var406A, 1},
};

/* 
	La estructura que viene ahora no se porque existe
	Tal vez actua como un placeholder asi que simplemente
	la conservare
*/
static const struct ListMenuItem sSecondaryListItems[] =
{
    {sText_EmptyString, 0},
    {sText_EmptyString, 1},
    {sText_EmptyString, 2},
    {sText_EmptyString, 3},
    {sText_EmptyString, 4},
    {sText_EmptyString, 5},
    {sText_EmptyString, 6},
    {sText_EmptyString, 7},
    {sText_EmptyString, 8},
};


static const struct ListMenuTemplate sMainListTemplate =
{
    .items = sMainListItems,
    .moveCursorFunc = NULL,
    .itemPrintFunc = NULL,
    .totalItems = ARRAY_COUNT(sMainListItems),
    .maxShowed = 2,
    .windowId = 0,
    .header_X = 0,
    /* De aqui en adelante no se que significa cada parametro*/
    .item_X = 8, 
    .cursor_X = 0,
    .upText_Y = 1,
    .cursorPal = 2,
    .fillValue = 1,
    .cursorShadowPal = 3,
    .lettersSpacing = 1,
    .itemVerticalPadding = 0,
    .scrollMultiple = LIST_NO_MULTIPLE_SCROLL,
    .fontId = 1,
    .cursorKind = 0
};

static const struct ListMenuTemplate sSecondaryListTemplate =
{
    .items = sSecondaryListItems,
    .moveCursorFunc = NULL,
    .itemPrintFunc = NULL,
    .totalItems = 0,
    .maxShowed = 0,
    .windowId = 0,
    .header_X = 0,
    .item_X = 8,
    .cursor_X = 0,
    .upText_Y = 0,
    .cursorPal = 0,
    .fillValue = 0,
    .cursorShadowPal = 0,
    .lettersSpacing = 0,
    .itemVerticalPadding = 0,
    .scrollMultiple = LIST_NO_MULTIPLE_SCROLL,
    .fontId = 1,
    .cursorKind = 0
};

static const struct WindowTemplate sMainListWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 3,
    .width = 10,
    .height = 12,
    .paletteNum = 0xF,
    .baseBlock = 0x2
};

static const struct WindowTemplate sSecondaryListWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 12,
    .tilemapTop = 3,
    .width = 10,
    .height = 2,
    .paletteNum = 0xF,
    .baseBlock = 0xA0
};

static const struct WindowTemplate sModifyWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 25,
    .tilemapTop = 2,
    .width = 4,
    .height = 2,
    .paletteNum = 0xF,
    .baseBlock = 0x200
};

static const struct BgTemplate sBgTemplates[] =
{
   {
       .bg = 0,
       .charBaseIndex = 0,
       .mapBaseIndex = 31,
       .screenSize = 0,
       .paletteMode = 0,
       .priority = 1,
       .baseTile = 0
   },
   {
       .bg = 1,
       .charBaseIndex = 2,
       .mapBaseIndex = 29,
       .screenSize = 0,
       .paletteMode = 0,
       .priority = 0,
       .baseTile = 0
   }
};

static const u8 sBitsToMaxDigit[] =
{
    [0] = 0,
    [1] = 1, // max 1
    [2] = 1, // max 3
    [3] = 1, // max 7
    [4] = 2, // max 15
    [5] = 2, // max 31
    [6] = 2, // max 63
    [7] = 3, // max 127
    [8] = 3, // max 255
};

static const bool8 sHasChangeableEntries[LIST_ITEM_COUNT] =
{
    [LIST_ITEM_TRAINER] = TRUE,
    [LIST_ITEM_WILD] = TRUE,
};

static const u16 sBgColor[] = {RGB_WHITE};


void StartMusicMenu_CB2();
void CallBack2_BattleMusicMenu(void);
static void Task_DebugMenuFadeIn(u8 taskId);
static void Task_DebugMenuProcessInput(u8 taskId);
static void Task_DebugMenuFadeOut(u8 taskId);
static void CreateSecondaryListMenu(struct BattleDebugMenu *data);
static void PadString(const u8 *src, u8 *dst);
static void PrintSecondaryEntries(struct BattleDebugMenu *data, u8 wildIndex, u8 trainerIndex);
static void DestroyModifyArrows(struct BattleDebugMenu *data);
static void PrintDigitChars(struct BattleDebugMenu *data);
static const u32 GetBitfieldToAndValue(u32 currBit, u32 bitsCount);
static const u32 GetBitfieldValue(u32 value, u32 currBit, u32 bitsCount);
static u32 CharDigitsToValue(u8 *charDigits, u8 maxDigits);
static void ValueToCharDigits(u8 *charDigits, u32 newValue, u8 maxDigits);
static void SetUpModifyArrows(struct BattleDebugMenu *data);
static bool32 TryMoveDigit(struct BattleDebugModifyArrows *modArrows, bool32 moveUp);
static void UpdateValue(struct BattleDebugMenu *data);
static void Task_HandleMainInputs(u8 taskId);

/*



	C O D E
	



*/




static struct BattleDebugMenu *GetStructPtr(u8 taskId)
{
    u8 *taskDataPtr = (u8*)(&gTasks[taskId].data[0]);

    return (struct BattleDebugMenu*)(T1_READ_PTR(taskDataPtr));
}

static void SetStructPtr(u8 taskId, void *ptr)
{
    u32 structPtr = (u32)(ptr);
    u8 *taskDataPtr = (u8*)(&gTasks[taskId].data[0]);

    taskDataPtr[0] = structPtr >> 0;
    taskDataPtr[1] = structPtr >> 8;
    taskDataPtr[2] = structPtr >> 16;
    taskDataPtr[3] = structPtr >> 24;
}


static void MainCallBack2(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCallBack(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}


void StartMusicMenu_CB2() //Este equivale a StartTutorialMenu_CB2() del video
{
	if(!gPaletteFade.active)
	{
		gMain.state = 0;
		CleanupOverworldWindowsAndTilemaps();
		SetMainCallback2(CallBack2_BattleMusicMenu);

		return;
	}

	return;
}


void CallBack2_BattleMusicMenu(void)
{
    u8 taskId;
    struct BattleDebugMenu *data;

    switch (gMain.state)
    {
    default:
    case 0:
        SetVBlankCallback(NULL);
        gMain.state++;
        break;
    case 1:
        ResetVramOamAndBgCntRegs();
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        ResetAllBgsCoordinates();
        FreeAllWindowBuffers();
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        gMain.state++;
        break;
    case 2:
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        gMain.state++;
        break;
    case 3:
        LoadPalette(sBgColor, 0, 2);
        LoadPalette(GetOverworldTextboxPalettePtr(), 0xf0, 16);
        gMain.state++;
        break;
    case 4:
        taskId = CreateTask(Task_DebugMenuFadeIn, 0);
        data = AllocZeroed(sizeof(struct BattleDebugMenu));
        SetStructPtr(taskId, data);


        data->mainListWindowId = AddWindow(&sMainListWindowTemplate);
        data->actualPos = 1;
        gMultiuseListMenuTemplate = sMainListTemplate;
        gMultiuseListMenuTemplate.windowId = data->mainListWindowId;
        data->mainListTaskId = ListMenuInit(&gMultiuseListMenuTemplate, 0, 0);

        data->currentMainListItemId = 0;
        data->activeWindow = ACTIVE_WIN_MAIN;

        CreateSecondaryListMenu(data);
        PrintSecondaryEntries(data, data->actualPos, 2);

        
        data->secondaryListTaskId = 0xFF;
        CopyWindowToVram(data->mainListWindowId, 3);



        gMain.state++;
        break;
    case 5:
        BeginNormalPaletteFade(-1, 0, 0x10, 0, 0);
        SetVBlankCallback(VBlankCallBack);
        SetMainCallback2(MainCallBack2);
        return;
    }
}

static void Task_DebugMenuFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_DebugMenuProcessInput;
}

static void Task_DebugMenuProcessInput(u8 taskId)
{
    s32 listItemId = 0;
    struct BattleDebugMenu *data = GetStructPtr(taskId);

    // Exit the menu.
    if (gMain.newKeys & SELECT_BUTTON)
    {
        BeginNormalPaletteFade(-1, 0, 0, 0x10, 0);
        gTasks[taskId].func = Task_DebugMenuFadeOut;
        return;
    }

    // A main list item is active, handle input.
    if (data->activeWindow == ACTIVE_WIN_MAIN)
    {
        listItemId = ListMenu_ProcessInput(data->mainListTaskId);
        if (listItemId != LIST_CANCEL && listItemId != LIST_NOTHING_CHOSEN && listItemId < LIST_ITEM_COUNT)
        {
            data->currentMainListItemId = listItemId;

            // Create the secondary menu list.
            CreateSecondaryListMenu(data);
            PrintSecondaryEntries(data, 1, 2);
            //data->activeWindow = ACTIVE_WIN_SECONDARY;
            gTasks[taskId].func = Task_HandleMainInputs;
        }
    }
}


static void Task_HandleMainInputs(u8 taskId)
{
    struct BattleDebugMenu *data = GetStructPtr(taskId);
    if (gMain.newKeys & A_BUTTON)
	{	
        //ClearWindowTilemap(SCREEN_TITLE);
		//RemoveWindow(SCREEN_TITLE);
		//CleanupOverworldWindowsAndTilemaps();
		PlaySE(SE_SELECT);

	}
    if (gMain.newKeys & DPAD_RIGHT)
	{
		PlaySE(SE_SELECT);
        data->actualPos += 1;

        ClearWindowTilemap(data->secondaryListWindowId);
		RemoveWindow(data->secondaryListWindowId);
        CreateSecondaryListMenu(data);
        PrintSecondaryEntries(data, data->actualPos, 2);
    }
    if (gMain.newKeys & DPAD_LEFT)
	{
		PlaySE(SE_SELECT);
        data->actualPos -= 1;

        ClearWindowTilemap(data->secondaryListWindowId);
		RemoveWindow(data->secondaryListWindowId);
        CreateSecondaryListMenu(data);
        PrintSecondaryEntries(data, data->actualPos, 2);
    }
}


static void Task_DebugMenuFadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        struct BattleDebugMenu *data = GetStructPtr(taskId);
        DestroyListMenuTask(data->mainListTaskId, 0, 0);
        if (data->secondaryListTaskId != 0xFF)
            DestroyListMenuTask(data->secondaryListTaskId, 0, 0);

        FreeAllWindowBuffers();
        //UpdateMonData(data);
        Free(data);
        DestroyTask(taskId);
        SetMainCallback2(CB2_ReturnToField); //OJO
    }
}


static void CreateSecondaryListMenu(struct BattleDebugMenu *data)
{
    struct WindowTemplate winTemplate;
    struct ListMenuTemplate listTemplate;
    u8 itemsCount = 1;

    winTemplate = sSecondaryListWindowTemplate;
    listTemplate = sSecondaryListTemplate;

    switch (data->currentMainListItemId)
    {
    case LIST_ITEM_TRAINER:
        itemsCount = 2;
        break;
    case LIST_ITEM_WILD:
        itemsCount = 2;
        break;
    }

    
    winTemplate.height *= itemsCount;
    data->secondaryListWindowId = AddWindow(&winTemplate);

    listTemplate.totalItems = itemsCount;
    listTemplate.maxShowed = itemsCount;
    if (listTemplate.maxShowed > 2 && !sHasChangeableEntries[data->currentMainListItemId]) // OJO
        listTemplate.maxShowed = 2;
    listTemplate.windowId = data->secondaryListWindowId;

    data->secondaryListTaskId = ListMenuInit(&listTemplate, 0, 0);
    CopyWindowToVram(data->secondaryListWindowId, 3);
}


static void PrintSecondaryEntries(struct BattleDebugMenu *data, u8 wildIndex, u8 trainerIndex)
{
    u8 text[20];
    s32 i;
    struct TextPrinterTemplate printer;
    u8 yMultiplier;

    // Do not print entries if they are not changing.
    if (!sHasChangeableEntries[data->currentMainListItemId])
        return;

    yMultiplier = (GetFontAttribute(sSecondaryListTemplate.fontId, 1) + sSecondaryListTemplate.itemVerticalPadding);

    printer.windowId = data->secondaryListWindowId;
    printer.fontId = 1;
    printer.unk = 0;
    printer.letterSpacing = 0;
    printer.lineSpacing = 1;
    printer.fgColor = 2;
    printer.bgColor = 1;
    printer.shadowColor = 3;
    printer.x = sSecondaryListTemplate.item_X;
    printer.currentX = sSecondaryListTemplate.item_X;
    printer.currentChar = text;

    switch (data->currentMainListItemId)
    {
    case LIST_ITEM_COUNT:
    case LIST_ITEM_WILD:
        PadString(gSongsNames[wildIndex], text);
        printer.currentY = printer.y = (0 * yMultiplier) + sSecondaryListTemplate.upText_Y;
        AddTextPrinter(&printer, 0, NULL);
        PadString(gSongsNames[trainerIndex], text);
        printer.currentY = printer.y = (1 * yMultiplier) + sSecondaryListTemplate.upText_Y;
        AddTextPrinter(&printer, 0, NULL);
        break;
    
    case LIST_ITEM_TRAINER:
        PadString(sText_Flag_0x20, text);
        printer.currentY = printer.y = (0 * yMultiplier) + sSecondaryListTemplate.upText_Y;
        AddTextPrinter(&printer, 0, NULL);
        PadString(sText_Flag_0x21, text);
        printer.currentY = printer.y = (1 * yMultiplier) + sSecondaryListTemplate.upText_Y;
        AddTextPrinter(&printer, 0, NULL);
        break;
    }
}

static void PadString(const u8 *src, u8 *dst)
{
    u32 i;

    for (i = 0; i < 17 && src[i] != EOS; i++)
        dst[i] = src[i];

    for (; i < 17; i++)
        dst[i] = CHAR_SPACE;

    dst[i] = EOS;
}


static void UpdateValue(struct BattleDebugMenu *data)
{
    bool8 value_Flag;
    switch (data->modifyArrows.typeOfVal)
    {
    case VAL_U16:
        *(u16*)(data->modifyArrows.modifiedValPtr) = data->modifyArrows.currValue;
        break;
    
    case FLAGs:
        *(u16*)(data->modifyArrows.modifiedValPtr) = data->modifyArrows.currValue;
        if(value_Flag == 1){
            FlagSet(gDebugFlag[data->currentSecondaryListItemId].flag);
        }
        else{
            FlagClear(gDebugFlag[data->currentSecondaryListItemId].flag);
        }


    case VARs:
        *(u16*)(data->modifyArrows.modifiedValPtr) = data->modifyArrows.currValue;
        VarSet(gDebugVar[data->currentSecondaryListItemId].var, (*(u16*)data->modifyArrows.modifiedValPtr));
        break;
    }
}
