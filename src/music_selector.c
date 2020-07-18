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


#define CATEGORY_COUNT 2
#define SONGS_COUNT 6

static const u32 sBg3_Tiles[] = INCBIN_U32("graphics/music_selector/bg3_tileset.4bpp.lz");
static const u32 sBg3_Map[] = INCBIN_U32("graphics/music_selector/bg3_map.bin.lz");
static const u16 sBg3Pal[] = INCBIN_U16("graphics/music_selector/bg3_tileset.gbapal");
static const u32 sBg2_Tiles[] = INCBIN_U32("graphics/music_selector/bg2_tileset.4bpp.lz");
static const u32 sBg2_Map[] = INCBIN_U32("graphics/music_selector/bg2_map.bin.lz");
static const u16 sBg2Pal[] = INCBIN_U16("graphics/music_selector/bg2_tileset.gbapal");


struct MusicMenu
{

    u8 mainListWindowId;
    u8 mainListTaskId;
    u8 currentMainListItemId;

    u8 secondaryListWindowId;
    u8 secondaryListTaskId;
    u8 currentSecondaryListItemId;

    u8 actualPos;
    u8 wildThemeIndex;
    u8 trainerThemeIndex;

    u8 rowIndex;
    u8 activeWindow;

    bool8 songIsActive;

};

enum
{
    LIST_ITEM_WILD,
    LIST_ITEM_TRAINER,
    LIST_ITEM_COUNT
};

enum
{
    ACTIVE_WIN_MAIN,
    ACTIVE_WIN_SECONDARY,
    ACTIVE_WIN_MODIFY
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



const u8 gSongsNames[][40] = {
	[1] = _("Hoenn Wild"),
	[2] = _("Hoenn Trainer"),
	[3] = _("Kanto Wild"),
	[4] = _("Kanto Trainer"),
    [5] = _("Kanto Gym"),
    [6] = _("Tower Battle SWSH"),
};



const u16 gSongsAvailable[SONGS_COUNT] = {
    MUS_BATTLE27,
	MUS_BATTLE20,
	MUS_RG_VS_YASEI,
	MUS_RG_VS_TORE,
    MUS_RG_VS_GYM,
    TEST_DRUMS,
};


static const struct ListMenuItem sMainListItems[] =
{
    {sText_Main_Var, LIST_ITEM_WILD},
    {sText_Main_Flags, LIST_ITEM_TRAINER},
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
    .fillValue = 0,
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
    .tilemapTop = 6,
    .width = 10,
    .height = 12,
    .paletteNum = 0xF,
    .baseBlock = 0x2
};

static const struct WindowTemplate sSecondaryListWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 12,
    .tilemapTop = 6,
    .width = 16,
    .height = 2,
    .paletteNum = 0xF,
    .baseBlock = 0xA0
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
		.bg = 2,
		.charBaseIndex = 2,
		.mapBaseIndex = 18,
		.screenSize = 0,
		.paletteMode = 0,
		.priority = 2,
		.baseTile = 2,
	},
	{
		.bg = 3,
		.charBaseIndex = 3,
		.mapBaseIndex = 26,
		.screenSize = 0,
		.paletteMode = 0,
		.priority = 3,
		.baseTile = 3,
	}
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
static void CreateSecondaryListMenu(struct MusicMenu *data);
static void PadString(const u8 *src, u8 *dst);
static void PrintSecondaryEntries(struct MusicMenu *data, u8 wildIndex, u8 trainerIndex);
static void DestroyModifyArrows(struct MusicMenu *data);
static void PrintDigitChars(struct MusicMenu *data);
static const u32 GetBitfieldToAndValue(u32 currBit, u32 bitsCount);
static const u32 GetBitfieldValue(u32 value, u32 currBit, u32 bitsCount);
static u32 CharDigitsToValue(u8 *charDigits, u8 maxDigits);
static void ValueToCharDigits(u8 *charDigits, u32 newValue, u8 maxDigits);
static void SetUpModifyArrows(struct MusicMenu *data);
static void UpdateValue(struct MusicMenu *data);
static void Task_HandleMainInputs(u8 taskId);

/*



	C O D E
	



*/



static void InitAndShowBgsFromTemplate()
{
	InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
	LZ77UnCompVram(sBg3_Tiles, (void *) VRAM + 0x4000 * 3);
	LZ77UnCompVram(sBg3_Map, (u16*) BG_SCREEN_ADDR(26));
	LZ77UnCompVram(sBg2_Tiles, (void *) VRAM + 0x4000 * 2);
	LZ77UnCompVram(sBg2_Map, (u16*) BG_SCREEN_ADDR(18));

	LoadPalette(sBg3Pal, 0x00, 0x20);
	LoadPalette(sBg2Pal, 0x10, 0x20);
	
	ResetAllBgsCoordinates();
	
	//ShowBg(0);
	//ShowBg(1);
	ShowBg(2);
	ShowBg(3);
	
}



static struct MusicMenu *GetStructPtr(u8 taskId)
{
    u8 *taskDataPtr = (u8*)(&gTasks[taskId].data[0]);

    return (struct MusicMenu*)(T1_READ_PTR(taskDataPtr));
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
    ChangeBgX(3, 30, 2);
    ChangeBgY(3, 30, 2);
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
    struct MusicMenu *data;

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
        InitAndShowBgsFromTemplate();
        LoadPalette(sBgColor, 0, 2);
        LoadPalette(GetOverworldTextboxPalettePtr(), 0xf0, 16);
        gMain.state++;
        break;
    case 4:
        taskId = CreateTask(Task_DebugMenuFadeIn, 0);
        data = AllocZeroed(sizeof(struct MusicMenu));
        SetStructPtr(taskId, data);


        data->mainListWindowId = AddWindow(&sMainListWindowTemplate);
        data->actualPos = 1;
        gMultiuseListMenuTemplate = sMainListTemplate;
        gMultiuseListMenuTemplate.windowId = data->mainListWindowId;
        data->mainListTaskId = ListMenuInit(&gMultiuseListMenuTemplate, 0, 0);

        data->currentMainListItemId = 0;
        data->activeWindow = ACTIVE_WIN_MAIN;
        data->wildThemeIndex = VarGet(VAR_WILD_MUSIC)+1;
        data->trainerThemeIndex = VarGet(VAR_TRAINER_MUSIC)+1;
        data->songIsActive = FALSE;

        CreateSecondaryListMenu(data);
        PrintSecondaryEntries(data, data->wildThemeIndex, data->trainerThemeIndex);

        
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


static void playSongInMenu(u8 category)
{
    switch (category)
    {
    case LIST_ITEM_COUNT:
    case LIST_ITEM_WILD:
        Overworld_ChangeMusicTo(gSongsAvailable[VarGet(VAR_WILD_MUSIC)]);
        break;
    
    case LIST_ITEM_TRAINER:
        Overworld_ChangeMusicTo(gSongsAvailable[VarGet(VAR_TRAINER_MUSIC)]);
        break;
    }
}

static void Task_DebugMenuProcessInput(u8 taskId)
{
    s32 listItemId = 0;
    struct MusicMenu *data = GetStructPtr(taskId);

    
    if ( (gMain.newKeys & DPAD_DOWN) && (data->currentMainListItemId != CATEGORY_COUNT-1) )
    {
        PlaySE(SE_SELECT);
        data->currentMainListItemId+= 1;
    }
 
    if ( (gMain.newKeys & DPAD_UP) && (data->currentMainListItemId != LIST_ITEM_WILD) )
    {
        PlaySE(SE_SELECT);
        data->currentMainListItemId-= 1;
    }
    
    if (gMain.newKeys & DPAD_RIGHT)
	{
		PlaySE(SE_SELECT);
        switch (data->currentMainListItemId)
            {
            case LIST_ITEM_COUNT:
            case LIST_ITEM_WILD:
                if(data->wildThemeIndex == SONGS_COUNT){ 
                    data->wildThemeIndex = 1;
                    break;
                }    
                else{
                    data->wildThemeIndex += 1;
                    break;
                }
            case LIST_ITEM_TRAINER:
                if(data->trainerThemeIndex == SONGS_COUNT){ 
                    data->trainerThemeIndex = 1;
                    break;
                }    
                else{
                    data->trainerThemeIndex += 1;
                    break;
                }
            }
    }
    if (gMain.newKeys & DPAD_LEFT)
	{
        
		PlaySE(SE_SELECT);
        switch (data->currentMainListItemId)
            {
            case LIST_ITEM_COUNT:
            case LIST_ITEM_WILD:
                if(data->wildThemeIndex == 1){
                    data->wildThemeIndex = SONGS_COUNT;
                    break;
                }    
                else{
                    data->wildThemeIndex -= 1;
                    break;
                }
            case LIST_ITEM_TRAINER:
                if(data->trainerThemeIndex == 1){
                    data->trainerThemeIndex = SONGS_COUNT;
                    break;
                }
                else
                {
                    data->trainerThemeIndex -= 1;
                    break;
                }
            }
    }

    VarSet(VAR_WILD_MUSIC, data->wildThemeIndex-1 );
    VarSet(VAR_TRAINER_MUSIC, data->trainerThemeIndex-1 );


    PrintSecondaryEntries(data, data->wildThemeIndex, data->trainerThemeIndex);
    
    if (gMain.newKeys & SELECT_BUTTON && !(data->songIsActive) )
	{	
        //Overworld_SetSavedMusic(gSongsAvailable[VarGet(VAR_WILD_MUSIC)]);
        //Overworld_ChangeMusicTo(gSongsAvailable[VarGet(VAR_WILD_MUSIC)]);
        playSongInMenu(data->currentMainListItemId);
        data->songIsActive = TRUE;
        return;

	}

    if (gMain.newKeys & SELECT_BUTTON && data->songIsActive )
	{	
        Overworld_ClearSavedMusic();
        playSongInMenu(data->currentMainListItemId);
        data->songIsActive = FALSE;
        return;

	}
    
    if (gMain.newKeys & B_BUTTON)
    {
        BeginNormalPaletteFade(-1, 0, 0, 0x10, 0);
        gTasks[taskId].func = Task_DebugMenuFadeOut;
    }



    if (data->activeWindow == ACTIVE_WIN_MAIN)
    {
        listItemId = ListMenu_ProcessInput(data->mainListTaskId);
        if (listItemId != LIST_CANCEL && listItemId != LIST_NOTHING_CHOSEN && listItemId < LIST_ITEM_COUNT)
        {
            data->currentMainListItemId = listItemId;

            CreateSecondaryListMenu(data);
            PrintSecondaryEntries(data, data->wildThemeIndex, data->trainerThemeIndex);

        }
    }
}




static void Task_HandleMainInputs(u8 taskId)
{
    struct MusicMenu *data = GetStructPtr(taskId);
    
}


static void Task_DebugMenuFadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        struct MusicMenu *data = GetStructPtr(taskId);
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


static void CreateSecondaryListMenu(struct MusicMenu *data)
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
    if (listTemplate.maxShowed > 2) // OJO
        listTemplate.maxShowed = 2;
    listTemplate.windowId = data->secondaryListWindowId;

    data->secondaryListTaskId = ListMenuInit(&listTemplate, 0, 0);
    CopyWindowToVram(data->secondaryListWindowId, 3);
}


static void PrintSecondaryEntries(struct MusicMenu *data, u8 wildIndex, u8 trainerIndex)
{
    u8 text[20];
    s32 i;
    struct TextPrinterTemplate printer;
    u8 yMultiplier;


    yMultiplier = (GetFontAttribute(sSecondaryListTemplate.fontId, 1) + sSecondaryListTemplate.itemVerticalPadding);

    printer.windowId = data->secondaryListWindowId;
    printer.fontId = 1;
    printer.unk = 0;
    printer.letterSpacing = 0;
    printer.lineSpacing = 1;
    printer.fgColor = 2;
    printer.bgColor = 0;
    printer.shadowColor = 3;
    printer.x = sSecondaryListTemplate.item_X;
    printer.currentX = sSecondaryListTemplate.item_X;
    printer.currentChar = text;

    FillWindowPixelBuffer(printer.windowId, 0);
    PadString(gSongsNames[wildIndex], text);
    printer.currentY = printer.y = (0 * yMultiplier) + sSecondaryListTemplate.upText_Y;
    AddTextPrinter(&printer, 0, NULL);
    PadString(gSongsNames[trainerIndex], text);
    printer.currentY = printer.y = (1 * yMultiplier) + sSecondaryListTemplate.upText_Y;
    AddTextPrinter(&printer, 0, NULL);

    
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
