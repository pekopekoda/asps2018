#include "../Hlsl/ASScreen.fx"
#define PARTICLE 1
//--------------------------------------------------------------------------------------
struct VS_ParticleIn
{
    float3	pos              : POSITION;         //position of the particle
    float3	vel              : NORMAL;           //velocity of the particle
    uint	type			 : TYPE;             //particle type
	float	lifespan         : TIMER;            //timer for the particle
	float   birth			 : BIRTH;
	float	mass			 : MASS;			 //particle mass
};
struct VS_ParticleInstanceIn
{
	float3	instancePos		 : POSITION1;
	float3	instanceNorm	 : NORMAL1;
	float2  texcoords		 : TEXCOORD1;
    float3	pos              : POSITION;        //position of the particle
    float3	vel              : NORMAL;			//velocity of the particle
	uint	type             : TYPE;            //particle type
    float	lifespan         : TIMER;           //timer for the particle
	float   birth			 : BIRTH;
	float	mass			 : MASS;			//particle mass	  
};
struct VS_OUTPUT
{
	float3 normal			: NORMAL1;
	float3 binormal			: BINORMAL;
	float3 tangent			: TANGENT;
	float3 position			: POSITION;
	float4 pos				: SV_Position;
	float2 texcoords		: TEXCOORD1;
	float  lifespanOnOne	: TIMER;
};
struct PS_OUTPUT
{
	float4 out1;
	float4 out2;
};
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Particle Functions
//--------------------------------------------------------------------------------------

float emissionRate()
{
	return g_rate * g_maxParticles;
}

float lifespanDecrement ()
{
	float j = 0;
	float4 _fieldTypeSize;
	/*for(float i = 0.005f; i < 1.0f; i += (1.0f / FIELDSTOTAL))
	{
		_fieldTypeSize = txFieldsTypeSize.SampleLevel(samLinear, i,0,0);
		if (EMITTER_TYPE == _fieldTypeSize.x && _fieldTypeSize.y > 0.0)
			j++;
	}*/
	return -(1.0/g_rate) * g_deltaTime;
}

float massUpdate  (VS_ParticleIn input)
{
	return input.mass * pow(input.lifespan / input.birth, 1);
}

float3 VelocityResistance (VS_ParticleIn input)
{
	return -input.vel * input.mass * RESISTANCE * g_deltaTime;
}

float3 Gravity (float mass)
{
	return float3 (0,-GRAVITY * (g_deltaTime)*mass, 0);
}

float3 PortalPhase(float4 _teleportFrom, float4 _teleportTo, float3 _offset, float3 _velocity)
{
	float _offsetLength			= length(_offset);
	float3 _normOffset			= -_teleportFrom.xyz / _offsetLength;
	float3 _deltaVelocity		= _velocity * g_deltaTime;
	float _lengthDeltaVelocity	= length(_deltaVelocity);
	float3 _normDeltaVelocity	= _deltaVelocity / _lengthDeltaVelocity;
	float _cos					= dot(_normOffset, _normDeltaVelocity);
	float3 _newPosition;

	if ((_lengthDeltaVelocity - _cos * _offsetLength) >= 0)
		_newPosition = _teleportTo.xyz - _offset;
	else
		_newPosition = _teleportFrom.xyz + _offset;
	return _newPosition;

	_normOffset   = normalize(_offset);
	float3 _nextPosition = normalize(_deltaVelocity - _teleportFrom.xyz);
	float _negCos = -dot(_normOffset, _nextPosition);
	if (_negCos > 0.0)
		_newPosition = _teleportTo.xyz + _deltaVelocity;
	else
		_newPosition = _teleportFrom.xyz + _offset;
	return _newPosition;
}

float3 FieldPhase(float3 _fieldToParticle, float _fieldSize, float4 _fieldGoal, float4 _fieldStrength, float _massUpdated)
{
	float3 _direction;
	float _posOnRange;
	_posOnRange = length(_fieldToParticle) / _fieldSize;
	_posOnRange *= _fieldStrength.y - _fieldStrength.x;	//Set to range force.z : force.y
	_posOnRange = _fieldStrength.x + abs(_posOnRange);
	_posOnRange = pow(abs(_posOnRange), _fieldStrength.z); // Submit to interpolation
	return normalize(_fieldGoal.xyz - _fieldToParticle) * clamp(-1000, 1000, _posOnRange) * _massUpdated;

}

void BouncerPhase(inout VS_ParticleIn output, float4 _fieldPos, float4 _fieldTypeSize, float4 _fieldGoal, float4 _fieldStrength,float3 _fieldToParticle)
{

	float3 _impactPoint;
	float3 _bounceVector;
	float3 _nextFramePosition;
	float3 _normOffset;
	float _overlap;
	float _lengthFieldToParticle;
	float _lengthNextFramePosition;
	
	_nextFramePosition			= _fieldToParticle.xyz + output.vel;
	_lengthNextFramePosition	= length(_nextFramePosition);
	_lengthFieldToParticle		= length(_fieldToParticle.xyz);

	//scaled down because the resultat is too violent
	_overlap					= (_fieldTypeSize.y - _lengthFieldToParticle) * 0.65;
	_normOffset					= _fieldToParticle.xyz / max(0.00001, _lengthFieldToParticle);
	_impactPoint				= _fieldPos + _normOffset * _fieldTypeSize.y;
	_bounceVector				= reflect(output.vel, _normOffset);

	output.pos					= _impactPoint;
	output.vel					= _bounceVector * (1.0 + _overlap) * _fieldStrength.x;
}

void AlterationPhase(VS_ParticleIn input, inout VS_ParticleIn output , int isValid)
{
	// Fields' phase
	output.vel = input.vel;
	float _lengthField, _lengthGoal;
	float4 _fieldPos, _fieldGoal, _fieldTypeSize, _fieldStrength;
	float3 _fieldToParticle, _goalToParticle;
	float _type, _size, _massUpdated;
	bool _alreadyPassedThroughAPortal = false;
	bool _particleInField;
	_massUpdated = massUpdate(input);
	for (float i = 0.005f; i < 1.0f; i += (1.0f / FIELDSTOTAL))
	{
		_fieldPos = txFieldsPos.SampleLevel(samLinear, i, 0, 0);
		_fieldTypeSize = txFieldsTypeSize.SampleLevel(samLinear, i, 0, 0);
		_fieldGoal = txFieldsGoal.SampleLevel(samLinear, i, 0, 0);
		_fieldStrength = txFieldsStrength.SampleLevel(samLinear, i, 0, 0);
		_fieldToParticle = input.pos - _fieldPos.xyz;
		_lengthField = length(input.pos - _fieldPos.xyz);			 //Gets distance between particle and field position
		//_posOnRange = _length / _fieldTypeSize.y;					 //length divided with field's size to set it to range (0:1)																
		_particleInField = _fieldTypeSize.y >= _lengthField;
		_type = _fieldTypeSize.x;
		_size = _fieldTypeSize.y;
		if (_type == PORTAL_TYPE &&!_alreadyPassedThroughAPortal)
		{
			_goalToParticle = _fieldToParticle - _fieldGoal.xyz;
			_lengthGoal = length(_goalToParticle);
			if (_particleInField)
			{
				output.pos = PortalPhase(_fieldPos, _fieldGoal + _fieldPos, _fieldToParticle, output.vel);
				_alreadyPassedThroughAPortal = true;
			}
			else if (_fieldTypeSize.y >= _lengthGoal)
			{
				output.pos = PortalPhase(_fieldGoal + _fieldPos, _fieldPos, _goalToParticle, output.vel);
				_alreadyPassedThroughAPortal = true;
			}
			else
				continue;
		}
		else if (!_particleInField)				//Test if particle is in the field. If not, _fIsValid is set to 0.
			continue;
		if (_type == FORCE_TYPE)
			output.vel += FieldPhase(_fieldToParticle, _size, _fieldGoal, _fieldStrength, _massUpdated);
		else if (_type == BOUNCER_TYPE)
			BouncerPhase(output, _fieldPos, _fieldTypeSize, _fieldGoal, _fieldStrength, _fieldToParticle);
	}
	output.vel += Gravity(output.mass);
	output.vel += VelocityResistance(input);
	output.pos += output.vel * g_deltaTime;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_ParticleIn VSDummy (VS_ParticleIn input)
{
	return input;
}
VS_ParticleInstanceIn VSDummyInst (VS_ParticleInstanceIn input)
{
	return input;
}
VS_OUTPUT VSRenderParticles (VS_ParticleInstanceIn input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	if (!input.type || input.lifespan <= 0.0) return output;
	matrix mwvp = mul(mul(matWorld, matView), matProj);
	// Returns the actual lifespan to range 1.0 : 0.0 --> birth time : death time
	output.lifespanOnOne = input.lifespan / input.birth;
	float _size = pow(output.lifespanOnOne, 0.5) * 3; //particles' size will decrease unlinearly with age
	float _length = length(input.vel);
	_length = max(0.0001, _length);
	float3 _vNorm = input.vel;
	_vNorm /= _length; //normal
					   

	float3 _vSide   = normalize(cross(float3(_vNorm.x, 0.0, _vNorm.z), _vNorm));
	float3 _vFront  = normalize(cross(_vSide, _vNorm));
	float3 _vUp     = normalize(cross(_vFront, _vSide));
	_vSide			= normalize(cross(_vUp, _vFront));

	float3x3 matRotationVel =
	{
		_vSide.x, _vUp.x, _vFront.x,
		_vSide.y, _vUp.y, _vFront.y,
		_vSide.z, _vUp.z, _vFront.z
	};

	//Rotate and scale model
	output.pos = float4(input.instancePos, 1);
	output.pos.xyz = mul(matRotationVel, input.instancePos);
	output.normal  = mul(matRotationVel, input.instanceNorm);
	output.binormal = -normalize(cross(output.normal, float3(0.0, 1.0, 0.0)));
	output.tangent = normalize(cross(output.binormal, output.normal));

	output.pos.xyz *= _size;

	//World space translate model
	output.pos.xyz += input.pos;
	output.position = input.instancePos + input.pos.xyz;
	//Submit model to world view projection
	output.pos = mul(output.pos, mwvp);

	//texcoords
	output.texcoords = input.texcoords;
	return output;
}

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------

//Initialize or reinitialize particles' parameters
void GSResetParticle (VS_ParticleIn output, inout PointStream<VS_ParticleIn> PtStream, float3 _emPos, float _size)
{
	float _randX    = cos(g_randX);
	float _randY    = cos(g_randY);
	float _randZ    = cos(g_randZ);
	matrix mwvp = mul(mul(matWorld, matView), matProj);
	output.pos.xy   = float2(_randX, _randY);
	output.pos.x	= _emPos.x + _randX * _size;
	output.pos.y	= _emPos.y + _randY * _size;
	output.pos.z	= _emPos.z + _randZ * _size;
	output.vel.x	= _randX;
	output.vel.y	= _randY;
	output.vel.z	= _randZ;
	output.vel	   *= GLOBALSPEED;
	output.type	    = PARTICLE;
	output.lifespan	= g_maxParticles; //excluding the emitter
	//Stocks total duration of life for this particle
	output.birth	= output.lifespan;
	output.mass		= 10;

	mul(output.vel, mwvp);
	PtStream.Append(output);
}

//limited number of call if emission priority mode is off
void GSEmitParticle (VS_ParticleIn output, inout PointStream<VS_ParticleIn> PtStream)
{
	float4 _fieldTypeSize, _fieldPos;
	output.pos = 0;
	output.vel = 0;
	PtStream.Append(output);

	for(float i = 0.005f; i < 1.0f; i += (1.0f / FIELDSTOTAL))
	{
		_fieldTypeSize = txFieldsTypeSize.SampleLevel(samLinear, i,0,0);
		if((EMITTER_TYPE == _fieldTypeSize.x) && _fieldTypeSize.y > 0)
		{
			_fieldPos = txFieldsPos.SampleLevel(samLinear, i,0,0);
			GSResetParticle(output, PtStream, _fieldPos.xyz, _fieldTypeSize.y);
		}
	}
}

[maxvertexcount(100)]
void GSUpdateParticles (point VS_ParticleIn input[1], inout PointStream<VS_ParticleIn> PtStream)
{
	VS_ParticleIn output = (VS_ParticleIn) input[0];
	int _iIsParticle = sign(saturate(input[0].type));

	if (1 - _iIsParticle)
	{
		/*if (g_time == 0.0)
		{
			for(unsigned int i = 0; i < g_maxParticles; i++)
				GSEmitParticle(output, PtStream);
		}*/
		if (output.lifespan <= 0.0f)
		{
			output.lifespan = 1.0/g_rate;
			GSEmitParticle(output, PtStream);
			return;
		}
		else
			output.lifespan -= g_deltaTime;
	}
	else
	{
		//Particles' parameters update
		output.lifespan -= g_rate * g_deltaTime;;
		AlterationPhase(input[0], output, _iIsParticle); // Alterations phase
	}
	PtStream.Append(output);
}
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
PS_OUTPUT PSRenderParticles( VS_OUTPUT input) : SV_Target
{
	PS_OUTPUT output = (PS_OUTPUT) 0;
	matrix mwvp = mul(mul(matWorld, matView), matProj);
	float3 _l = mul(mwvp, LightDirection);
	float4 color0, color1, _ramp, _tex, _txRamp;
	float _diff, _spec, _lerpContour, _brightnessClamp, _diffToon, _lerpGlow, _lerpDOF;
	float _invGlow   = 1 - g_glow;
	float3 _camPos	 = float3(matPickInfos._m00, matPickInfos._m10, matPickInfos._m20);
	float3 _camDir	 = float3(matPickInfos._m02, matPickInfos._m12, matPickInfos._m22);
	float3 _normalCamSpace;
	float4 color2	 = txBump.Sample(samLinear, -input.texcoords) * 2 - 1;
	color2.rgb = input.normal + (color2.r * input.tangent + color2.g * input.binormal);
	float _length	= length(color2.rgb);
	color2.rgb 	   /= _length;
	float3 _vNorm 	= lerp(input.normal, color2.rgb, g_bump);
	_normalCamSpace = 1 - saturate(dot(normalize(_camPos - input.position), _vNorm));
	//Color////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//1D texture which texcoord is particle's lifespan
	_txRamp			= txRamp.Sample( samLinear, input.lifespanOnOne);
	_ramp			= lerp(float4(g_tex, g_tex, g_tex, 1), _txRamp, g_color);
	_tex			= txDiffuse.Sample( samLinear, -input.texcoords);
	_tex			= lerp(float4(g_color, g_color, g_color, 1), _tex, g_tex);
	//diffuse//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	_diff			= saturate(dot (_vNorm, -LightDirection) + 0.2);
	_diff			= lerp(1, _diff, g_diffSpec * _invGlow);
	//Specular/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	_length = length(input.pos.xyz);
	_spec			= saturate(dot(reflect(input.pos.xyz / _length, _vNorm), LightDirection));
	_spec			= pow(_spec, 16);
	_spec		   *= g_diffSpec * _invGlow;
	//Contour//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	float3 _temp	= sign(_normalCamSpace - 0.3);
	_lerpContour	= saturate(_temp.x / 3 + _temp.y / 3 + _temp.z / 3);
	_lerpContour   *= g_toon;
	//Toon/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	_brightnessClamp= 0.4; //If color brigthness is lower than _brigthnessClamp, clamp it to 0
	_brightnessClamp= saturate(sign(_tex.r - _brightnessClamp)) * saturate(sign(_tex.g - _brightnessClamp)) * saturate(sign( _tex.b * _brightnessClamp));
	_tex			= lerp(_tex, _tex * _brightnessClamp, g_toon);
	_diffToon		= _diff;
	_diffToon		= lerp(0, _diffToon, saturate(sign(_diffToon - 0.3))); //Low diffuse clamped to 0
	_diffToon		= lerp(_diffToon, 1 , saturate(sign(_diffToon - 0.7)));//High diffuse clamped to 1
	_diffToon		= lerp(_diffToon, 0.5 , sign(_diffToon) * sign(1 - _diffToon));//Median diffuse clamped to 0.5
	_diff			= lerp(_diff, _diffToon, g_toon);
	_spec			= lerp(_spec, saturate(sign(_spec - 0.3)), g_toon);
	//Glow/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	_lerpGlow		= 1 - length(_normalCamSpace) * 0.2;
	_lerpGlow	   *= lerp(1, 0.1, _lerpContour);
	_lerpGlow	   *= g_glow;
	//Depth of field///////////////////////////////////////////////////////////////////////////////////////////////////////
	_lerpDOF		= 1 - saturate(length(input.position - (_camPos + FocusDist * _camDir)) * FocusRange);
	_lerpDOF		= pow(_lerpDOF, 2);
	_lerpDOF	   *= g_DOF;
	
	//Add color
	color0 = _tex * _ramp;
	//Add contour
	color0.rgb = lerp(color0.rgb, lerp(_txRamp.rgb, float4(0.0, 0.0, 0.0, 1.0), g_color), _lerpContour);
	//Add diffuse
	color0.xyz *= _diff;
	//Add specular
	color0.xyz += _spec;
	//Add glow
	color1 = _lerpGlow;
	//Add depth of field
	color1 += _lerpDOF;
	//Adjust opacity with particle's lifespan
	//color0.a= pow(input.lifespanOnOne, 0.3)* lerp(_tex.a, 1, _lerpContour);
	color0.a = lerp(_tex.a, 1, _lerpContour);
	//Output final color
	output.out1 = color0;
	//Output glow and DOF
	output.out2 = color1;
	return output;
}

//--------------------------------------------------------------------------------------
technique10 RenderParticles
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VSRenderParticles() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSRenderParticles() ) );
		SetDepthStencilState( EnableDepth, 0 );
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState(RasterizerParams);
    }
}
GeometryShader gsStreamOutPar = ConstructGSWithSO( CompileShader( gs_4_0, GSUpdateParticles() ), "POSITION.xyz; NORMAL.xyz; TYPE.x; TIMER.x; BIRTH.x; MASS.x;" );
technique10 UpdateParticles
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, VSDummy() ) );
        SetGeometryShader( gsStreamOutPar );
        SetPixelShader( NULL );
	}
}

