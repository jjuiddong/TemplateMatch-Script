
#include "stdafx.h"
#include "errorcorrectcompare.h"
#include "dictionary.h"


using namespace tess;

cErrCorrectCompare::cErrCorrectCompare()
{
	ZeroMemory(&m_result, sizeof(m_result));
}

cErrCorrectCompare::~cErrCorrectCompare()
{
}


bool cErrCorrectCompare::Compare(const cDictionary &dict, char *src, char *dict_word)
{
	CompareSub2Type(dict, src, dict_word, m_result);
	return true;
}


bool cErrCorrectCompare::CompareSub2Type(const cDictionary &dict, char *src, char *dict_word, OUT sInfo &info)
{
	sInfo info1;
	ZeroMemory(&info1, sizeof(info1));
	CompareSub(dict, src, dict_word, info1); // ���� �ܾ �ٲ㰡�鼭 ��

	sInfo info2;
	ZeroMemory(&info2, sizeof(info2));
	info2.flags = 1;
	CompareSub(dict, src, dict_word, info2); // ���� �ܾ �ٲ㰡�鼭 ��

	// �� ����� ����� �����ϰ� �����Ѵ�.
	info = (info1.tot < info2.tot) ? info2 : info1;
	return true;
}


bool cErrCorrectCompare::CompareSub(const cDictionary &dict, char *src, char *dict_word, OUT sInfo &info)
{
	const int MAX_ERROR = 15;
	const int MAX_WORD_ERROR = 3;

	sInfo maxFitness;
	ZeroMemory(&maxFitness, sizeof(maxFitness));
	maxFitness.tot = -1000;

	int nextCount = 0;
	char *storePtr = NULL;

	while (*src)
	{
		while (*dict_word)
		{
			if ((*src < 0) || (*dict_word < 0)) // �����ڵ� �۾� �ʿ�.
				break;
			if ((*src == '\n') || (*src == '\r')) // ���๮�ڸ�, ����.
				break;

			if (*src == *dict_word)
			{
				nextCount = 0;
				++info.hit;
				++src;
				++dict_word;
			}
			else if (
				(0 != dict.m_ambiguousTable[*src]) &&
				(dict.m_ambiguousTable[ *src] == dict.m_ambiguousTable[ *dict_word]))
			{
				// ��ȣ�� �������� ������ ��, ������ ������ ����
				nextCount = 0;
				++info.rep;
				++src;
				++dict_word;
			}
			else
			{
				++info.err;

				if (0 == nextCount)
					storePtr = (info.flags==0)? src : dict_word;

				if (info.flags ==0)
					++src; // �ν��� �ܾ ������Ų��. ���� �ν��� �߸��Ǽ�, ���� �ܾ ���� ������ ��찡 ����.
				else
					++dict_word; // ���� �ܾ� �ε����� ������Ų��. �ν��� ���忡�� �ܾ ������ ��찡 ���� ����.

				++nextCount;

				// ���������� ��Ī�� �ȵȰ��, src or dict_word �� ������Ų��.
				// ��Ī�� �ȵ� ������ ��������� �ٽ� �˻��Ѵ�.
				if (nextCount > MAX_WORD_ERROR)
				{
					info.err -= (nextCount - 1);

					sInfo tmp = info;
					CompareSub2Type(dict, src, dict_word, tmp);
					if (tmp.tot > maxFitness.tot)
						maxFitness = tmp;

					if (0 == info.flags)
					{
						src = storePtr + 1;
						++dict_word;
					}
					else
					{
						dict_word = storePtr + 1;
						++src;
					}

					nextCount = 0;
				}
			}

			if (!*src || !*dict_word)
				break;
			if (info.err >= MAX_ERROR)
				break;
		}

		if (!*src || !*dict_word)
			break;
		if ((*src == '\n') || (*src == '\r')) // ���๮�ڸ�, ����.
			break;
		if (info.err >= MAX_ERROR)
			break;
		if ((*src < 0) || (*dict_word < 0)) // �����ڵ� �۾� �ʿ�.
			break;
	}

	// ���� ������ ���ڸ�ŭ ���� �߻�
	while (*dict_word++)
		++info.err;
	while (*src++)
		++info.err;

	info.tot = info.hit - info.err;

	if (maxFitness.tot > info.tot)
		info = maxFitness;

	return false;
}
