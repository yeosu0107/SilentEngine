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
    uint    gNumberType ; // 0 : float, 1 : float �м� , 2 : float �ۼ�Ʈ , 3 : int , 4 : int �м� , 5 : int �ۼ�Ʈ
    uint    gNumberMaximumLength; // ���� �ִ� ����
    float   gOriginNumber1;
    float   gOriginNumber2;
    uint4    gfNumber[NUM_MAXIMUMNUMBER_LENGTH];
}

#endif