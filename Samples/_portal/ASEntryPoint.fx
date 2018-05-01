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
	InOut.size = -1;

	if(InOut.index == 0)// bouncer
	{
		InOut.size		= 20;
		InOut.pos		= float3(1,70,0);
		InOut.type		= 0;
		InOut.goal		= float3(1,0,0);
		InOut.strength	= float3(1.1,0,1);
	}
	else if(InOut.index < 13) //portals
	{
		float _temp		= (PI / ((FIELDSTOTAL / 4) - 1)) * (InOut.index - 1);
		InOut.size		= 25;
		InOut.pos		= float3( 200 * (cos(_temp) ),200 * -abs( sin(_temp) ),0 );
		InOut.type		= 2;
		InOut.goal		= float3(-InOut.pos.xy * 2,0);
	}
	
	else	//emitters
	{
		float _temp		= ((2 * PI) / (FIELDSTOTAL - 12)) * (InOut.index - 12);
		InOut.size		= 4;
		InOut.pos		= float3( 60 * (cos(_temp) ),60 * sin(_temp),0 );
		InOut.type		= 3;
	}

	return InOut;
}

// This function is called at each frame
// ! Size should never be set to 0
VS_FIELDPARAMETERSOUTPUT UpdateFieldsWithScenario(inout VS_FIELDPARAMETERSOUTPUT InOut)
{
	//This program simulates pendulum dynamic. Each frame, speed factor is applied to object's postion. Acceleration is determined by gravity/sec² and wire tansion
	float2 _rotater;
	float _length;
	float _cos, _sin, _speed;
	if(InOut.index == 0)
	{
		_length = length( InOut.pos );
		_length			= lerp( 0.01, _length, sign(_length) );
		InOut.dummy.xy	= (InOut.pos.xy / _length);								  //Dont't use hlsl normalize() to prevent division by zero
		_speed			= cross( float3(0, -1, 0),  float3(InOut.dummy.xy, 0) ).z;//Determine if speed is increased or decreased
		_speed			*=GRAVITY * g_deltaTime * 0.5;							  //Apply gravity
		_speed			+=InOut.dummy.w;										  //Sum result to old speed
		// Application of law of cosines. Results are unstable with small angles
		_cos			= clamp(( 2 * pow(_length, 2) - pow(_speed, 2)) / (2 * pow(_length, 2)), -1, 1);
		_sin			= sqrt( 1 - pow(_cos, 2) ) * -sign(_speed);				  //Find sin equivalent
		//Rotation from old pos to new pos
		_rotater.x		= InOut.pos.x * _cos - InOut.pos.y * _sin;				  
		_rotater.y		= InOut.pos.x * _sin + InOut.pos.y * _cos;
		InOut.pos.xy	= lerp( _rotater, InOut.pos.xy, saturate(InOut.isSelected) );
		//Object speed is increased or decreased thanks to gravity and acceleration mix
		InOut.dummy.w	= lerp( _speed, 0, saturate(InOut.isSelected) );
	}
	return InOut;
}
//__________________________________________________________________________________________________________________________________________________________

#include "../../Hlsl/ASFields.fx"