//
// 2016-08-01, jjuiddong
// ������ ��ϵ� �����, �Է����� ���� ������ ���ؼ� �˻��Ѵ�.
// ������ �����ϸ鼭 �˻��Ѵ�.
//
#pragma once


namespace tess
{
	class cDictionary;

	class cErrCorrectCompare
	{
	public:
		struct sInfo
		{
			int tot;
			int hit;
			int err;
			int rep;
			int flags;
		};

		cErrCorrectCompare();
		virtual ~cErrCorrectCompare();
		bool Compare(const cDictionary &dict, char *src, char *dict_word);


	protected:
		bool CompareSub(const cDictionary &dict, char *src, char *dict_word, OUT sInfo &info);
		bool CompareSub2Type(const cDictionary &dict, char *src, char *dict_word, OUT sInfo &info);


	public:
		sInfo m_result;
	};
}

