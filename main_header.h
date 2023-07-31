// main_header.h

#ifndef MAIN_HEADER_H
#define MAIN_HEADER_H

#include <pbc/pbc.h>
#include <pbc/pbc_test.h>
#include <bits/stdc++.h>

#define BLOCK_SIZE 25
using namespace std;

// converting binary string to number
long binaryStringToNumber(string binaryString) 
{
	int length = binaryString.length();
	long long number = 0;
	int c = 0;
	for (int i = length - 1; i >= 0; i--)
	{
		long power = pow(2, c++);
		number += binaryString[i] * power;
	}
	return number;
}

// Function to generate random challenge array and compute v1 elements
vector<int> generateRandomChallenge(int number_of_blocks, int random_n)
{
	vector<int> challenge;
	challenge.resize(random_n);
	// Set to store unique random numbers
	unordered_set<int> randomSet;
	// Generate random numbers within the limit
	srand(static_cast<unsigned>(time(nullptr)));
	int i = 0;
	while (i < random_n)
	{
		int random_number = rand() % number_of_blocks;
		// Check if the random number is unique
		if (randomSet.find(random_number) == randomSet.end())
		{
			randomSet.insert(random_number);
			challenge[i] = random_number;
            // Increment value of i
			i++;
		}
	}
	return challenge;
}

#endif // MAIN_HEADER_H
