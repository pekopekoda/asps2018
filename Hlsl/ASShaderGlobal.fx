#define GLOBALSPEED 5
#define FIELDSTOTAL 50
#define SQRT_2 1.41421356

//GRAVITY and RESISTANCE definitions located in ASPS_parameters.txt
// Used with interface input query to warn program user may change currently picked field's parameters, or about different events :
#define CLICK						1		//Correspond to a trigger which means "right mouse button has just been pushed"
#define CHANGE_TYPE					0x54	// type may be changed
#define CHANGE_SIZE					0x53	// size may be changed
#define CHANGE_CENTER_FORCE			0x43	// center force may be changed
#define CHANGE_EXTREMITY_FORCE		0x58	// extremity force may be changed
#define CHANGE_INTERPOLATION		0x49	// interpolation may be changed
#define GRAVITY_ON					-1		// turn gravity on/off
#define GRAVITY_OFF					-2		// turn gravity on/off
#define EMISSION_TYPE				0x45	// particles emitted at center of emitter / randomly in emitter area
#define CLEAR_CAM					0x60	// camera is reseted to its initial position
#define SHOW_PANEL					80		// show explanations
#define CHANGE_RATE					82		// emission rate about to be updated
#define MOVE_OBJECT					77		// Translate mode

#define EMITTER_TYPE 0
#define FORCE_TYPE 1
#define PORTAL_TYPE 2
#define BOUNCER_TYPE 3

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
uniform int g_dispCoordsSize = 15;
struct hudcoords { int index; float4 coords; };
hudcoords g_dispCoords[] = //Display coords in HUD single texture
{
	{0							,float4(0.0,  0.0 , 0.0 , 0.0 )},
	{ CHANGE_TYPE				,float4(0.0 , 0.1 , 0.3 , 0.15)}, // CHANGE_TYPE					
	{ CHANGE_SIZE				,float4(0.0 , 0.15, 0.2 , 0.2) }, // CHANGE_SIZE					
	{ CHANGE_CENTER_FORCE		,float4(0.0 , 0.2 , 0.42, 0.25) }, // CHANGE_CENTER_FORCE			
	{ CHANGE_EXTREMITY_FORCE	,float4(0.55, 0.0 , 1.0 , 0.04) }, // CHANGE_EXTREMITY_FORCE		
	{ CHANGE_INTERPOLATION		,float4(0.55, 0.04, 1.0 , 0.1) }, // CHANGE_INTERPOLATION
	{ GRAVITY_ON				,float4(0.0 , 0.0 , 0.1 , 0.03) }, // SWITCH_GRAVITY_ON
	{ GRAVITY_OFF				,float4(0.0 , 0.05, 0.1 , 0.1) }, // SWITCH_GRAVITY_OFF
	{ EMISSION_TYPE				,float4(0.55, 0.14, 1.0 , 0.18) }, // EMISSION_TYPE			
	{ CLEAR_CAM					,float4(0.0 , 0.0 , 0.0 , 0.0) }, // CLEAR_CAM
	{ SHOW_PANEL				,float4(0.0, 0.28, 1.0 , 1.0 ) }  // SHOW_PANEL

};
float4 g_colorCoords	= float4(0.0, 0.25,0.24, 0.31) ;
float4 g_texCoords		= float4(0.0, 0.31, 0.24, 0.37) ;
float4 g_toonCoords		= float4(0.0, 0.39, 0.24, 0.42) ;
float4 g_diffSpecCoords	= float4(0.0, 0.45, 0.24, 0.48) ;
float4 g_bumpCoords	 	= float4(0.0, 0.5, 0.24, 0.55) ;
float4 g_DOFCoords		= float4(0.0, 0.55, 0.24, 0.6) ;
float4 g_glowCoords		= float4(0.0, 0.6, 0.24, 1.0) ;
float4 g_clickExpl 		= float4(0.0, 0.75, 1.0 , 1.0 ) ;

static const float PI = 3.14159265f;
float3 LightDirection = {-0.57735,-0.57735,0.57735};
float  FocusDist	  = 25.0;
float  FocusRange	  = 0.009;

float  ParticlesSize  = 3;
//matrix matWorldViewProjection;
matrix matView;
matrix matProj;
matrix matWorld;
float g_time;
//Time elapsed between each frame
float g_deltaTime;

float g_randX, g_randY, g_randZ;

uint g_maxParticles;		// Max particles buffer can contain
float g_timeToNextEmit;				// Trigger for the emitter to emit
float g_rate;				// Particles to emit per second
float g_width;				// viewport width / 100
int	  g_fieldNbr;		//Number of fields to render at current frame
int g_userInterface;
float g_plusMinus;			// If different than 0, increments or decrements field's property specified by g_userInterface
float3 g_translate;

float RESISTANCE;
float GRAVITY;

int g_gravity;
int g_emAtCenter;
int g_released;
int g_display;

int g_color		;
int g_tex		;
int g_toon		;
int g_diffSpec  ;
int g_bump		;
int g_DOF		;
int g_glow		;

float4x4 matPickInfos;		// Contains every needed informations about pick 3D position and orientation
/*  _mTemp._11 = vPickRayOrig.x;		_mTemp._12 = vPickRayDir.x;		_mTemp._13 = _lookAt.x;
	_mTemp._21 = vPickRayOrig.y;		_mTemp._22 = vPickRayDir.y;		_mTemp._23 = _lookAt.y;
	_mTemp._31 = vPickRayOrig.z;		_mTemp._32 = vPickRayDir.z;		_mTemp._33 = _lookAt.z;*/

static const float2 samples[12] = {
   -0.326212, -0.405805,
   -0.840144, -0.073580,
   -0.695914,  0.457137,
   -0.203345,  0.620716,
    0.962340, -0.194983,
    0.473434, -0.480026,
    0.519456,  0.767022,
    0.185461, -0.893124,
    0.507431,  0.064425,
    0.896420,  0.412458,
   -0.321940, -0.932615,
   -0.791559, -0.597705,
};
//--------------------------------------------------------------------------------------
//Textures declaration
//--------------------------------------------------------------------------------------
SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samEnv
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Mirror;
};

SamplerState sam1D
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
};

//Pixel shader resource
Texture2D txFields			: register (t0);
Texture2D txParticles		: register (t1);
Texture2D txParticlesParam	: register (t2);
Texture2D txScreen0			: register (t3);
Texture2D txRamp			: register (t4);
Texture2D txDiffuse			: register (t5);
Texture2D txBump			: register (t6);
Texture2D txHUD				: register (t7);
Texture2D txEnv				: register (t8);
//Geometry shader resource
Texture1D txFieldsPos		: register (t0);
Texture1D txFieldsGoal		: register (t1);
Texture1D txFieldsStrength  : register (t2);
Texture1D txFieldsTypeSize  : register (t3);

//--------------------------------------------------------------------------------------
//Common layouts
//--------------------------------------------------------------------------------------

//Common to ASSample and ASField.fx
struct VS_FIELDPARAMETERSOUTPUT
{
	float					index				: INDEX;			   //index
	uint					isSelected			: ISSELECTED;		   //if field is picked, set to true. False when released
	nointerpolation float	type				: FIELDTYPE;		   //0 = bouncer; 1 = directional; 2 = portal; 3 = particles emitter
	nointerpolation float	size				: SIZE;				   //size
	float3					offset				: OFFSET;			   //when picked, maintain distance between center of field and cursor
	nointerpolation float3  pos					: POSITION;			   //position in 3D space
	nointerpolation float3  goal				: GOAL;				   //Goal. Only for directional, portals and bouncers
	nointerpolation float3  strength			: FORCE;			   //strength of the field
	nointerpolation float4	position            : SV_Position;         //position of the field in texture sent to particles
	float4					color				: FIELDCOLOR;          //color of the field
	float4					dummy				: DUMMY;			   //optional parameter to make cool custom samples
};

//--------------------------------------------------------------------------------------
//Render states
//--------------------------------------------------------------------------------------
RasterizerState RasterizerParams
{
	SCISSORENABLE				= FALSE;
	FILLMODE					= SOLID;
	CULLMODE					= BACK;
	FRONTCOUNTERCLOCKWISE		= FALSE;
	DEPTHBIAS					= 0;
	DEPTHBIASCLAMP				= 0;
	SLOPESCALEDDEPTHBIAS		= 0;
	MULTISAMPLEENABLE			= FALSE;
	ANTIALIASEDLINEENABLE		= TRUE;
};

RasterizerState Rasterizer
{
	SCISSORENABLE				= FALSE;
	FILLMODE					= SOLID;
	CULLMODE					= BACK;
	FRONTCOUNTERCLOCKWISE		= TRUE;
	DEPTHBIAS					= 0;
	DEPTHBIASCLAMP				= 0;
	SLOPESCALEDDEPTHBIAS		= 0;
	MULTISAMPLEENABLE			= FALSE;
	ANTIALIASEDLINEENABLE		= TRUE;
};

DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
	DepthFunc = LESS_EQUAL;
};

DepthStencilState DisableDepth
{
    DepthEnable					= FALSE;
    DepthWriteMask				= ZERO;
};

BlendState NoBlend {
	BlendEnable[0]				= FALSE;
};

BlendState Blend 
{
	SrcBlend			   = SRC_ALPHA;
	DestBlend			   = INV_SRC_ALPHA;
	//SrcBlendAlpha			   = SRC_ALPHA;
	//DestBlendAlpha			   = INV_SRC_ALPHA;
	BlendEnable[0]			   = TRUE;
};




