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



static const u32 sBg3_Tiles[] = INCBIN_U32("graphics/new_pokenav/bg3_tileset.4bpp.lz");
static const u32 sBg3_Map[] = INCBIN_U32("graphics/new_pokenav/bg3_map.bin.lz");
static const u16 sBg3Pal[] = INCBIN_U16("graphics/new_pokenav/bg3_tileset.gbapal");

static const u32 sBg2_Tiles[] = INCBIN_U32("graphics/new_pokenav/bg2_tileset.4bpp.lz");
static const u32 sBg2_Map[] = INCBIN_U32("graphics/new_pokenav/bg2_map.bin.lz");
static const u16 sBg2Pal[] = INCBIN_U16("graphics/new_pokenav/bg2_tileset.gbapal");

static const u32 sBg1_Tiles[] = INCBIN_U32("graphics/new_pokenav/bg1_tileset.4bpp.lz");
static const u32 sBg1_Map[] = INCBIN_U32("graphics/new_pokenav/bg1_map.bin.lz");
//static const u16 sBg1Pal[] = INCBIN_U16("graphics/new_pokenav/bg1_tileset.gbapal");

static const u16 sBgColor[] = {RGB_WHITE};

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
		.charBaseIndex = 1,
		.mapBaseIndex = 10,
		.screenSize = 0,
		.paletteMode = 0,
		.priority = 1,
		.baseTile = 1,
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


static void InitAndShowBgsFromTemplate(void);
static void MainCallBack2(void);
static void VBlankCallBack(void);
void StartNewPokenav_CB2(void);
void Callback2_StartNewPokenav(void);
static void Task_PokeNavFadeIn(u8 taskId);



static void InitAndShowBgsFromTemplate()
{
	InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
	LZ77UnCompVram(sBg3_Tiles, (void *) VRAM + 0x4000 * 3);
	LZ77UnCompVram(sBg3_Map, (u16*) BG_SCREEN_ADDR(26));
	LZ77UnCompVram(sBg2_Tiles, (void *) VRAM + 0x4000 * 2);
	LZ77UnCompVram(sBg2_Map, (u16*) BG_SCREEN_ADDR(18));

    LZ77UnCompVram(sBg1_Tiles, (void *) VRAM + 0x4000 * 1);
	LZ77UnCompVram(sBg1_Map, (u16*) BG_SCREEN_ADDR(10));

	LoadPalette(sBg3Pal, 0x00, 0x20);
	LoadPalette(sBg2Pal, 0x10, 0x20);
	
	ResetAllBgsCoordinates();
	
	//ShowBg(0);
    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG1 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_BG3 | BLDCNT_TGT2_OBJ);
    SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(8, 4));
	ShowBg(1);
	ShowBg(2);
	ShowBg(3);
	
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
    ChangeBgX(3, -60, 2);
    //ChangeBgY(3, 30, 2);
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}


void StartNewPokenav_CB2() //Este equivale a StartTutorialMenu_CB2() del video
{
	if(!gPaletteFade.active)
	{
		gMain.state = 0;
		CleanupOverworldWindowsAndTilemaps();
		SetMainCallback2(Callback2_StartNewPokenav);

		return;
	}
	return;
}



void Callback2_StartNewPokenav(void)
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
        taskId = CreateTask(Task_PokeNavFadeIn, 0);
        //data = AllocZeroed(sizeof(struct MusicMenu));
        //SetStructPtr(taskId, data);
        gMain.state++;
        break;
    case 5:
        BeginNormalPaletteFade(-1, 0, 0x10, 0, 0);
        SetVBlankCallback(VBlankCallBack);
        SetMainCallback2(MainCallBack2);
        return;
    }
}


static void Task_PokeNavFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        return;
}

