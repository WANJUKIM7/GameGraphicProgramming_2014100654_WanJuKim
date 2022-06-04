/*
2022.3.22(화)

1. 포인터에 비해 Comptr의 장점은? → 마이크로소프트에서 Comptr를 장려 ? >> 디스코드에 올렸다는데.
2. Reset을 해줘야 하나? → 명시적으로 하는게 좋으니 Reset 해줘야 한다.모든 Comptr에 Reset 함수가 존재 한다는데? → 진짜 있네.
3. ??

저번에 Renderer 한 거 문제가 있었다.
변수들을 전역변수로 선언했었다는 거. 코드의 재사용성, 유용성이 좋지 않다. 하드코딩 되었다.
그리고 만들어야 할 변수들이 몇 개인데, 그거 다 전역으로 할거냐?
이번 실습은, 저번 실습을 객체 지향으로 만들 것이다.
WindowProc은 static이어야 해. 왜? 함수 포인터를 전달해주니까. DerivedType 템플릿으로 만들 거야. 이해안돼? 외워.

struct StateInfo{
};

LPARAM lParam을 형변환 어쩌구 해야지 StateInfo에서 받을 수 있다?
이태리체 - 순수가상함수
노란색 다이아몬드 - protected
초록색 다이아몬드 - public
밑줄 - static

이번엔 CleanupDevice가 없다. 왤까? 컴포인터라서 알아서 릴리즈가 된다.
*/

/*
2022.3.29(화)

객체 지향을 공부하는 방법은 여러 번 코드를 갈아 엎는 과정을 거치는 것이다.
D3D11_BUFFER_DESC bd = {}; 이건 초기화가 아니라 할당이다. 맨 첨에는 초기화를 하는 게 좋나봐.
m_의 의미가 멤버 변수구나;
출력 창을 매우 크게 해놓고 사용하네. 근데 뭐 이렇게 창이 많아?
콜스택이 함수 실행 순서를 보여주는구나.

본 수업?
삼각형 만들게. Vertex 하나 만들어서 Position만 넣을게.
삼각형 그릴거니까 점 3개 배열로 만듦.
D3D11_BUFFER_DESC bd = {};
채우고, subResourceData 하고...
InDexBuffer도 만들고,
등등
*/

/*
4.5(화)
Camel, Pascal, 헝가리언 다 아냐?
헝가리언 표기법 중에 DWORD dwError 이런 것도 있네.
psz는 p는 포인터, sz는 string zero뜻함. const char* 같은 거 할 때 하나보네.
*/

/*
4.12(화)
QUESTION : cpp들을 library 안에다가 하는 이유는 무엇인가?
4.13(수)
QUESTION : 왜 이렇게 객체가 많은 건가? window, renderer, game 등등... 객체가 많을 수록 좋은 건가? 이게 객체화 프로그래밍?
QUESITON : 추가 종속성, 링커 이런 거 다 아냐?
4.19(화)
QUESTION : return E_NOTIMPL; 이것보다 return MessageBox 쓰는 게 낫지 않나? 전자는 하나하나 다 살펴봐야잖아.
4.29(금)
QUESTION : _In_을 굳이 쓰는 이유는? const로도 충분하지 않나? _Inout_이것도 마찬가지. & 쓰면 충분히 알 수 있지 않나?

4.30(토)
TIP : LNK2019 에러 떴는데 원인이 뭐였는 지 알아?
Library 속성 - 라이브러리 관리자 - 일반 - 출력 파일 맨 뒤에 ';' 세미콜론 붙어있어서 Lib가 업데이트가 안되고 있었다;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
와 어케 알았지?

5.30(월)
QUESTION : push 할 때 Model의 obj 파일 및 Assimp의 Binary 및 Library 안 올라가는 거 정상임? 추가하기 귀찮을 것 같은데.
*/

