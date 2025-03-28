#ifndef BH_HIGHSCORE_H
#define BH_HIGHSCORE_H

#include <cstring>

#define BH_HIGHSCORE_NAME_LEN 16

////////////////////////////////////////////////////////////////////////////////
namespace bh{

class Highscore
{
public:

	Highscore(short cNumEntries);
	~Highscore();
	bool Load();
	bool Save();
	void InsertScore(char const* name,int score);
	
	inline bool IsHighScore(int score)
	{
		return (score > entries[numEntries - 1]->score);
	}

	void DEBUG_Print();

protected:
private:

	struct Pair
	{
		Pair()
			:score(0)
		{
			strncpy(name,"UNDEFINED",BH_HIGHSCORE_NAME_LEN);
		}

		Pair(char const* cName,int cScore)
			:score(cScore)
		{
			strncpy(name,cName,BH_HIGHSCORE_NAME_LEN);
			name[BH_HIGHSCORE_NAME_LEN - 1] = '\0';
		}

		char name[BH_HIGHSCORE_NAME_LEN];
		int score;
	};

	Pair** entries;
	short numEntries;
};

};
////////////////////////////////////////////////////////////////////////////////
#endif
