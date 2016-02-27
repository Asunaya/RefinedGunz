#ifndef RFRAMEWORK_H
#define RFRAMEWORK_H

_NAMESPACE_REALSPACE2_BEGIN

// 렌더링 함수를 호출하기 위해 외부로 노출
void RFrame_Render();

// 파티클 / 기본폰트 등의 리소소는 framework 에 전역으로 있다.
// 이런 리소스를 invalidate/restore 해줘야한다.

void RFrame_Invalidate();
void RFrame_Restore();

void RFrame_ToggleFullScreen();

_NAMESPACE_REALSPACE2_END

bool IsToolTipEnable();

#endif