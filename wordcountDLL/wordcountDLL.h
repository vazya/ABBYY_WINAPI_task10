#include <Windows.h>

#ifdef COUNT_API
#define COUNT_API __declspec(dllexport) 
#else
#define COUNT_API __declspec(dllimport) 
#endif


////////////////////////////////////////////////////////////////////////////
// ����� ������������ ��� ��������� ������ � �������������� (�������)
////////////////////////////////////////////////////////////////////////////
// ����� ������������ ��������� �������������� �������
extern "C" COUNT_API int WordsCount(const wchar_t* text);
////////////////////////////// ����� ����� /////////////////////////////////