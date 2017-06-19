#pragma once
#include <string>
#include <set>
bool isSubsequence(const std::string &lhs, const std::string &rhs, int m = 0, int n = 0)
{
	if (m == lhs.length())
	{
		return true;
	}
	if (n == rhs.length())
	{
		return false;
	}
	if (lhs[m] == rhs[n])
	{
		return isSubsequence(lhs, rhs, m + 1, n + 1);
	}
	else
	{
		return isSubsequence(lhs, rhs, m, n + 1);
	}
}
//Finds all strings in collections in which str is a subsequence of collection, storing into supersequences
void matchSubsequence(const std::string &str, const std::set<std::string> &collection, std::set<std::string> &supersequences)
{
	for (const auto &word : collection)
	{
		if (isSubsequence(str, word))
		{
			supersequences.insert(word);
		}
	}
}
