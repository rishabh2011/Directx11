//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Buffer Definitions: 
//
// cbuffer cbperobject
// {
//
//   float4x4 gWorldViewProj;           // Offset:    0 Size:    64
//   float4x4 gWorldInvTranspose;       // Offset:   64 Size:    64
//   float4x4 gWorld;                   // Offset:  128 Size:    64
//   
//   struct Material
//   {
//       
//       float4 ambientColor;           // Offset:  192
//       float4 diffuseColor;           // Offset:  208
//       float4 specColor;              // Offset:  224
//
//   } material;                        // Offset:  192 Size:    48 [unused]
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- -------------- ------
// cbperobject                       cbuffer      NA          NA            cb0      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// POSITION                 0   xyz         0     NONE   float   xyz 
// NORMAL                   0   xyz         1     NONE   float   xyz 
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_POSITION              0   xyzw        0      POS   float   xyzw
// POSITION                 0   xyz         1     NONE   float   xyz 
// NORMAL                   0   xyz         2     NONE   float   xyz 
//
vs_5_0
dcl_globalFlags refactoringAllowed | skipOptimization
dcl_constantbuffer CB0[11], immediateIndexed
dcl_input v0.xyz
dcl_input v1.xyz
dcl_output_siv o0.xyzw, position
dcl_output o1.xyz
dcl_output o2.xyz
dcl_temps 4
//
// Initial variable locations:
//   v0.x <- vin.PosL.x; v0.y <- vin.PosL.y; v0.z <- vin.PosL.z; 
//   v1.x <- vin.NormalL.x; v1.y <- vin.NormalL.y; v1.z <- vin.NormalL.z; 
//   o2.x <- <vertexShader return value>.NormalW.x; o2.y <- <vertexShader return value>.NormalW.y; o2.z <- <vertexShader return value>.NormalW.z; 
//   o1.x <- <vertexShader return value>.PosW.x; o1.y <- <vertexShader return value>.PosW.y; o1.z <- <vertexShader return value>.PosW.z; 
//   o0.x <- <vertexShader return value>.PosH.x; o0.y <- <vertexShader return value>.PosH.y; o0.z <- <vertexShader return value>.PosH.z; o0.w <- <vertexShader return value>.PosH.w
//
#line 84 "D:\Programming\D3D11\D3D11\Box.hlsl"
mov r0.xyz, v0.xyzx
mov r0.w, l(1.000000)
dp4 r1.x, r0.xyzw, cb0[0].xyzw  // r1.x <- vout.PosH.x
dp4 r1.y, r0.xyzw, cb0[1].xyzw  // r1.y <- vout.PosH.y
dp4 r1.z, r0.xyzw, cb0[2].xyzw  // r1.z <- vout.PosH.z
dp4 r1.w, r0.xyzw, cb0[3].xyzw  // r1.w <- vout.PosH.w

#line 85
dp3 r2.x, v1.xyzx, cb0[4].xyzx  // r2.x <- vout.NormalW.x
dp3 r2.y, v1.xyzx, cb0[5].xyzx  // r2.y <- vout.NormalW.y
dp3 r2.z, v1.xyzx, cb0[6].xyzx  // r2.z <- vout.NormalW.z

#line 86
dp4 r3.x, r0.xyzw, cb0[8].xyzw  // r3.x <- vout.PosW.x
dp4 r3.y, r0.xyzw, cb0[9].xyzw  // r3.y <- vout.PosW.y
dp4 r3.z, r0.xyzw, cb0[10].xyzw  // r3.z <- vout.PosW.z

#line 88
mov o0.xyzw, r1.xyzw
mov o1.xyz, r3.xyzx
mov o2.xyz, r2.xyzx
ret 
// Approximately 16 instruction slots used
