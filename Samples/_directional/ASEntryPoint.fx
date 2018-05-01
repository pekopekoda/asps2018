#include "../../Hlsl/ASShaderGlobal.fx"
#include "../../Hlsl/ASParticles.fx"

/*
__This shader can be modified to make custom fields samples ____________________________________________________________________________________________
__ Commented part below inform about parametrable field's properties and a few global variables declared outside. These can be used to make samples________
___________________________________________________________________________________________________________________________________________________________
__ Fields' structures______________________________________________________________________________________________________________________________________

	float					index				: INDEX;			   __ index
	nointerpolation float	type				: FIELDTYPE;		   __ 0 = bouncer; 1 = directional; 2 = portal; 3 = particles' emitter
	nointerpolation float	size				: SIZE;				   __ size
	nointerpolation float3  pos					: POSITION;			   __ position in 3D space
	nointerpolation float3  goal				: GOAL;				   __ Goal. Only for directional and portals
	nointerpolation float3  strength			: FORCE;			   __ strength of the field
	float4					color				: FIELDCOLOR;          __ color of the field
	float4					dummy				: DUMMY				   __ optional parameters (ex : velocity) can be stored in
	! This part is informative and these settings shouldn't be changed :
	uint					isSelected			: ISSELECTED;		   __ if field is picked, set to true. False when released
	float3					offset				: OFFSET;			   __ when picked, maintain distance between center of field and cursor
	nointerpolation float4	position            : SV_Position;         __ position of the field in texture sent to particles
___________________________________________________________________________________________________________________________________________________________

These variables are declared in ASShaderGlobal.fx and can be used for fields settings :

static const float PI = 3.14159265f
float g_time		: Time elapsed since program start
float g_deltaTime   : Time elapsed between each frame
bool  g_trigger		: Trigger for the particles emitter to emit
float g_rate		: Particles to emit per second
int   g_emAtCenter  : 1 Particles are emitted randomly around center of emitter, 0 Particles are emitted randomly in emitter area
int   g_fieldUpdate : Tells program nature of event at this frame :
	#define CLICK						 1  //Correspond to a trigger which means "right mouse button has just been pushed".
	#define CHANGE_TYPE					-1  // picked field's type may be changed
	#define CHANGE_SIZE					-2  // picked field's size may be changed
	#define CHANGE_CENTER_FORCE			-3  // picked field's center force may be changed
	#define CHANGE_EXTREMITY_FORCE		-4  // picked field's extremity force may be changed
	#define CHANGE_INTERPOLATION		-5  // picked field's interpolation may be changed
	#define RATE_PLUS					-6  // emission rate increased
	#define RATE_MINUS					-7  // emission rate decreased
	#define CHANGE_OBJECT_DIST_TO_CAM	-8  // change object distance from camera
	#define SWITCH_GRAVITY_ON			-9  // turn gravity on
	#define SWITCH_GRAVITY_OFF			-10 // turn gravity off
	#define EMISSION_TYPE				-11 // particles emitted at center of emitter / randomly in emitter area
	#define ADD							-12 // if deleted (size set below -1), field is reseted
	#define CLEAR_CAM					-13 // camera is reseted to its initial position
	#define SHOW_PANEL					-14 // show explanations

int g_released	    : If field isn't picked, set to 1, else to 0
____________________________________________________________________________________________________________________________________________________________
*/

// This function is called once at program start
VS_FIELDPARAMETERSOUTPUT GSFieldsStart(inout VS_FIELDPARAMETERSOUTPUT InOut)
{
	InOut.size			= -1;
	InOut.offset		= 0;
	float _angle = (PI/(FIELDSTOTAL)) * InOut.index * 2;
	float x;
	float3 _direction = float3 (cos (_angle), sin (_angle),0);
	if(InOut.index == 0)
	{
		InOut.pos		= float3(0.1,0,0);
		InOut.type		= EMITTER_TYPE;
		InOut.goal		= float3(0,-10,0);
		InOut.strength	= float3(1,0.1,1);
	}

	else if(InOut.index == 1)
	{
		InOut.pos		= float3(0.1,0,0);
		InOut.type		= FORCE_TYPE;
		InOut.goal		= float3(0,-70,0);
		InOut.strength	= float3(1,3,1.3);
	}

	else if(InOut.index == 21 || InOut.index == 30)
	{
		InOut.pos		= float3(50 * cos(InOut.index), 50 * sin(InOut.index),0);
		InOut.type		= EMITTER_TYPE;
		InOut.goal		= float3(0,-70,0);
		InOut.strength	= float3(1,3,1.3);
	}
	else
	{
		InOut.size		= 20;
		InOut.pos		= float3(50 * cos(InOut.index), 50 * sin(InOut.index),0);
		InOut.type		= FORCE_TYPE;
		InOut.goal		= float3(50 * cos(InOut.index + 1), 50 * sin(InOut.index + 1),0) * 0.1 - InOut.pos;
		InOut.strength	= float3(1,3,1);
	}
	return InOut;
}

// This function is called once at each frame
// ! Size should never be set to 0
VS_FIELDPARAMETERSOUTPUT UpdateFieldsWithScenario(inout VS_FIELDPARAMETERSOUTPUT InOut)
{
	float _length;
	if(InOut.index == 1 || InOut.index == 0)
	{
		_length = length(InOut.goal);
		InOut.goal = float3(_length * cos(-g_time), _length * sin(-g_time), 0);
		InOut.size = lerp(5, -1, sign(fmod(int(g_time / 10), 2)));//Every 10 seconds, fields are switched on/off
	}
	else if(InOut.index == 21 || InOut.index == 30)
	{
		_length = 50;
		InOut.size = lerp(-1, 5, sign(fmod(int(g_time / 10), 2)));//Every 10 seconds, fields are switched on/off
		InOut.pos = float3(_length * cos(g_time + InOut.index), _length * sin(g_time + InOut.index), 0);
	}
	return InOut;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../Hlsl/ASFields.fx"