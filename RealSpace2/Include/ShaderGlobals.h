#pragma once

namespace ShaderConstant
{
enum
{
	WorldViewProjection = 0,
	WorldView = 4,
	Projection = 8,
	LightViewProjection = 12,
	Material = 16,
	LightDiffuse,
	LightAmbient,
	CosTheta,
	ShadowMapSize,
	AttenuationValues,
	World,
	LightPos0 = 40,
	LightDir0 = 48,
};
}

namespace ShaderSampler
{
enum
{
	Scene,
	Lightmap,
	Diffuse = 3,
	Shadow0 = 0,
};
}

namespace DeferredShaderConstant
{
enum
{
	WorldViewProjection = 0,
	WorldView = 4,
	World = 8,
	Diffuse = 12,
	Ambient,
	Spec,
	SpecLevel,
	Glossiness,
	Opacity,
	Near,
	Far,
};
}

inline void SetVSVector4(UINT Register, const v4& Vector) {
	RGetDevice()->SetVertexShaderConstantF(Register, static_cast<const float*>(Vector), 1); }
inline void SetVSVector3(UINT Register, const v3& Vector) {
	RGetDevice()->SetVertexShaderConstantF(Register, static_cast<const float*>(v4{ EXPAND_VECTOR(Vector), 1 }), 1); }
inline void SetVSMatrix(UINT Register, const rmatrix& Matrix) {
	RGetDevice()->SetVertexShaderConstantF(Register, static_cast<const float*>(Matrix), 4); }
inline void SetVSFloat(UINT Register, float Value) {
	RGetDevice()->SetVertexShaderConstantF(Register, static_cast<const float*>(v4{ Value, 0, 0, 0 }), 1); }

inline void SetPSVector4(UINT Register, const v4& Vector) {
	RGetDevice()->SetPixelShaderConstantF(Register, static_cast<const float*>(Vector), 1); }
inline void SetPSVector3(UINT Register, const v3& Vector) {
	RGetDevice()->SetPixelShaderConstantF(Register, static_cast<const float*>(v4{ EXPAND_VECTOR(Vector), 1 }), 1); }
inline void SetPSMatrix(UINT Register, const rmatrix& Matrix) {
	RGetDevice()->SetPixelShaderConstantF(Register, static_cast<const float*>(Matrix), 4); }
inline void SetPSFloat(UINT Register, float Value) {
	RGetDevice()->SetPixelShaderConstantF(Register, static_cast<const float*>(v4{ Value, 0, 0, 0 }), 1); }