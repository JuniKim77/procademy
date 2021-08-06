#pragma once

#define dfACTION_MOVE_LL (0)
#define dfACTION_MOVE_LU (1)
#define dfACTION_MOVE_UU (2)
#define dfACTION_MOVE_RU (3)
#define dfACTION_MOVE_RR (4)
#define dfACTION_MOVE_RD (5)
#define dfACTION_MOVE_DD (6)
#define dfACTION_MOVE_LD (7)

#define dfACTION_ATTACK1 (8)
#define dfACTION_ATTACK2 (9)
#define dfACTION_ATTACK3 (10)

#define dfAction_STAND (11)
#define dfAction_NONE (12)

#define dfDELAY_STAND	(5)
#define dfDELAY_MOVE	(4)
#define dfDELAY_ATTACK1	(3)
#define dfDELAY_ATTACK2	(4)
#define dfDELAY_ATTACK3	(4)
#define dfDELAY_EFFECT	(3)

//-----------------------------------------------------------------
// 화면 이동 범위.
//-----------------------------------------------------------------
#define dfRANGE_MOVE_TOP	0
#define dfRANGE_MOVE_LEFT	0
#define dfRANGE_MOVE_RIGHT	6400
#define dfRANGE_MOVE_BOTTOM	6400
#define dfSECTOR_SIZE (128)
#define dfSECTOR_MAX_X (dfRANGE_MOVE_RIGHT / dfSECTOR_SIZE)
#define dfSECTOR_MAX_Y (dfRANGE_MOVE_BOTTOM / dfSECTOR_SIZE)

//---------------------------------------------------------------
// 공격범위.
//---------------------------------------------------------------
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20


//---------------------------------------------------------------
// 공격 데미지.
//---------------------------------------------------------------
#define dfATTACK1_DAMAGE		1
#define dfATTACK2_DAMAGE		2
#define dfATTACK3_DAMAGE		3


//-----------------------------------------------------------------
// 캐릭터 이동 속도   // 25fps 기준 이동속도
//-----------------------------------------------------------------
#define dfSPEED_PLAYER_X	3	// 3   50fps
#define dfSPEED_PLAYER_Y	2	// 2   50fps


//-----------------------------------------------------------------
// 이동 오류체크 범위
//-----------------------------------------------------------------
#define dfERROR_RANGE		50

#define dfSCREEN_WIDTH 640
#define dfSCREEN_HEIGHT 480
