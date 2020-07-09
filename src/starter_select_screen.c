#include "global.h"
#include "battle.h"
#include "battle_factory_screen.h"
#include "battle_factory.h"
#include "sprite.h"
#include "event_data.h"
#include "overworld.h"
#include "random.h"
#include "battle_tower.h"
#include "text.h"
#include "menu_helpers.h"
#include "menu.h"
#include "palette.h"
#include "task.h"
#include "main.h"
#include "malloc.h"
#include "bg.h"
#include "gpu_regs.h"
#include "text_window.h"
#include "international_string_util.h"
#include "window.h"
#include "data.h"
#include "decompress.h"
#include "pokemon_summary_screen.h"
#include "sound.h"
#include "scanline_effect.h"
#include "pokedex.h"
#include "string.h"
#include "util.h"
#include "trainer_pokemon_sprites.h"
#include "string_util.h"
#include "constants/battle_frontier.h"
#include "constants/songs.h"
#include "constants/rgb.h"
#include "constants/moves.h"
#include "constants/species.h"


#include "main_menu.h"


//Cantidad de Iniciales en el juego

#define STARTER_MON_COUNT   13	
#define MONO_TYPE 0
#define DUAL_TYPE 1

// Posici�n del sprite del pok�mon a seleccionar

#define STARTER_PKMN_POS_X 120
#define STARTER_PKMN_POS_Y 64


#define COORD_TYPE1_ICON_X 86
#define COORD_TYPE2_ICON_X 119
#define COORD_TYPE_ICON_Y 60



const u8 gText_MenuTitle[] = _("Select a starter: ");
const u8 gText_UnkCtrlF908Clear001[] = _("{NO}{CLEAR 0x01}");
const u8 gText_AbilityTitle[] = _("ABILITY");
const u8 gText_ItemTitle[] = _("ITEM");

const u8 gText_ConfirmStarter[] = _("Are you sure?");


static const u32 sBg3_Tiles[] = INCBIN_U32("graphics/starter_select_screen/bg3_tiles.4bpp.lz");
static const u32 sBg3_Map[] = INCBIN_U32("graphics/starter_select_screen/bg3_map.bin.lz");
static const u16 sBg3Pal[] = INCBIN_U16("graphics/starter_select_screen/bg3_tiles.gbapal");
static const u32 sBg2_Tiles[] = INCBIN_U32("graphics/starter_select_screen/bg2_tiles.4bpp.lz");
static const u32 sBg2_Map[] = INCBIN_U32("graphics/starter_select_screen/bg2_map.bin.lz");
static const u16 sBg2Pal[] = INCBIN_U16("graphics/starter_select_screen/bg2_tiles.gbapal");


struct SelectScreen
{
	u8 mainState;
	u8 windowId;
	u8 pkmnIconId;
	u8 spriteTypeId;
	u8 pkmntype;
	u16 indexPos;
	u16 actualSpriteId;
	u16 pkmnNumberSpecies;
	MainCallback returnCallback;
	
};





// IWRAM bss
static IWRAM_DATA struct SelectScreen *sScreen;



static const u8 sMonCoords[STARTER_MON_COUNT][2] =
{
    {0xD0, 0x4D}, //Bulbasaur 0
    {0xD2, 0x4B},//Charmander 1
    {0xCD, 0x4B}, //Squirtle 2
	{0xCF, 0x4A}, //Pikachu 3
	{0xCE, 0x4A}, //Psyduck 4 
	{0xCC, 0x49}, //Eevee 5
	{0xCB, 0x4B}, //Chikorita 6 
	{0xCD, 0x4A}, //Cyndaquil 7
	{0xCA, 0x4D}, //Totodile 8
	{0xCE, 0x4A}, //Treecko 9
	{0xCA, 0x4C}, //Torchic 10
	{0xCC, 0x4D}, //Mudkip 11
	{0xCE, 0x50}, //Slakoth 12
	

};

static const u16 sStarterMon[STARTER_MON_COUNT] =
{
    SPECIES_BULBASAUR,
    SPECIES_CHARMANDER,
    SPECIES_SQUIRTLE,
	SPECIES_PIKACHU,
	SPECIES_PSYDUCK,
	SPECIES_EEVEE,
	SPECIES_CHIKORITA,
	SPECIES_CYNDAQUIL,
	SPECIES_TOTODILE,
	SPECIES_TREECKO,
	SPECIES_TORCHIC,
	SPECIES_MUDKIP,
	SPECIES_SLAKOTH,
};



const u8 gAbilitiesNames[][30] = {
	[SPECIES_BULBASAUR] = _("CHLOROPHYLL"),
	[SPECIES_CHARMANDER] = _("SHEER FORCE"),
	[SPECIES_SQUIRTLE] = _("DRIZZLE"),
	[SPECIES_PIKACHU] = _("LIGHTNING ROD"),
	[SPECIES_PSYDUCK] = _("UNAWARE"),
	[SPECIES_EEVEE] = _("ADAPTABILITY"),
	[SPECIES_CHIKORITA] = _("THICK FAT"),
	[SPECIES_CYNDAQUIL] = _("DROUGHT"),
	[SPECIES_TOTODILE] = _("INTIMIDATE"),
	[SPECIES_TREECKO] = _("INFILTRATOR"),
	[SPECIES_TORCHIC] = _("SPEED BOOST"),
	[SPECIES_MUDKIP] = _("ADAPTABILITY"),
	[SPECIES_SLAKOTH] = _("TRUANT"),
};

const u8 gItemsNames[][15] = {
	[SPECIES_BULBASAUR] = _("DUMMY 1"),
	[SPECIES_CHARMANDER] = _("DUMMY 2"),
	[SPECIES_SQUIRTLE] = _("DUMMY 3"),
	[SPECIES_PIKACHU] = _("DUMMY 4"),
	[SPECIES_PSYDUCK] = _("DUMMY 5"),
	[SPECIES_EEVEE] = _("DUMMY 6"),
	[SPECIES_CHIKORITA] = _("DUMMY 8"),
	[SPECIES_CYNDAQUIL] = _("DUMMY 9"),
	[SPECIES_TOTODILE] = _("DUMMY 10"),
	[SPECIES_TREECKO] = _("DUMMY 11"),
	[SPECIES_TORCHIC] = _("DUMMY 12"),
	[SPECIES_MUDKIP] = _("DUMMY 13"),
	[SPECIES_SLAKOTH] = _("DUMMY 14"),
};

const u8 gMonDexNum[][4] = {
	[SPECIES_BULBASAUR] = _("001"),
	[SPECIES_CHARMANDER] = _("004"),
	[SPECIES_SQUIRTLE] = _("003"),
	[SPECIES_PIKACHU] = _("025"),
	[SPECIES_PSYDUCK] = _("054"),
	[SPECIES_EEVEE] = _("133"),
	[SPECIES_CHIKORITA] = _("152"),
	[SPECIES_CYNDAQUIL] = _("155"),
	[SPECIES_TOTODILE] = _("158"),
	[SPECIES_TREECKO] = _("252"),
	[SPECIES_TORCHIC] = _("255"),
	[SPECIES_MUDKIP] = _("258"),
	[SPECIES_SLAKOTH] = _("287"),
	
};





// New Content




static const u16 grassPal[] = INCBIN_U16("graphics/starter_select_screen/grass.gbapal");
static const u8 grassSprite[] = INCBIN_U8("graphics/starter_select_screen/grass.4bpp");

static const u16 firePal[] = INCBIN_U16("graphics/starter_select_screen/fire.gbapal");
static const u8 fireSprite[] = INCBIN_U8("graphics/starter_select_screen/fire.4bpp");

static const u16 waterPal[] = INCBIN_U16("graphics/starter_select_screen/water.gbapal");
static const u8 waterSprite[] = INCBIN_U8("graphics/starter_select_screen/water.4bpp");

static const u16 electricPal[] = INCBIN_U16("graphics/starter_select_screen/electric.gbapal");
static const u8 electricSprite[] = INCBIN_U8("graphics/starter_select_screen/electric.4bpp");

static const u16 normalPal[] = INCBIN_U16("graphics/starter_select_screen/normal.gbapal");
static const u8 normalSprite[] = INCBIN_U8("graphics/starter_select_screen/normal.4bpp");

static const u16 poisonPal[] = INCBIN_U16("graphics/starter_select_screen/poison.gbapal");
static const u8 poisonSprite[] = INCBIN_U8("graphics/starter_select_screen/poison.4bpp");

static const u16 fightPal[] = INCBIN_U16("graphics/starter_select_screen/fight.gbapal");
static const u8 fightSprite[] = INCBIN_U8("graphics/starter_select_screen/fight.4bpp");

static const u8 darkSprite[] = INCBIN_U8("graphics/starter_select_screen/dark.4bpp");



static u8 iconNameEWRAM;
static u8 iconNameEWRAM2;


static const struct OamData spriteTypeOamData =
{
    .y = 0, //Esto no hace lo que parece, no es necesario cambiarlo
    .affineMode = 0, //Esto es el modo af�n, lo usaremos para las animaciones de tipo af�n, pero de momento, nos olvidamos y lo dejamos en 0
    .objMode = 0, //Lo dejamos en 0
    .mosaic = 0, //Lo dejamos en 0
    .bpp = 0, //Lo dejamos en 0
    .shape = 1, //Hace que el sprite sea un: cuadrado = 0, rect�ngulo horizontal = 1, rect�ngulo vertical = 2
    .x = 0, //Esto no hace lo que parece, no es necesario cambiarlo
    .matrixNum = 0, //Lo dejamos en 0
    .size = 2, //Esto indica el tama�o de nuestro sprite, m�s abajo est� la tabla
    .tileNum = 0, //Lo dejamos en 0
    .priority = 0, //Esto es la prioridad, a menor n�mero, mayor prioridad, por tanto, para tener la m�xima prioridad, lo dejamos en 0
    .paletteNum = 0, //Esto no hace lo que parece, no es necesario cambiarlo
    .affineParam = 0, //Esto lo dejaremos siempre en 0
};


static const union AnimCmd nameTypeAnimSeq0[] =
{
    ANIMCMD_FRAME(0, 5),
    ANIMCMD_END,
};

static const union AnimCmd *const nameTypeAnimTable[] =
{
    nameTypeAnimSeq0,
};



const struct SpriteSheet spriteSheetGrass =
{
	.data = grassSprite,
	.size = 512,
	.tag = 8, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};
const struct SpritePalette spritePaletteGrass =
{
	.data = grassPal,
	.tag = 8, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};

const struct SpriteSheet spriteSheetPoison =
{
	.data = poisonSprite,
	.size = 512,
	.tag = 16, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};

const struct SpritePalette spritePalettePoison =
{
	.data = poisonPal,
	.tag = 16, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};

const struct SpriteSheet spriteSheetFire =
{
	.data = fireSprite,
	.size = 512,
	.tag = 24, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};

const struct SpritePalette spritePaletteFire =
{
	.data = firePal,
	.tag = 24, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};

const struct SpriteSheet spriteSheetWater =
{
	.data = waterSprite,
	.size = 512,
	.tag = 32, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};

const struct SpritePalette spritePaletteWater =
{
	.data = waterPal,
	.tag = 32, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};

const struct SpriteSheet spriteSheetNormal =
{
	.data = normalSprite,
	.size = 512,
	.tag = 40, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};

const struct SpritePalette spritePaletteNormal =
{
	.data = normalPal,
	.tag = 40, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};

const struct SpriteSheet spriteSheetElectric =
{
	.data = electricSprite,
	.size = 512,
	.tag = 48, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};

const struct SpritePalette spritePaletteElectric =
{
	.data = electricPal,
	.tag = 48, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};

const struct SpriteSheet spriteSheetFight =
{
	.data = fightSprite,
	.size = 512,
	.tag = 56, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};

const struct SpritePalette spritePaletteFight =
{
	.data = fightPal,
	.tag = 56, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};

const struct SpriteSheet spriteSheetDark =
{
	.data = darkSprite,
	.size = 512,
	.tag = 64, //Este es lugar en que el sistema dibujar� el sprite en la OAM
};

//const struct SpritePalette spritePaletteDark =
//{
//	.data = fightPal,
//	.tag = 64, //Este es lugar en que el sistema dibujar� el sprite en la OAM
//};


const struct SpriteTemplate spriteTemplateGrass =
{
	.tileTag = 8, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.paletteTag = 8, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.oam = &spriteTypeOamData,
	.anims = nameTypeAnimTable,
	.images = NULL, //Lo dejaremos siempre como NULL
	.affineAnims = gDummySpriteAffineAnimTable, //Esto es para las animaciones afines, de nuevo, si no las usamos, dejaremos esto por defecto
	.callback = SpriteCallbackDummy, //Esto es para las animaciones, si no las usaremos, tambi�n se quedar� as�
};


const struct SpriteTemplate spriteTemplatePoison =
{
	.tileTag = 16, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.paletteTag = 16, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.oam = &spriteTypeOamData,
	.anims = nameTypeAnimTable,
	.images = NULL, //Lo dejaremos siempre como NULL
	.affineAnims = gDummySpriteAffineAnimTable, //Esto es para las animaciones afines, de nuevo, si no las usamos, dejaremos esto por defecto
	.callback = SpriteCallbackDummy, //Esto es para las animaciones, si no las usaremos, tambi�n se quedar� as�
};

const struct SpriteTemplate spriteTemplateFire =
{
	.tileTag = 24, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.paletteTag = 24, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.oam = &spriteTypeOamData,
	.anims = nameTypeAnimTable,
	.images = NULL, //Lo dejaremos siempre como NULL
	.affineAnims = gDummySpriteAffineAnimTable, //Esto es para las animaciones afines, de nuevo, si no las usamos, dejaremos esto por defecto
	.callback = SpriteCallbackDummy, //Esto es para las animaciones, si no las usaremos, tambi�n se quedar� as�
};

const struct SpriteTemplate spriteTemplateWater =
{
	.tileTag = 32, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.paletteTag = 32, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.oam = &spriteTypeOamData,
	.anims = nameTypeAnimTable,
	.images = NULL, //Lo dejaremos siempre como NULL
	.affineAnims = gDummySpriteAffineAnimTable, //Esto es para las animaciones afines, de nuevo, si no las usamos, dejaremos esto por defecto
	.callback = SpriteCallbackDummy, //Esto es para las animaciones, si no las usaremos, tambi�n se quedar� as�
};

const struct SpriteTemplate spriteTemplateNormal =
{
	.tileTag = 40, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.paletteTag = 40, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.oam = &spriteTypeOamData,
	.anims = nameTypeAnimTable,
	.images = NULL, //Lo dejaremos siempre como NULL
	.affineAnims = gDummySpriteAffineAnimTable, //Esto es para las animaciones afines, de nuevo, si no las usamos, dejaremos esto por defecto
	.callback = SpriteCallbackDummy, //Esto es para las animaciones, si no las usaremos, tambi�n se quedar� as�
};

const struct SpriteTemplate spriteTemplateElectric =
{
	.tileTag = 48, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.paletteTag = 48, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.oam = &spriteTypeOamData,
	.anims = nameTypeAnimTable,
	.images = NULL, //Lo dejaremos siempre como NULL
	.affineAnims = gDummySpriteAffineAnimTable, //Esto es para las animaciones afines, de nuevo, si no las usamos, dejaremos esto por defecto
	.callback = SpriteCallbackDummy, //Esto es para las animaciones, si no las usaremos, tambi�n se quedar� as�
};

const struct SpriteTemplate spriteTemplateFight =
{
	.tileTag = 56, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.paletteTag = 56, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.oam = &spriteTypeOamData,
	.anims = nameTypeAnimTable,
	.images = NULL, //Lo dejaremos siempre como NULL
	.affineAnims = gDummySpriteAffineAnimTable, //Esto es para las animaciones afines, de nuevo, si no las usamos, dejaremos esto por defecto
	.callback = SpriteCallbackDummy, //Esto es para las animaciones, si no las usaremos, tambi�n se quedar� as�
};

const struct SpriteTemplate spriteTemplateDark =
{
	.tileTag = 64, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.paletteTag = 56, //Este es lugar en que el sistema dibujar� el sprite en la OAM
	.oam = &spriteTypeOamData,
	.anims = nameTypeAnimTable,
	.images = NULL, //Lo dejaremos siempre como NULL
	.affineAnims = gDummySpriteAffineAnimTable, //Esto es para las animaciones afines, de nuevo, si no las usamos, dejaremos esto por defecto
	.callback = SpriteCallbackDummy, //Esto es para las animaciones, si no las usaremos, tambi�n se quedar� as�
};


enum { YES_NO_WINDOW2 };


static const struct WindowTemplate sWindowTemplate_YesNo[] =

{
	[YES_NO_WINDOW2] = {
		.bg = 0,
        .tilemapLeft = 23,
        .tilemapTop = 13,
        .width = 5,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 60,
    }
};


enum { NAME_PKMN, DEX_NUM, ABILITY_NAME, ITEM_NAME, SCREEN_TITLE, ARE_YOU_SURE_TEXTBOX};

static const struct WindowTemplate sWindowTemplate_MiPlantilla[] = 
{
	[NAME_PKMN] = {
		.bg = 0,
		.tilemapLeft = 22,
		.tilemapTop = 3,
		.width = 15,
		.height = 3,
		.paletteNum = 15,
		.baseBlock = 1,
	},
	[DEX_NUM] = {
		.bg = 0,
		.tilemapLeft = 26,
		.tilemapTop = 5,
		.width = 15,
		.height = 2,
		.paletteNum = 15,
		.baseBlock = 90,
	},
	[ABILITY_NAME] = {
		.bg = 0,
		.tilemapLeft = 9,
		.tilemapTop = 8,
		.width = 16,
		.height = 2,
		.paletteNum = 15,
		.baseBlock = 120,
	},
	[SCREEN_TITLE] = {
		.bg = 0,
		.tilemapLeft = 1,
		.tilemapTop = 0,
		.width = 15,
		.height = 2,
		.paletteNum = 15,
		.baseBlock = 160,
	},
	[ITEM_NAME] = {
		.bg = 0,
		.tilemapLeft = 9,
		.tilemapTop = 10,
		.width = 15,
		.height = 2	,
		.paletteNum = 15,
		.baseBlock = 190,
	},
	[ARE_YOU_SURE_TEXTBOX] = {
		.bg = 0,
		.tilemapLeft = 1,
		.tilemapTop = 0,
		.width = 15,
		.height = 2,
		.paletteNum = 15,
		.baseBlock = 38,
	}


};



static const struct BgTemplate sBgTemplateTutorial[3] =
{
	{
		.bg = 0,
		.charBaseIndex = 0, // 
		.mapBaseIndex = 31, // 
		/*
			char: Direccion de VRAM donde se cargan los tiles. Aquí va en incrementos de 4000
			base: Direccion de la VRAM donde se descomprimen los tiles. va en incrementos de 800
				ejemplo: 6000000 + 0x800 * 10 = 6008000
		*/
		.screenSize = 0,
		.paletteMode = 0,
		.priority = 1,
		.baseTile = 0,
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
	},
};



bool8 StartTutorialMenu_CB2();
void StarterSelectScreen_CB2(MainCallback returnCallback);
void CB2_StarterMenuSelectScreen();
static void Update_CB2();
static void LoadStarterMenuBGs();
bool8 StartOption_CB2();



static void SetUpFunc();
static void InitConstTexts();
static void InitMonText(u8 actualPos);
static void InitMon();
void C2_StarterSelectScreen(void);
static void Task_InitBasicStructures(u8 taskId);
static void Task_MainInputs(u8 taskId);
static void Task_ReturnChoose2(u8 taskId);
static void Task_MainInputs(u8 taskId);
static void Task_InitTextWindowsAndPokemon(u8 taskId);
static void AnimateSelectedPartyIcon(u8 spriteId, u8 a);
static void UpdatePartyMonIconFrameAndBounce(struct Sprite *sprite);
static void UpdatePartyMonIconFrame(struct Sprite *sprite);
static void CreateTextBoxType(u8 windowId, u8 font, const u8* colourTable, u8 top, u8 left);
static void CreateMoveTypeIcons(void);
static void SetSpriteInvisibility(u8 spriteArrayId, bool8 invisible);
static void Task_StarterChoose5(u8 taskId);
static void Task_Confirm(u8 taskId);
static void Task_OptionMenuFadeOut(u8 taskId);
bool8 MainState_StarterBeginFadeInOut();
bool8 MainState_StarterWaitFadeOutAndExit(MainCallback returnCallback);
static void Task_FadeOutAndDelete(u8 taskId);
void InitMyTextWindows();
void AddText(const u8* text);
void CreateTextBox(u8 windowId, u8 font, const u8* colourTable, u8 left, u8 top);
void CreateTextBoxType(u8 windowId, u8 font, const u8* colourTable, u8 top, u8 left);
static void MainState_BeginFadeIn(u8 taskId);




static u16 CreateFrontSprite(u16 species, u8 x, u8 y)
{
	/*
		LE ENTREGAS LA "ESPECIE" DE POKéMON PARA DIBUJAR SU SPRITE FRONTAL
		EN LAS COORDENADAS X, Y
	*/
    u16 spriteId;
	struct Sprite *sprite;
    spriteId = CreatePicSprite2(species, 8, 0, 1, x, y, 0xE, 0xFFFF);
	sprite = &gSprites[spriteId];
    gSprites[spriteId].oam.priority = 2;
	BattleAnimateFrontSprite(sprite, sStarterMon[sScreen->indexPos], FALSE, 1);

    return spriteId;
}

static void scrollBgX(u8 bg, s32 value, u8 op) 
{
	/*
		DESPLAZA EL bg Y LO DESPLAZA EN EL EJE X A UNA VELOCIDAD op
	*/
	ChangeBgX(bg, value, op);
}

static void scrollBgY(u8 bg, s32 value, u8 op)
{
	/*
		DESPLAZA EL bg Y LO DESPLAZA EN EL EJE Y A UNA VELOCIDAD op
	*/
	ChangeBgY(bg, value, op);
}


static void LoadStarterMenuBGs()
{
	/*
	A diferencia de de naming screen, aquí no usamos DmaClear ni SetGpuReg
	Tampoco usa ResetBgsAndClearDma3BusyFlags
	DMA := Direct Access Memory. Se utiliza para transferir data de manera rápida
	*/
	InitBgsFromTemplates(0, sBgTemplateTutorial, ARRAY_COUNT(sBgTemplateTutorial));
	LZ77UnCompVram(sBg3_Tiles, (void *) VRAM + 0x4000 * 3);
	LZ77UnCompVram(sBg3_Map, (u16*) BG_SCREEN_ADDR(26));
	LZ77UnCompVram(sBg2_Tiles, (void *) VRAM + 0x4000 * 2);
	LZ77UnCompVram(sBg2_Map, (u16*) BG_SCREEN_ADDR(18));

	LoadPalette(sBg3Pal, 0x00, 0x20);
	LoadPalette(sBg2Pal, 0x11, 0x20);
	
	ResetAllBgsCoordinates();
	
	ShowBg(0);
	ShowBg(1);
	ShowBg(2);
	ShowBg(3);
	
}

static void VBlankCB_StarterSelectScreen()
{
	/*
		Aquí también deberíamos encargarnos de setear
		los registos de la GPU
	*/
	LoadOam();
	scrollBgY(3, 69, 2);	
	scrollBgX(3, 69, 2);
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}


static void printMonType(u16 indexPos)
{
	/*
		Esta función se encarga de encarga de retornar
		el/los sprites asociados al typing de cada pokémon.
		Esta función recibe solo como parametro el indice
		en el que estamos actualmente en el menu (hard-coding) 

	*/
	switch(indexPos){
		case 0:
			LoadSpriteSheet(&spriteSheetGrass);
			LoadSpritePalette(&spritePaletteGrass);
			iconNameEWRAM = CreateSprite(&spriteTemplateGrass, COORD_TYPE1_ICON_X, COORD_TYPE_ICON_Y, 0);
			LoadSpriteSheet(&spriteSheetPoison);
			LoadSpritePalette(&spritePalettePoison);
			iconNameEWRAM2 = CreateSprite(&spriteTemplatePoison, COORD_TYPE2_ICON_X, COORD_TYPE_ICON_Y, 0);
			break;

		case 6:
		case 9:
		case 13:
			LoadSpriteSheet(&spriteSheetGrass);
			LoadSpritePalette(&spritePaletteGrass);
			iconNameEWRAM = CreateSprite(&spriteTemplateGrass, COORD_TYPE1_ICON_X, COORD_TYPE_ICON_Y, 0);
			break;

		case 1:
		case 7:
		case 10:
		case 14:
			LoadSpriteSheet(&spriteSheetFire);
			LoadSpritePalette(&spritePaletteFire);
			iconNameEWRAM = CreateSprite(&spriteTemplateFire, COORD_TYPE1_ICON_X, COORD_TYPE_ICON_Y, 0);
			break;
		case 2:
		case 4:
		case 8:
		case 11:
		case 15:
			LoadSpriteSheet(&spriteSheetWater);
			LoadSpritePalette(&spritePaletteWater);
			iconNameEWRAM = CreateSprite(&spriteTemplateWater, COORD_TYPE1_ICON_X, COORD_TYPE_ICON_Y, 0);
			break;

		case 5:
		case 12:
		case 17:
			LoadSpriteSheet(&spriteSheetNormal);
			LoadSpritePalette(&spritePaletteNormal);
			iconNameEWRAM = CreateSprite(&spriteTemplateNormal, COORD_TYPE1_ICON_X, COORD_TYPE_ICON_Y, 0);
			break;

		case 16:
			LoadSpriteSheet(&spriteSheetFight);
			LoadSpritePalette(&spritePaletteFight);
			iconNameEWRAM = CreateSprite(&spriteTemplateFight, COORD_TYPE1_ICON_X, COORD_TYPE_ICON_Y, 0);
			break;

		case 3:
			LoadSpriteSheet(&spriteSheetElectric);
			LoadSpritePalette(&spritePaletteElectric);
			iconNameEWRAM = CreateSprite(&spriteTemplateElectric, COORD_TYPE1_ICON_X, COORD_TYPE_ICON_Y, 0);
			break;

		case 18:
			LoadSpriteSheet(&spriteSheetDark);
			LoadSpritePalette(&spritePaletteFight);
			iconNameEWRAM = CreateSprite(&spriteTemplateDark, COORD_TYPE1_ICON_X, COORD_TYPE_ICON_Y, 0);
			break;
	}
}


static void Update_CB2()
{
	RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

bool8 StartTutorialMenu_CB2() 
{
	if(!gPaletteFade.active)
	{
		gMain.state = 0;
		//RemoveExtraStartMenuWindows();
		CleanupOverworldWindowsAndTilemaps();
		SetMainCallback2(CB2_StarterMenuSelectScreen);

		return;
	}

	return;
}


void StarterSelectScreen_CB2(MainCallback returnCallback)
{
	/* 
		Función utilizada en main_menu.c. Esta recibe la función que se debe
		utilizar una vez termina el proceso y se encarga de iniciar la
		generación del menú.
	*/
	sScreen->returnCallback = returnCallback;
	if(!gPaletteFade.active)
	{
		gMain.state = 0;
		//RemoveExtraStartMenuWindows();
		CleanupOverworldWindowsAndTilemaps();
		SetMainCallback2(C2_StarterSelectScreen);

		return;
	}

	return;

}

bool8 StartOption_CB2()
{
	if(!gPaletteFade.active)
	{
		gMain.state = 0;
		CleanupOverworldWindowsAndTilemaps();
		SetMainCallback2(C2_StarterSelectScreen);

		return TRUE;
	}

	return FALSE;
}



void CB2_StarterMenuSelectScreen()
{
	/*
		Esta función se encarga de iniciar el menú de los iniciales,
		mas no de la gestión del mismo. Solo prepara el entorno. Lo
		importante de aquí es el CreateTask. Desde ahí se deriva todo

	*/
	switch (gMain.state)
	{

		case 0:
			SetVBlankCallback(NULL);
    		SetHBlankCallback(NULL);
			gMain.state++;
			break;
		case 1: 
			SetVBlankHBlankCallbacksToNull();
			ScanlineEffect_Stop();
			ResetTasks();
			ResetSpriteData();
			ResetPaletteFade();
			FreeAllSpritePalettes();
			DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
			gMain.state++;
			break;
		
		case 2:
			CreateTask(Task_InitBasicStructures, 0);
			gMain.state++;
			break;

		default:
			BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
			SetVBlankCallback(VBlankCB_StarterSelectScreen);
			//CreateTask(FadeOut);
			SetMainCallback2(Update_CB2);
			break;
	}
}

void C2_StarterSelectScreen(void)
{
	/*
		Variante de la función anterior (test purposes only)
	*/
	switch (gMain.state)
    {
    	case 0:
			SetVBlankCallback(NULL);
    		SetHBlankCallback(NULL);
			sScreen->mainState = 0;
			gMain.state++;
			break;
		case 1:
			DmaClear32(3, (void *)OAM, OAM_SIZE);
    		DmaClear16(3, (void *)PLTT, PLTT_SIZE);
			DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
			SetVBlankHBlankCallbacksToNull();
			ScanlineEffect_Stop();
			ResetTasks();
			ResetSpriteData();
			ResetPaletteFade();
			FreeAllSpritePalettes();
			gMain.state++;
			break;
		case 2:
			ResetPaletteFade();
			gMain.state++;
			break;
		case 3:
			ResetSpriteData();
			FreeAllSpritePalettes();
			gMain.state++;
			break;
		case 4:
			ResetTasks();
			gMain.state++;
			break;
		case 5:
			UpdatePaletteFade();
			LoadStarterMenuBGs();
			gMain.state++;
			break;
		default:
			//CrearTask e Iniciar Callback
			
			SetUpFunc();
			//BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
			SetVBlankCallback(VBlankCB_StarterSelectScreen);
			SetMainCallback2(Update_CB2);
			CreateTask(Task_InitBasicStructures, 0);
			//CreateTask(FadeOut);
			
			break;

	}
}

const u8 sFontColourTableType[] = {0, 1, 2, 0};
const u8 sFontColourTableTypeBlack[] = {0, 2, 3, 0};


static void SetUpFunc()
{
	CreateTask(MainState_BeginFadeIn, 2);
	SetMainCallback2(Update_CB2);
}


static void MainState_BeginFadeIn(u8 taskId)
{
	switch (sScreen->mainState)
	{
		case 0:
			BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
    		sScreen->mainState++;
			break;
		case 1:
			//InitTextWindowBoxes
			if (!gPaletteFade.active)
				sScreen->mainState++;
			break;

		case 2:
			break;

		case 3:
			MainState_StarterBeginFadeInOut();
			//sScreen->mainState++;

			break;
		case 4:
			//MainState_StarterWaitFadeOutAndExit(sScreen->returnCallback);
			break;
			
		//case 4:
		//	sScreen->mainState++;
		//	break;
	}
}




static void Task_InitBasicStructures(u8 taskId)
{
	u8 spriteId;
	
	sScreen = AllocZeroed(sizeof(*sScreen));
	sScreen->mainState = 0;
	sScreen -> indexPos = 0;
	
	LoadStarterMenuBGs();	
	InitMyTextWindows();
	InitConstTexts();
	
	//taskId = CreateTask(Task_InitTextWindowsAndPokemon, 0);
	//InitMon();
	gTasks[taskId].func = Task_InitTextWindowsAndPokemon;
	return;


}


static void Task_MainInputs(u8 taskId)
{
	u8 spriteId;
	if (gMain.newKeys & A_BUTTON)
	{	
		//DisplayYesNoMenuDefaultYes();
		//LoadUserWindowBorderGfx(YES_NO_WINDOW, 211, 0xe0);
		PlaySE(SE_SELECT);
		ScheduleBgCopyTilemapToVram(0);
		CreateYesNoMenu(&sWindowTemplate_YesNo[YES_NO_WINDOW2], 640, 14, 0);
		
		LoadUserWindowBorderGfx(YES_NO_WINDOW2, 640, 224);	
		//DisplayYesNoMenuDefaultYes();
		ClearWindowTilemap(SCREEN_TITLE);
		RemoveWindow(SCREEN_TITLE);
		CleanupOverworldWindowsAndTilemaps();
		
		//LoadUserWindowBorderGfx(ARE_YOU_SURE_TEXTBOX, 960, 224);	
		gTasks[taskId].func = Task_Confirm;
	}

	if ( (gMain.newKeys & DPAD_RIGHT) && (!gPaletteFade.active))
	{
		PlaySE(SE_SELECT);
		
		//BeginNormalPaletteFade(0xFFFFFFFF, 1, 16, 0, RGB_BLACK);

		FreeOamMatrix(gSprites[sScreen->actualSpriteId].oam.matrixNum);
		
		ClearWindowTilemap(NAME_PKMN);
		RemoveWindow(NAME_PKMN);
		ClearWindowTilemap(DEX_NUM);
		RemoveWindow(DEX_NUM);
		FreeAndDestroyMonPicSprite(sScreen->actualSpriteId);

		DestroySpriteAndFreeResources(&gSprites[iconNameEWRAM]);
		DestroySpriteAndFreeResources(&gSprites[iconNameEWRAM2]);

		CleanupOverworldWindowsAndTilemaps();
		
		if( (sScreen-> indexPos) == (STARTER_MON_COUNT-1) )
		{
			sScreen->indexPos = 0;
			gTasks[taskId].func = Task_InitTextWindowsAndPokemon;
		}
		else
		{
			sScreen->indexPos = (sScreen->indexPos) + 1;
			gTasks[taskId].func = Task_InitTextWindowsAndPokemon;
		}
		
	}
	if ( (gMain.newKeys & DPAD_LEFT) && (!gPaletteFade.active))
	{
		PlaySE(SE_SELECT);
		//BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
		FreeOamMatrix(gSprites[sScreen->actualSpriteId].oam.matrixNum);
		
		ClearWindowTilemap(NAME_PKMN); //DEX_NUM
		RemoveWindow(NAME_PKMN);
		ClearWindowTilemap(DEX_NUM);
		RemoveWindow(DEX_NUM);
		FreeAndDestroyMonPicSprite(sScreen->actualSpriteId);
		DestroySpriteAndFreeResources(&gSprites[iconNameEWRAM]);
		DestroySpriteAndFreeResources(&gSprites[iconNameEWRAM2]);
		CleanupOverworldWindowsAndTilemaps();
		
		if( (sScreen->indexPos) == 0 )
		{
			sScreen->indexPos = STARTER_MON_COUNT-1;
			gTasks[taskId].func = Task_InitTextWindowsAndPokemon;
		}
		else
		{
			sScreen->indexPos = (sScreen->indexPos) -1;
			gTasks[taskId].func = Task_InitTextWindowsAndPokemon;
		}
	}
}


static void Task_InitTextWindowsAndPokemon(u8 taskId)
{
	u16 actualPos = sScreen->indexPos;
	
	InitMyTextWindows();		
	InitMon();
	
	
	//CreateMoveTypeIcons();
	gTasks[taskId].func = Task_MainInputs;
}

static void Task_ReturnChoose2(u8 taskId)
{
	//CreateLabels
	//BeginNormalPaletteFade(0xFFFFFFFF, 1, 0, 16, RGB_BLACK);
	gTasks[taskId].func = Task_MainInputs;
}



bool8 MainState_StarterBeginFadeInOut()
{
    BeginNormalPaletteFade(0xFFFFFFFF, 1, 0, 16, RGB_BLACK);
    sScreen->mainState;
    return FALSE;
}


bool8 MainState_StarterWaitFadeOutAndExit(MainCallback returnCallback)
{
	if(!gPaletteFade.active){
		//ResetBgsAndClearDma3BusyFlags(0);
		//ClearWindowTilemap(NAME_PKMN); //DEX_NUM
		//RemoveWindow(NAME_PKMN);
		//ClearWindowTilemap(DEX_NUM);
		//RemoveWindow(DEX_NUM);
		//ClearWindowTilemap(ARE_YOU_SURE_TEXTBOX);
		//RemoveWindow(ARE_YOU_SURE_TEXTBOX);
		//ClearWindowTilemap(ABILITY_NAME);
		//RemoveWindow(ABILITY_NAME);
		//ClearWindowTilemap(ITEM_NAME);
		//RemoveWindow(ITEM_NAME);
        //SetMainCallback2(returnCallback); //ATENCION
		SetMainCallback2(sScreen->returnCallback); //ATENCION
        DestroyTask(FindTaskIdByFunc(Task_FadeOutAndDelete));
        FreeAllWindowBuffers();
        FREE_AND_SET_NULL(sScreen);
    
    	return FALSE;
	}
}

static void Task_FadeOutAndDelete(u8 taskId)
{
	switch (sScreen->mainState)
	{
		case 0:
			MainState_StarterBeginFadeInOut();
			sScreen->mainState++;
			break;
		
		case 1:
			MainState_StarterWaitFadeOutAndExit(sScreen->returnCallback);
			break;
	}
}

static void Task_Confirm(u8 taskId)
{
	
	InitMyTextWindows();
	AddText(gText_ConfirmStarter);
	CreateTextBox(SCREEN_TITLE, 1, sFontColourTableType, 0, 0);
	
	switch (Menu_ProcessInputNoWrapClearOnChoose())
    {
    	case 0:  // YES
    	    // Return the starter choice and exit.
    	    
			//MainState_StarterWaitFadeOutAndExit(sScreen->returnCallback);
			
			sScreen->mainState++;
			GoBackToBirchScene(sScreen->pkmnNumberSpecies, taskId);
    		break;

    	case 1:  // NO
    	case -1: // B button 

			ClearStdWindowAndFrameToTransparent(YES_NO_WINDOW2, TRUE);
			RemoveWindow(YES_NO_WINDOW2);
			CleanupOverworldWindowsAndTilemaps();

			gTasks[taskId].func = Task_MainInputs;
    	    break;

    }
}

static void InitMonText(u8 actualPos)
{
	const u8 *speciesName;
	const u8 *abilityName;
	const u8 *itemName;
	u16 dexNum;
	u8 dexHolderNum;

	speciesName = gSpeciesNames[sStarterMon[actualPos]];
	abilityName = gAbilitiesNames[sStarterMon[actualPos]];
	itemName = gItemsNames[sStarterMon[actualPos]];

	dexNum = SpeciesToPokedexNum(sStarterMon[actualPos]);
	sScreen->pkmnNumberSpecies = sStarterMon[actualPos];
	StringCopy(gStringVar1, &gText_UnkCtrlF908Clear001[0]);
    ConvertIntToDecimalStringN(gStringVar3, dexNum, 2, 3);
    StringAppend(gStringVar1, gStringVar3);

	
	printMonType(actualPos);
	AddText(abilityName);
	CreateTextBox(ABILITY_NAME, 0, sFontColourTableTypeBlack, 0, 3);
	AddText(itemName);
	CreateTextBox(ITEM_NAME, 0, sFontColourTableTypeBlack, 0, 3);
	
	
	AddText(gStringVar1);
	CreateTextBox(DEX_NUM, 0, sFontColourTableType, 4, 4);
	
	AddText(speciesName);
	CreateTextBox(NAME_PKMN, 1, sFontColourTableType, 0, 6);
	

}



/* FUNCIONES AUXILIARES */

static void InitMon()
{
	u16 actualPos;
	u16 spriteId;
	actualPos = sScreen->indexPos;
	spriteId = CreateFrontSprite(sStarterMon[actualPos], sMonCoords[actualPos][0], sMonCoords[actualPos][1]);
	
	//PlayCry1(sStarterMon[actualPos], 0);
	sScreen -> actualSpriteId = spriteId;
	InitMonText(actualPos);
}

void InitMyTextWindows()
{
	InitWindows(sWindowTemplate_MiPlantilla);	//Inicializa los window
	DeactivateAllTextPrinters(); 	//Desactiva cualquier posible Text printer que haya quedado abierto
	LoadPalette(GetOverworldTextboxPalettePtr(), 0xf0, 0x20);	//Carga la paleta del texto por defecto del juego en el slot 15 de las paletas para backgrounds
	//LoadUserWindowBorderGfx(YES_NO_WINDOW, 211, 0xe0); //Opcional, necesario si se van a usar ventanas Yes/no
}


void AddText(const u8* text)
{
    StringExpandPlaceholders(gStringVar4, text); //Coloca el texto en el buffer de strings
}


const u8 sFontColourTable[] = {0, 1, 2, 0};



void CreateTextBox(u8 windowId, u8 font, const u8* colourTable, u8 left, u8 top)
{
	FillWindowPixelBuffer(windowId, 0);	//Llena todos los pixeles del Buffer con el color indicado (0-> transparencia)
    PutWindowTilemap(windowId);	//Coloca el tilemap de los gr�ficos
    AddTextPrinterParameterized3(windowId, font, left, top, colourTable, -1, gStringVar4); //Asigna un textPrinter
    CopyWindowToVram(windowId, 3);	//Copia la ventana del buffer a la VRam
}

void CreateTextBoxType(u8 windowId, u8 font, const u8* colourTable, u8 top, u8 left)
{
	FillWindowPixelBuffer(windowId, 0);	//Llena todos los pixeles del Buffer con el color indicado (0-> transparencia)
    PutWindowTilemap(windowId);	//Coloca el tilemap de los gr�ficos
    AddTextPrinterParameterized3(windowId, font, left, top, colourTable, -1, gStringVar4); //Asigna un textPrinter
    CopyWindowToVram(windowId, 3);	//Copia la ventana del buffer a la VRam
}


static void InitConstTexts()
{
	AddText(gText_MenuTitle);
	CreateTextBox(SCREEN_TITLE, 1, sFontColourTableType, 0, 0);
}



static void Task_OptionMenuFadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        PlaySE(SE_SELECT);
		FreeOamMatrix(gSprites[sScreen->actualSpriteId].oam.matrixNum);

		FreeAndDestroyMonPicSprite(sScreen->actualSpriteId);
		
		ClearWindowTilemap(NAME_PKMN); //DEX_NUM
		RemoveWindow(NAME_PKMN);
		ClearWindowTilemap(DEX_NUM);
		RemoveWindow(DEX_NUM);
		CleanupOverworldWindowsAndTilemaps();
		
        FreeAllWindowBuffers();
        //UpdateMonData(data);
        Free(sScreen);
        DestroyTask(taskId);
		//gTasks[taskId].func = Task_NewGameBirchSpeech_AreYouReady;
        //SetMainCallback2(Task_NewGameBirchSpeech_AreYouReady); //OJO
    }
}


