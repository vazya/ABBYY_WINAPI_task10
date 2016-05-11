#define COUNT_API __declspec(dllexport)

#include "stdafx.h"
#include "wordcountDLL.h"
#include <wchar.h>

int WordsCount(const wchar_t* text) {
	size_t len = wcslen(text);
	bool isPrevSpace = false;
	bool isExistSymb = false;
	int count = 0;

	for (size_t i = 0; i < len; ++i) {
		if (text[i] == (wchar_t)' ') {

			if (isExistSymb && !isPrevSpace) {
				++count;
			}

			isPrevSpace = true;
		} else {
			isExistSymb = true;
			isPrevSpace = false;
		}
	}

	if (!isPrevSpace) {
		++count;
	}

	return count;
}