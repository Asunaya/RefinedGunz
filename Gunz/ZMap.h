#ifndef _ZMAP_H
#define _ZMAP_H

#include "ZFilePath.h"
// 맵과 관련된 것들은 이 곳에 넣읍시다.


class MWidget;

template<size_t size>
void ZGetCurrMapPath(char(&outPath)[size]) {
	ZGetCurrMapPath(outPath, size);
}
void ZGetCurrMapPath(char* outPath, int maxlen);

// 해당위젯(콤보박스)에 게임가능한 맵을 추가한다.
bool InitMaps(MWidget *pWidget);


#endif