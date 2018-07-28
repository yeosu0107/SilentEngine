#ifndef _UICBUFFER_SHADER
#define _UICBUFFER_SHADER

#ifndef NUM_MAXIMUMNUMBER_LENGTH
    #define NUM_MAXIMUMNUMBER_LENGTH 2
#endif

cbuffer cbUIInfo : register(b0)
{
    float2  gxmf2ScreenPos;
    float2  gxmf2ScreenSize;

    uint2   gnNumSprite;
    uint2   gnNowSprite;

    uint2   gnSize;
    uint    gnTexType;
    float   gfData;

    float  gfData2;
    float2 gfScale;
    float  gfAlpha;
};

cbuffer cbNumberData : register(b1)
{
    uint    gNumberType ; // 0 : float, 1 : float 분수 , 2 : float 퍼센트 , 3 : int , 4 : int 분수 , 5 : int 퍼센트
    uint    gNumberMaximumLength; // 문자 최대 길이
    float   gOriginNumber1;
    float   gOriginNumber2;
    uint4    gfNumber[NUM_MAXIMUMNUMBER_LENGTH];
}

#endif