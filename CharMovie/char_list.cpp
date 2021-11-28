#include "char_list.hpp"

using namespace std;

namespace clist
{
	// determine if the same character is used more than once
	// args:
	//      str: string to check duplicate
	// return:
	//      true: 'str' is duplicate-string
	//      false: 'str' is not duplicate-string
	bool isDupChar(string str)
	{
		string s2 = str;
		// remove duplicates
		sort(s2.begin(), s2.end());
		s2.erase(unique(s2.begin(), s2.end()), s2.end());
		// determine if there are erased characters.
		return s2.length() < str.length();
	}

	// func: disp sample character (multi-byte character is not supported.)
	// args:
	//      line_char_num: number of characters to be outputted per line
	// return:
	//      0: success
	//      -1: failure ('characters' contains duplicate character)
	int charList(unsigned int line_char_num, string characters, bool do_check)
	{
		if (do_check && isDupChar(characters))
		{
			return -1;
		}

		string rvs = ccolor::bg::BG_White + ccolor::fg::FG_Black;
		string nrm = ccolor::bg::BG_Black + ccolor::fg::FG_White;
		string def = ccolor::deco::Deco_DefaultAll;

		int col = line_char_num;
		int row = (int)characters.length() / col;

		if (characters.length() % 10 != 0)
		{
			row++;
		}

		string space_line = string(col * 2 + 3, ' ');

		// white line
		cout << rvs << space_line << def << endl;
		cout << rvs << " " << ends;

		for (size_t i = 0; i < characters.length(); i++)
		{
			// output white-space and character
			cout << rvs << " " << def << characters[i] << ends;

			// next line
			if ((i + 1) % col == 0)
			{
				// 2x white-space at the end of line
				cout << rvs << "  " << def << endl;
				// white line
				cout << rvs << space_line << def << endl;
				// white-space at the top of line
				cout << rvs << " " << ends;
			}
		}

		// fill in the extra space
		cout << rvs << string((col - characters.length() % col) * 2 + 2, ' ') << def << endl;
		// white line
		cout << rvs << space_line << def << endl;

		return 0;
	}
}