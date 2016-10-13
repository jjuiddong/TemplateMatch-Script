
#include "stdafx.h"
#include "dictionary2.h"
#include "scanner.h"

using namespace tess;

cDictionary2::cDictionary2()
	: m_ambigId(1)
	, col(256), prevCol(256)
{
	memset(m_ambiguousTable, 1, sizeof(m_ambiguousTable));
}

cDictionary2::~cDictionary2()
{
}


// ���� ��ũ��Ʈ�� �о, �ܾ� ������ �����.
bool cDictionary2::Init(const string &fileName)
{
	using namespace std;

	Clear();

	m_sentences.reserve(MAX_WORD);
	m_words.reserve(MAX_WORD);

	ifstream ifs(fileName);
	if (!ifs.is_open())
		return false;
	
	int state = 0; // 0:word, 1:ambiguous
	string line;
	while (getline(ifs, line))
	{
		trim(line);
		if (line.empty())
			continue;

		if (line == "word:")
		{
			state = 0;
			continue;
		}
		else if (line == "ambiguous:")
		{
			state = 1;
			continue;
		}

		if (0 == state ) // word list command
		{
			// ���� ���� ---> sentence { output sentence }
			string outputStr;
			{
				const int pos1 = line.find('{');
				const int pos2 = line.find('}');
				if ((pos1 != string::npos) && (pos2 != string::npos))
				{
					outputStr = line.substr(pos1+1, pos2 - pos1 - 1);
					trim(outputStr);
				}
				else
				{
					outputStr = line;
				}
			}

			string srcLine = (outputStr.empty())? line : line.substr(0, line.find('{'));
			trim(srcLine);
			const string lower = lowerCase(srcLine);

			if (m_sentenceLookUp.end() == m_sentenceLookUp.find(lower))
			{// ������ �߰�
				m_sentences.push_back({ srcLine, lower, outputStr});
				m_sentenceLookUp[lower] = m_sentences.size() - 1; // word index
			}
			else
			{ // ������ �̹� ������ �Ѿ
				//dbg::ErrLog("error already exist word = [%s]\n", line.c_str());
			}

			// ������ ������ �������� ���� �ܾ�� ������ ����.
			const int sentenceId = m_sentences.size() - 1;
			vector<string> words;
			tokenizer(lower, " ", "", words);
			for each (auto word in words)
			{
				trim(word);
				if (word.length() < MIN_WORD_LEN)
					continue; // �ʹ� ª�� �ܾ�� �����Ѵ�.

				int wordId = 0;
				const auto it = m_wordLookup.find(word);
				if (it == m_wordLookup.end())
				{
					m_words.push_back(word);
					wordId = m_words.size() - 1;
					m_wordLookup[word] = m_words.size()-1;
				}
				else
				{
					wordId = it->second;
				}

				if (wordId < MAX_WORD)
					m_sentenceWordSet[wordId].insert(sentenceId);
			}
		}
		else if (1 == state) // ambiguous command
		{
			// format = c1 - c2 - c3
			string tmp = line;
			const string lower = lowerCase(tmp);

			vector<string> words;
			tokenizer(lower, "-", "", words);
			for each (auto word in words)
			{
				trim(word);
				if (word.empty())
					continue;
				if (word.length() > 1) // allow only one character
					continue;
				m_ambiguousTable[word[0]] = m_ambigId; // ���� ID�� �ο��Ѵ�.
			}
			++m_ambigId;
		}

	}

	return true;
}


// word �� ������ �ִ� �ܾ�θ� �����Ǿ� �ִٸ�, true�� �����Ѵ�.
// ��ġ�ϴ� �ܾ out�� ������ �����Ѵ�.
// ErrorCorrectionSearch() ���� ������.
string cDictionary2::FastSearch(const string &sentence, OUT vector<string> &out)
{
	cScanner scanner(sentence);// �ʿ���� ���� ����
	if (scanner.IsEmpty())
		return "";

	stringstream ss;
	set<int> sentenceSet; // words �ܾ ���Ե� ���� id�� ����

	// ������ �ܾ� ������ ���Ѵ�.
	vector<string> words;
	tokenizer(scanner.m_str, " ", "", words);
	for each (auto word in words)
	{
		trim(word);
		if (word.length() < MIN_WORD_LEN)
			continue; // �ʹ� ª�� �ܾ�� �����Ѵ�.
		
		auto it = m_wordLookup.find(word);
		if (it == m_wordLookup.end())
			continue;

		ss << (out.empty()? word : string(" ")+ word);
		out.push_back(word); // add maching word

		const int wordId = it->second;
		if (wordId >= MAX_WORD)
			continue;

		if (sentenceSet.empty())
		{
			sentenceSet = m_sentenceWordSet[wordId];
		}
		else
		{
			// �� �ܾ ���Ե� ����鳢���� �������� ���Ѵ�.
			vector<int> tmp;
			std::set_intersection(sentenceSet.begin(), sentenceSet.end(),
				m_sentenceWordSet[wordId].begin(), m_sentenceWordSet[wordId].end(),
				std::back_inserter(tmp));
				
			sentenceSet.clear();
			for each (auto id in tmp)
				sentenceSet.insert(id);
		}
	}
	
	if (sentenceSet.size() == 1)
	{
		const int idx = *sentenceSet.begin();
		return m_sentences[idx].output.empty() ? m_sentences[idx].src : m_sentences[idx].output;
	}

	return "";
}


void cDictionary2::Clear()
{
	m_sentences.clear();
	m_words.clear();
	m_sentenceLookUp.clear();

	for (uint i = 0; i < MAX_WORD; ++i)
		m_sentenceWordSet[i].clear();
}


// ������ �����ϸ鼭, ������ �������� ã�´�.
// FastSearch() ���� ������.
string cDictionary2::ErrorCorrectionSearch(const string &sentence, OUT float &maxFitness)
{
	maxFitness = -FLT_MAX;

	cScanner scanner(sentence);// �ʿ���� ���� ����
	if (scanner.IsEmpty())
		return "";

 	string src = scanner.m_str;

	int sentenceId = -1;
	int minDistance = 1000000;
	for (uint i = 0; i < m_sentences.size(); ++i)
	{
		const int dist = levenshtein_distance(src.c_str(), m_sentences[i].lower);
		if (dist < minDistance)
		{
			minDistance = dist;
			sentenceId = i;
		}
	}

	if (sentenceId < 0)
		return "";

	maxFitness = (20 - minDistance) * 0.01f;
	return m_sentences[sentenceId].output;
}


// FastSearch() + ErrorCorrectionSearch()
string cDictionary2::Search(const string &sentence, OUT vector<string> &out, OUT float &maxFitness)
{
	maxFitness = 1.f;

	string result = FastSearch(sentence, out);
	if (result.empty())
		result = ErrorCorrectionSearch(sentence, maxFitness);
	return result;
}


unsigned int cDictionary2::levenshtein_distance(const std::string& s1, const std::string& s2)
{
#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

	const std::size_t len1 = s1.size(), len2 = s2.size();

	for (unsigned int i = 0; i < prevCol.size(); i++)
		prevCol[i] = i;
	for (unsigned int i = 0; i < len1; i++) {
		col[0] = i + 1;
		for (unsigned int j = 0; j < len2; j++)
			col[j + 1] = MIN3(prevCol[1 + j] + 1, col[j] + 1, prevCol[j] + (s1[i] == s2[j] ? 0 : 1));

		col.swap(prevCol);
	}
	return prevCol[len2];
}
