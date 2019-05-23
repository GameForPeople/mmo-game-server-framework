#include "pch.h"

#include "MonsterLoader.h"

MonsterLoader::MonsterLoader()
{
	// Slime
	{
		std::string fileNameBuffer{ "Data/Monster_Slime_Position.txt" };
		std::ifstream inFile(fileNameBuffer, std::ios::in);

		while (inFile.peek() != EOF)
		{
			_PosType xPos{}, yPos{};

			inFile
				>> xPos
				>> yPos;

			slimePosCont.emplace_back(std::make_pair(xPos, yPos));
		}
	}

	//debug
	//for (const auto& iter : slimePosCont)
	//{
	//	std::cout << iter.first << " " << iter.second << "\n";
	//}
}