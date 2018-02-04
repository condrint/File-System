#include "stdafx.h"
#include "FileDirectory.h"
#include <iostream>
#include <ctime> 
#include <cstdlib>

using std::cout;

int main()
{
	/*Write a driver function, i.e. main(), to test this program by
1.	create and write a file, file1,  of 40 bytes,
2.	create and write a file, file2,  of 200 bytes,
3.	create and write a file, file3,  of 300 bytes,
4.	create and write a file, file4,  of 500 bytes.
5.	delete file2,
6.	create and write a file, file4,  of 500 bytes.
*/

	FileDirectory directory;

	//file 1
	char file1name[8] = { 'f', 'i', 'l', 'e', '1',' ', ' ', ' ' };
	int file1size = 40;
	char file1data[40] = {};
	for (int i = 0; i < 40; i++)
	{
		file1data[i] = (rand() % 2) + 1;
	}

	directory.create(file1name, file1size);
	directory.write(file1name, file1size, file1data, 2018, 1, 29, 13, 42, 30);
	
	//file 2
	char file2name[8] = { 'f', 'i', 'l', 'e', '2',' ', ' ', ' ' };
	int file2size = 200;
	char file2data[200] = {};
	for (int i = 0; i < 200; i++)
	{
		file2data[i] = (rand() % 10) + 20;
	}

	directory.create(file2name, file2size);
	directory.write(file2name, file2size, file2data, 2018, 2, 2, 11, 40, 2);

	//file 3
	char file3name[8] = { 'f', 'i', 'l', 'e', '3',' ', ' ', ' ' };
	int file3size = 300;
	char file3data[300] = {};
	for (int i = 0; i < 300; i++)
	{
		file3data[i] = (rand() % 10) + 30;
	}

	directory.create(file3name, file3size);
	directory.write(file3name, file3size, file3data, 2017, 11, 13, 3, 02, 23);
	
	//file 4 - i made it only 400 bytes because there is not enough space for a 500 byte file ( 500 + 300 + 200 + 40 = 1040 which is greater than 1024, our data limit)
	char file4name[8] = { 'f', 'i', 'l', 'e', '4',' ', ' ', ' ' };
	int file4size = 400;
	char file4data[400] = {};
	for (int i = 0; i < 400; i++)
	{
		file4data[i] = (rand() % 10) + 40;
	}

	directory.create(file4name, file4size);
	directory.write(file4name, file4size, file4data, 2018, 1, 15, 16, 20, 4);
	directory.printDirectory();
	directory.deleteFile(file2name);
	directory.deleteFile(file3name);
	cout << "------------------------------------------------------------------------------------- after deleting file 2 and 3:\n";
	directory.printDirectory();

	
	//file 5
	char file5name[8] = { 'f', 'i', 'l', 'e', '5',' ', ' ', ' ' };
	int file5size = 500;
	char file5data[500] = {};
	for (int i = 0; i < 500; i++)
	{
		file5data[i] = (rand() % 10) + 60;
	}

	directory.create(file5name, file5size);
	directory.write(file5name, file5size, file5data, 2018, 1, 8, 19, 10, 6);
	cout << "------------------------------------------------------------------------------------- after adding file 5:\n";
	directory.printDirectory();
	directory.printClusters(file1name);
	directory.printClusters(file4name);
	directory.printClusters(file5name);
	directory.printData(file1name);

    return 0;
}

