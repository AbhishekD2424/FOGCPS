#include "main_header.h"

// Function to read binary data from a file
vector<char> read_file_data(const string &filename)
{
	ifstream file(filename, ios::binary | ios::ate);
	if (!file.is_open())
	{
		cout << "File not found." << endl;
		exit(1);
	}
	// Determine the file size
	streamsize size = file.tellg();
	file.seekg(0, ios::beg);
	// Read the file data into a vector
	vector<char> data(size);
	if (!file.read(data.data(), size))
	{
		cout << "Error reading file." << endl;
		exit(1);
	}
	return data;
}

// Function to create blocks from binary data
vector<vector<char>> create_blocks(const string &filename, int &number_of_blocks)
{
	vector<char> data = read_file_data(filename);
	// Adjust the block size as needed
	number_of_blocks = data.size() / BLOCK_SIZE;
	if (data.size() % BLOCK_SIZE != 0)
		number_of_blocks++;
	vector<vector<char>> resultBlocks(number_of_blocks);
	for (int i = 0; i < number_of_blocks; i++)
	{
		int blockSize = BLOCK_SIZE;
		if (i == number_of_blocks - 1 && data.size() % BLOCK_SIZE != 0)
			blockSize = data.size() % BLOCK_SIZE;

		resultBlocks[i] = vector<char>(data.begin() + (i * BLOCK_SIZE), data.begin() + (i * BLOCK_SIZE) + blockSize);
	}
	return resultBlocks;
}

int main(int argc, char **argv)
{
	pairing_t pairing;
	pbc_demo_pairing_init(pairing, argc, argv);
	if (!pairing_is_symmetric(pairing))
		pbc_die("pairing must be symmetric");

	//----------------------------------------Step 1 : Setup----------------------------------------
	element_t alpha, g, Ppub;
	element_init_Zr(alpha, pairing);
	element_init_G1(g, pairing);
	element_init_G1(Ppub, pairing);
	// alpha is the Master Secret Key(MSK)
	element_random(alpha);  
	// g is the generator
	element_random(g);  
    // Ppub is the public key
	element_pow_zn(Ppub, g, alpha);  

	//----------------------------------------Step 2 : Key Generation----------------------------------------
	element_t Qid, Did, betaid, Pu;
	element_init_G1(Qid, pairing);
	element_init_G1(Did, pairing);
	element_init_Zr(betaid, pairing);
	element_init_G1(Pu, pairing);
	const char *source = "Any_User_ID";
	char *dynamicString = new char[strlen(source) + 1];
	strcpy(dynamicString, source);
	void *U_id = dynamicString;
	// Hash Functions
	element_from_hash(Qid, U_id, strlen(source));
	// Did is the Partial Private Key  for Idu generated by KGC  
	element_pow_zn(Did, Qid, alpha);  
	// betaid is the Personal Secret Value for U_id
	element_random(betaid);  
    // Public Key for U_id is Pu
	element_pow_zn(Pu, g, betaid);  

	//----------------------------------------Step 3 : Tag Generation----------------------------------------
	element_t a1, a3;
	element_init_G1(a1, pairing);
	element_init_G1(a3, pairing);
	string filename = "/home/abhishek/Documents/FOGCPS/13/f1.txt";
	int number_of_blocks = 0;
	vector<vector<char>> blocks = create_blocks(filename, number_of_blocks);
	// Calculates the authentication tag-information
	char w[number_of_blocks][5000];  
	// Computes the tag-signature
	element_t temp_number[number_of_blocks];
	element_t temp_w[number_of_blocks];
	element_t a4[number_of_blocks];
	// Encrypts the tag using its shared secret key
	for (int i = 0; i < number_of_blocks; i++)  
	{
		element_snprintf(w[i], 5000, "%s%d%B", "f1.txt", i, Pu);
		element_init_G1(temp_w[i], pairing);
		element_init_Zr(temp_number[i], pairing);
		element_init_G1(a4[i], pairing);
		element_from_hash(temp_w[i], w[i], 5000);
		element_pow_zn(a3, temp_w[i], betaid);
		string blockString(blocks[i].begin(), blocks[i].end());
		long number = binaryStringToNumber(blockString);
		element_set_si(temp_number[i], number);
		element_pow_zn(a1, Did, temp_number[i]);
		element_mul(a4[i], a1, a3);
	}

	//----------------------------------------Step 4 : Challenge Generation----------------------------------------
    // Random number taken as size of the challenge vector
	int random_n = 20;  
	// Declare and initialize the v1 vector
	vector<element_t> v1(number_of_blocks);  
	for (int i = 0; i < number_of_blocks; i++)
	{
		element_init_Zr(v1[i], pairing);
	}
	// Call the function to generate random challenge array
	vector<int> challenge = generateRandomChallenge(number_of_blocks, random_n);

	for (int i = 0; i < random_n; i++)
	{
			// Initialize v1 elements
			element_init_Zr(v1[i], pairing);
			element_random(v1[i]);
	}

	//----------------------------------------Step 5 :Proof generation----------------------------------------
	// Auditor --> CSP: t,c,IDf
	element_t mu_r, m1, mu, sigu_r, x5, sigu;
	// Sigu = π(i∈I) Si^ui
	element_init_G1(sigu_r, pairing);  
	element_init_G1(sigu, pairing);
	// Mu = Σ(i∈I) vibi
	element_init_Zr(mu_r, pairing);  
	element_init_Zr(m1, pairing);
	element_init_Zr(mu, pairing);
	// Computing proof of possession for the challenged blocks
	for (int i = 0; i < random_n; i++)  
	{
		element_mul(mu, temp_number[challenge[i]], v1[challenge[i]]);
		element_add(mu_r, mu_r, mu);
		element_pow_zn(sigu, a4[challenge[i]], v1[challenge[i]]);
		element_mul(sigu_r, sigu_r, sigu);
	}

	//----------------------------------------Step 6 : Proof verification----------------------------------------
	element_t b1, b2, b3, b4, x1, x2;
	element_init_GT(b1, pairing);
	element_init_GT(b2, pairing);
	element_init_GT(b3, pairing);
	element_init_GT(b4, pairing);
	element_init_G1(x1, pairing);
	element_init_G1(x2, pairing);

	element_pairing(b1, sigu_r, g);
	element_printf("\n\nLHS = %B \n", b1);
	element_pow_zn(x1, Qid, mu_r);
	element_pairing(b2, x1, Ppub);
	element_init_G1(x5, pairing);

	for (int i = 0; i < random_n; i++)
	{
		element_pow_zn(x2, temp_w[challenge[i]], v1[challenge[i]]);
		element_mul(x5, x5, x2);
	}
	element_pairing(b3, x5, Pu);
	element_mul(b4, b2, b3);
	element_printf("\n\nRHS = %B \n", b4);

	return 0;
}
