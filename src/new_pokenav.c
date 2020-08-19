#include "global.h"
#include "battle.h"
#include "battle_anim.h"
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
#include "trig.h"


#define MAX_ICONS_ON_SCREEN 5

// Move Cursor to Left
#define IncreaseIndex(n) ( (n+1) %3 )
#define DecreaseIndex(n) ( (n+2) %3 )


struct NewPokenav
{
    //struct PokedexListItem pokedexList[NATIONAL_DEX_COUNT + 1];
    u16 iconSpriteIds[MAX_ICONS_ON_SCREEN];
    bool8 cursorIsMoving;
    u8 rowIndex;
    u8 columnIndex;

    u8 cursorId;
    u8 taskId;
    bool8 columnIsMoving;
    u8 spriteId0, spriteId1, spriteId2, spriteId3,
    spriteId4, spriteId5, spriteId6, spriteId7, spriteId8;
};

struct NewPokenavIcon
{
    u8 rowIndex;
    u8 columnIndex;
    u8 appName[16];

};

const struct NewPokenavIcon NewPokenavIcons[3][3] =
{
    [0][0] =
    {
        .rowIndex = 0,
        .columnIndex = 0,
        .appName = _("Music Selector"),
    },
    [0][1] =
    {
        .rowIndex = 0,
        .columnIndex = 1,
        .appName = _("Peli-Taxi"),
    },
    [0][2] =
    {
        .rowIndex = 0,
        .columnIndex = 2,
        .appName = _("Remote Mart"),
    },
    [1][0] =
    {
        .rowIndex = 1,
        .columnIndex = 0,
        .appName = _("Music Selector"),
    },
    [1][1] =
    {
        .rowIndex = 1,
        .columnIndex = 1,
        .appName = _("Peli-Taxi"),
    },
    [1][2] =
    {
        .rowIndex = 1,
        .columnIndex = 2,
        .appName = _("Remote Mart"),
    },
    [2][0] =
    {
        .rowIndex = 2,
        .columnIndex = 0,
        .appName = _("Music Selector"),
    },
    [2][1] =
    {
        .rowIndex = 2,
        .columnIndex = 1,
        .appName = _("Peli-Taxi"),
    },
    [2][2] =
    {
        .rowIndex = 2,
        .columnIndex = 2,
        .appName = _("Remote Mart"),
    },
};


/* BACKGROUNDS */


static const u32 sBg3_Tiles[] = INCBIN_U32("graphics/new_pokenav/bg3_tileset.4bpp.lz");
static const u32 sBg3_Map[] = INCBIN_U32("graphics/new_pokenav/bg3_map.bin.lz");
static const u16 sBg3Pal[] = INCBIN_U16("graphics/new_pokenav/bg3_tileset.gbapal");

static const u32 sBg2_Tiles[] = INCBIN_U32("graphics/new_pokenav/bg2_tileset.4bpp.lz");
static const u32 sBg2_Map[] = INCBIN_U32("graphics/new_pokenav/bg2_map.bin.lz");
static const u16 sBg2Pal[] = INCBIN_U16("graphics/new_pokenav/bg2_tileset.gbapal");

static const u32 sBg1_Tiles[] = INCBIN_U32("graphics/new_pokenav/bg1_tileset.4bpp.lz");
static const u32 sBg1_Map[] = INCBIN_U32("graphics/new_pokenav/bg1_map.bin.lz");

static const u32 sBg1_Pokeball_Tiles[] = INCBIN_U32("graphics/new_pokenav/pokeball_tileset.4bpp.lz");
static const u32 sBg1_Pokeball_Map[] = INCBIN_U32("graphics/new_pokenav/pokeball_tileset.bin.lz");

static const u16 sBgColor[] = {RGB_WHITE};


/* SPRITES */

static const u16 iconPal[] = INCBIN_U16("graphics/new_pokenav/icons/icon_container.gbapal");
static const u8 iconSprite[] = INCBIN_U8("graphics/new_pokenav/icons/icon_container.4bpp");

static const u16 iconSelectorPal[] = INCBIN_U16("graphics/new_pokenav/icons/cursor.gbapal");
static const u32 iconSelectorSprite[] = INCBIN_U32("graphics/new_pokenav/icons/cursor.4bpp");

/* Variables globales */

static EWRAM_DATA struct NewPokenav *sNewPokenavView = NULL;

/* Declaracion implicita de funciones */

static void InitAndShowBgsFromTemplate(void);
static void MainCallBack2(void);
static void VBlankCallBack(void);
void StartNewPokenav_CB2(void);
void Callback2_StartNewPokenav(void);
static void UpdatePaletteTest(u8 frameNum);
static void SetAffineData(struct Sprite *sprite, s16 xScale, s16 yScale, u16 rotation);
static void ConstructOamMatrix(u8 spriteId, s16 sX, s16 sY, s16 rotation);

static void Task_PokeNavFadeIn(u8 taskId);
static void Task_LoadBorderBg(u8 taskId);
static void Task_FadePokeball(u8 taskId);
static void Task_WaitingForInput(u8 taskId);
static void Task_IdleState(u8 taskId);
static void Task_LoadIconsAndCursor(u8 taskId);
static void Task_GoBackToOverworld(u8 taskId);
static void Task_StartFadeOut(u8 taskId);
static void Task_SpawnIconColumns(u8 taskId);
static void Task_SpawnIconColumns_V2(u8 taskId);

static void SpawnIcons();

static void SpriteCB_moveCursor(struct Sprite *sprite);
static void SpriteCB_resizeSprite(struct Sprite *sprite);
static void SpriteCB_moveIcon(struct Sprite *sprite);


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

/* Structs Sprites */

static const struct OamData spriteIconOamData =
{
    .y = 0, 
    .affineMode = 0, //Esto es el modo afin, lo usaremos para las animaciones
    .objMode = 0, 
    .mosaic = 0, 
    .bpp = 0, 
    .shape = 0, //Hace que el sprite sea un: cuadrado = 0, rectangulo horizontal = 1, rectangulo vertical = 2
    .x = 0,
    .matrixNum = 0,
    .size = 2, //Esto indica el tamaño de nuestro sprite, más abajo está la tabla
    .tileNum = 0,
    .priority = 3, //Esto es la prioridad, a menor número, mayor prioridad, por tanto, para tener la máxima prioridad
    .paletteNum = 0,
    .affineParam = 0,
};

static const struct OamData spriteCursorOamData =
{
    .y = 0, 
    .affineMode = 0,
    .objMode = 0, 
    .mosaic = 0, 
    .bpp = 0, 
    .shape = 0, 
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(16x16), 
    .tileNum = 0,
    .priority = 0, 
    .paletteNum = 0,
    .affineParam = 0,
};

const struct SpriteSheet spriteSheetIcon =
{
	.data = iconSprite,
	.size = 512,
	.tag = 12, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};

const struct SpritePalette spritePaletteIcon =
{
	.data = iconPal,
	.tag = 8,
};

const struct SpriteSheet spriteSheetIconSelector =
{
	.data = iconSelectorSprite,
	.size = 256,
	.tag = 0,
};

const struct SpritePalette spritePaletteIconSelector =
{
	.data = iconSelectorPal,
	.tag = 1,
};


const struct SpriteTemplate spriteTemplateIcon =
{
	.tileTag = 12, 
	.paletteTag = 8, 
	.oam = &spriteIconOamData,
	.anims = gDummySpriteAnimTable,
	.images = NULL, 
	.affineAnims = gDummySpriteAffineAnimTable, 
	.callback = SpriteCallbackDummy, 
};


const union AnimCmd gAnims_CursorIdleAnim[] =
{  
    ANIMCMD_FRAME(0, 30),
    ANIMCMD_FRAME(4, 30),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const sAnims_CursortableAnim[] =
{
    gAnims_CursorIdleAnim,
};

const struct SpriteTemplate spriteTemplateCursor =
{
	.tileTag = 0, 
	.paletteTag = 1, 
	.oam = &spriteCursorOamData,
	.anims = sAnims_CursortableAnim,
	.images = NULL, 
	.affineAnims = gDummySpriteAffineAnimTable, 
	.callback = SpriteCB_moveCursor, //ESTO
};




/* Definicion de funciones */


static void InitAndShowBgsFromTemplate()
{
	InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
	LZ77UnCompVram(sBg3_Tiles, (void *) VRAM + 0x4000 * 3);
	LZ77UnCompVram(sBg3_Map, (u16*) BG_SCREEN_ADDR(26));
	LZ77UnCompVram(sBg2_Tiles, (void *) VRAM + 0x4000 * 2);
	LZ77UnCompVram(sBg2_Map, (u16*) BG_SCREEN_ADDR(18));
    //LZ77UnCompVram(sBg1_Tiles, (void *) VRAM + 0x4000 * 1);
	//LZ77UnCompVram(sBg1_Map, (u16*) BG_SCREEN_ADDR(10));

    LZ77UnCompVram(sBg1_Pokeball_Tiles, (void *) VRAM + 0x4000 * 1);
	LZ77UnCompVram(sBg1_Pokeball_Map, (u16*) BG_SCREEN_ADDR(10));

	LoadPalette(sBg3Pal, 0x00, 0x20);
	LoadPalette(sBg2Pal, 0x10, 0x20);
	
	ResetAllBgsCoordinates();

	//ShowBg(0);
	ShowBg(1);
	ShowBg(2);
	ShowBg(3);
	
}

static void printIcons()
{
    u8 spriteId0, spriteId1, spriteId2, spriteId3,
    spriteId4, spriteId5, spriteId6, spriteId7, spriteId8;

    LoadSpriteSheet(&spriteSheetIcon);
	LoadSpritePalette(&spritePaletteIcon);
    
    /*
        LEFT COLUMN = 50
        MID COLUMN = 120
        RIGHT COLUMN = 190

        UPPER ROW = 50
        MID ROW = 80
        LOWER ROW = 110
    */

	spriteId0 = CreateSprite(&spriteTemplateIcon, 50, 50, 0);
    spriteId1 = CreateSprite(&spriteTemplateIcon, 120, 50, 0);
    spriteId2 = CreateSprite(&spriteTemplateIcon, 190, 50, 0);

    spriteId3 = CreateSprite(&spriteTemplateIcon, 50, 80, 0);
    spriteId4 = CreateSprite(&spriteTemplateIcon, 120, 80, 0);
    spriteId5 = CreateSprite(&spriteTemplateIcon, 190, 80, 0);

    spriteId6 = CreateSprite(&spriteTemplateIcon, 50, 110, 0);
    spriteId7 = CreateSprite(&spriteTemplateIcon, 120, 110, 0);
    spriteId8 = CreateSprite(&spriteTemplateIcon, 190, 110, 0);

    sNewPokenavView->spriteId0 = spriteId0;
    sNewPokenavView->spriteId1 = spriteId1;
    sNewPokenavView->spriteId2 = spriteId2;
    sNewPokenavView->spriteId3 = spriteId3;
    sNewPokenavView->spriteId4 = spriteId4;
    sNewPokenavView->spriteId5 = spriteId5;
    sNewPokenavView->spriteId6 = spriteId6;
    sNewPokenavView->spriteId7 = spriteId7;
    sNewPokenavView->spriteId8 = spriteId8;
}

static void printCursor()
{
    u8 cursorId;
    LoadSpriteSheet(&spriteSheetIconSelector);
	LoadSpritePalette(&spritePaletteIconSelector);
    cursorId = CreateSprite(&spriteTemplateCursor, 133, 69, 0);
    sNewPokenavView->cursorId = cursorId;
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
        FreeAllSpritePalettes();
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

        gTasks[taskId].data[0] = 0;
        sNewPokenavView = AllocZeroed(sizeof(struct NewPokenav));
        sNewPokenavView->cursorIsMoving = FALSE;
        sNewPokenavView->rowIndex = 1;
        sNewPokenavView->columnIndex = 1;
        sNewPokenavView->columnIsMoving = FALSE;
        gMain.state++;
        break;
    case 4:
        InitAndShowBgsFromTemplate();
        LoadPalette(sBgColor, 0, 2);
        LoadPalette(GetOverworldTextboxPalettePtr(), 0xf0, 16);
        gMain.state++;
        break;
    case 5:
        
        taskId = CreateTask(Task_WaitingForInput, 0);
        //data = AllocZeroed(sizeof(struct MusicMenu));
        //SetStructPtr(taskId, data);
        gMain.state++;
        break;
    case 6:
        BeginNormalPaletteFade(-1, 0, 0x10, 0, 0);
        SetVBlankCallback(VBlankCallBack);
        SetMainCallback2(MainCallBack2);
        return;
    }
}

static void Task_LoadIconsAndCursor(u8 taskId)
{
    //printIcons();
    printCursor();
    gTasks[taskId].func = Task_IdleState;
}

static void Task_PokeNavFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        return;
}

static void Task_WaitingForInput(u8 taskId)
{

    if (gMain.newKeys & A_BUTTON)
    {
        /*
            Aqui debemos hacer un degrade sobre el BG1 hasta
            que deje de ser visible, borrar el BG1 y cargar
            el nuevo BG1, lugar en donde se printearan los
            diferentes strings.
        */
        PlaySE(SE_PN_ON);
        gTasks[taskId].data[1] = 6;
        gTasks[taskId].func = Task_FadePokeball;

    }

    if (gMain.newKeys & B_BUTTON)
    {
        gTasks[taskId].func = Task_StartFadeOut;
    }
    
    else
    {
        gTasks[taskId].data[0]++;
        UpdatePaletteTest(gTasks[taskId].data[0]);
    }
    
}


static void Task_FadePokeball(u8 taskId)
{
    if(gTasks[taskId].data[1] > 0)
    {
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG1 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_ALL);
        SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(gTasks[taskId].data[1], 16));
        gTasks[taskId].data[1] -= 1;
    }
    else
    {
        LZ77UnCompVram(sBg1_Tiles, (void *) VRAM + 0x4000 * 1);
	    LZ77UnCompVram(sBg1_Map, (u16*) BG_SCREEN_ADDR(10));
        gTasks[taskId].func = Task_LoadBorderBg;
    }
    
}

static void Task_LoadBorderBg(u8 taskId)
{
    if(gTasks[taskId].data[1] <= 6)
    {
        gTasks[taskId].data[1] += 1;
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG1 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_ALL);
        SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(gTasks[taskId].data[1], 16));
    }
    else
    {
        sNewPokenavView->taskId = taskId;
        gTasks[taskId].data[0] = 0;
        gTasks[taskId].data[1] = 0;
        gTasks[taskId].func = Task_SpawnIconColumns_V2;
        
    }
}


static void Task_WaitColumnToSpawn(u8 taskId)
{
    if(sNewPokenavView->columnIsMoving)
    {
        gTasks[taskId].func = Task_WaitColumnToSpawn;
    }
    else
    {
        gTasks[sNewPokenavView->taskId].data[0] -= 1;
        gTasks[sNewPokenavView->taskId].data[1] += 1;
        gTasks[taskId].func = Task_SpawnIconColumns;
    }
    
}

static void Task_MoveIcons(u8 taskId)
{
    if(gSprites[sNewPokenavView->spriteId0].pos2.x >= -190)
    {
        gSprites[sNewPokenavView->spriteId0].pos2.x -=10;
        gSprites[sNewPokenavView->spriteId1].pos2.x -=10;
        gSprites[sNewPokenavView->spriteId2].pos2.x -=10;
    }

    if(gSprites[sNewPokenavView->spriteId3].pos2.x >= -120)
    {
        gSprites[sNewPokenavView->spriteId3].pos2.x -=10;
        gSprites[sNewPokenavView->spriteId4].pos2.x -=10;
        gSprites[sNewPokenavView->spriteId5].pos2.x -=10;
    }

    if(gSprites[sNewPokenavView->spriteId6].pos2.x >= -50)
    {
        gSprites[sNewPokenavView->spriteId6].pos2.x -=10;
        gSprites[sNewPokenavView->spriteId7].pos2.x -=10;
        gSprites[sNewPokenavView->spriteId8].pos2.x -=10;
    }

    if(gSprites[sNewPokenavView->spriteId0].pos2.x < -190)
        gTasks[taskId].func = Task_LoadIconsAndCursor;


    
    
}


static void Task_SpawnIconColumns(u8 taskId)
{
    sNewPokenavView->taskId = taskId;
    sNewPokenavView->columnIsMoving = TRUE;
    SpawnIcons();
    gTasks[sNewPokenavView->taskId].data[0] += 1;
    gTasks[taskId].func = Task_PokeNavFadeIn;
    
}

static void Task_SpawnIconColumns_V2(u8 taskId)
{
    sNewPokenavView->taskId = taskId;
    sNewPokenavView->columnIsMoving = TRUE;
    SpawnIcons();
    gTasks[taskId].func = Task_MoveIcons;
}



static void SpawnIcons()
{
    u8 spriteId0, spriteId1, spriteId2, spriteId3,
    spriteId4, spriteId5, spriteId6, spriteId7, spriteId8;

    LoadSpriteSheet(&spriteSheetIcon);
	LoadSpritePalette(&spritePaletteIcon);

    spriteId0 = CreateSprite(&spriteTemplateIcon, 255, 50, 0);
    spriteId1 = CreateSprite(&spriteTemplateIcon, 255, 80, 0);
    spriteId2 = CreateSprite(&spriteTemplateIcon, 255, 110, 0);
    spriteId3 = CreateSprite(&spriteTemplateIcon, 255, 50, 0);
    spriteId4 = CreateSprite(&spriteTemplateIcon, 255, 80, 0);
    spriteId5 = CreateSprite(&spriteTemplateIcon, 255, 110, 0);
    spriteId6 = CreateSprite(&spriteTemplateIcon, 255, 50, 0);
    spriteId7 = CreateSprite(&spriteTemplateIcon, 255, 80, 0);
    spriteId8 = CreateSprite(&spriteTemplateIcon, 255, 110, 0); 

    sNewPokenavView->spriteId0 = spriteId0;
    sNewPokenavView->spriteId1 = spriteId1;
    sNewPokenavView->spriteId2 = spriteId2;
    sNewPokenavView->spriteId3 = spriteId3;
    sNewPokenavView->spriteId4 = spriteId4;
    sNewPokenavView->spriteId5 = spriteId5;
    sNewPokenavView->spriteId6 = spriteId6;
    sNewPokenavView->spriteId7 = spriteId7;
    sNewPokenavView->spriteId8 = spriteId8;
}



static void Task_IdleState(u8 taskId)
{
    u8 columnIndex;
    
    
    if (gMain.newKeys & B_BUTTON)
    {
        gTasks[taskId].func = Task_StartFadeOut;
    }

    if (gMain.newKeys & DPAD_LEFT)
    {
        sNewPokenavView->columnIndex = DecreaseIndex(sNewPokenavView->columnIndex);
        sNewPokenavView->cursorIsMoving = TRUE;
    }
    if (gMain.newKeys & DPAD_RIGHT)
    {
        sNewPokenavView->columnIndex = IncreaseIndex(sNewPokenavView->columnIndex);
        sNewPokenavView->cursorIsMoving = TRUE;
    }
    if (gMain.newKeys & DPAD_UP)
    {
        sNewPokenavView->rowIndex = DecreaseIndex(sNewPokenavView->rowIndex);
        sNewPokenavView->cursorIsMoving = TRUE;
    }
    if (gMain.newKeys & DPAD_DOWN)
    {
        sNewPokenavView->rowIndex = IncreaseIndex(sNewPokenavView->rowIndex);
        sNewPokenavView->cursorIsMoving = TRUE;
    }
}

static void Task_StartFadeOut(u8 taskId)
{
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 0x10, RGB_BLACK);
    PlaySE(SE_PN_OFF);
    gTasks[taskId].func = Task_GoBackToOverworld;
}

static void Task_GoBackToOverworld(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        Free(sNewPokenavView);
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        SetMainCallback2(CB2_ReturnToField);
    }
}

static void UpdatePaletteTest(u8 frameNum)
{
    if ((frameNum % 4) == 0) // Change color every 4th frame
    {
        s32 intensity = Cos(frameNum, 128) + 128;
        s32 r = 31 - ((intensity * 32 - intensity) / 269);
        s32 g = 31 - ((intensity * 32 - intensity) / 256);
        s32 b = 0;

        u16 color = RGB(r, g, b);
        LoadPalette(&color, 0x1F, sizeof(color));
   }
}

static void SpriteCB_moveCursor(struct Sprite *sprite)
{
    /*
        LEFT COLUMN = 50
        MID COLUMN = 120
        RIGHT COLUMN = 190

        UPPER ROW = 50
        MID ROW = 80
        LOWER ROW = 110
    */
    if (sNewPokenavView->cursorIsMoving)
    {
        PlaySE(SE_SELECT);
        sprite->pos2.x = 50 + (sNewPokenavView->columnIndex*70) - 120;
        sprite->pos2.y = (sNewPokenavView->rowIndex*30) - 30;
        sNewPokenavView->cursorIsMoving = FALSE;
    }
}

static void SpriteCB_moveIcon(struct Sprite *sprite)
{
    /*

        entry point: 240
        1st checkpoint: 50
    */
 
    if(sprite->pos2.x >= -( gTasks[sNewPokenavView->taskId].data[0] * 70 + 40 ) )
        sprite->pos2.x -= 10;
    else
    {
        sNewPokenavView->columnIndex = FALSE;
    }

}


static void SetAffineData(struct Sprite *sprite, s16 xScale, s16 yScale, u16 rotation)
{
    u8 matrixNum;
    struct ObjAffineSrcData affineSrcData;
    struct OamMatrix dest;

    affineSrcData.xScale = xScale;
    affineSrcData.yScale = yScale;
    affineSrcData.rotation = rotation;

    matrixNum = sprite->oam.matrixNum;

    ObjAffineSet(&affineSrcData, &dest, 1, 2);
    gOamMatrices[matrixNum].a = dest.a;
    gOamMatrices[matrixNum].b = dest.b;
    gOamMatrices[matrixNum].c = dest.c;
    gOamMatrices[matrixNum].d = dest.d;
}

static void ConstructOamMatrix(u8 spriteId, s16 sX, s16 sY, s16 rotation)
{
    s16 a, b, c, d;
    a = Cos(rotation, 16)/sX;
    b = -(Sin(rotation, 16)/sX);
    c = Sin(rotation, 16)/sY;
    d = Cos(rotation, 16)/sY;
    SetOamMatrix(gSprites[spriteId].oam.matrixNum, 
                Q_8_8(a),
                Q_8_8(b),
                Q_8_8(c),
                Q_8_8(d)); 
}