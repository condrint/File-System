#include "stdafx.h"
#include "FileDirectory.h"
#include <iostream>
#define EOF 0xffff

using std::cout;

FileDirectory::FileDirectory()
{	//initiliaze file directory and FAT to all 0's
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 32; j++)
			fileDirectory[i][j] = 0;
	for (int i = 0; i < 256; i++) 
		FAT16[i] = 0; 
}

bool FileDirectory::create(char filename[], int numberBytes) 
{
	//check directory 
	char directoryIndex = -1;
	for (int i = 0; i < 4; i++)
	{
		if (fileDirectory[i][0] == 0)
		{
			directoryIndex = i;
			break;
		}

		//no open spot was found
		if (i == 3)
		{
			return false;
		}
	}
		

	//check FAT (each FAT entry is 2 bytes and each fat entry corresponds to 4 bytes of available data)
	int byteCount = 0;
	for (int i = 2; i < 256 && byteCount <= numberBytes; i++)
	{
		if (FAT16[i] == 0)
		{
			byteCount += 4;
		}

		//not enough space in FAT table
		if (i == 255)
		{
			return false;
		}
	}

	//create file name in directory
	for (int i = 0; i < 8; i++)
	{
		fileDirectory[directoryIndex][i] = filename[i];
	}

	//file name created succesfully
	return true;

}
bool FileDirectory::deleteFile(char filename[])
{
	unsigned short int sectorAddress;

	//iterate through file directory
	for (int i = 0; i < 4; i++)
	{
		//iterate through name of a file in file directory
		for (int j = 0; j < 8; j++)
		{
			
			//name doesnt not match for filedirectory[i]
			if (fileDirectory[i][j] != filename[j])
			{	
				break;
			}
			
			//matching name in file directory
			if (j == 7)
			{
				//clear first entry in file directory
				fileDirectory[i][0] = 0;
				short int firstAddress = (fileDirectory[i][27] << 8) + fileDirectory[i][26];

				//iterate through FAT16 table and remove file
				short int prev, curr = firstAddress;
				while (!(FAT16[curr] == EOF))
				{
					prev = curr; 
					curr = FAT16[prev];
					FAT16[prev] = 0;					
				}
				FAT16[curr] = 0;
				return true;
			}
		}
		if (i == 3) return false;
	}
}
bool FileDirectory::read(char filename[], char fileData[]) 
{	
	//check if file exists
	char directoryIndex;
	bool found = false;

	for (int i = 0; i < 4; i++)
	{
		//iterate through name of a file in file directory
		for (int j = 0; j < 8; j++)
		{

			//name doesnt not match for filedirectory[i]
			if (fileDirectory[i][j] != filename[j])
			{
				found = false;
				break;
			}

			//matching name in file directory
			if (j == 7)
			{
				directoryIndex = i;
				found = true;
				break;
			}
		}
		if (found == true)
		{
			break;
		}
	}

	if ((found == false))
	{
		return false;
	}

	//read data from directory and input into file data (hopefully the file data array is big enough...)
	//date
	fileData[0] = fileDirectory[directoryIndex][25];
	fileData[1] = fileDirectory[directoryIndex][24];
	
	//time
	fileData[2] = fileDirectory[directoryIndex][23];
	fileData[3] = fileDirectory[directoryIndex][22];
	
	//size
	fileData[4] = fileDirectory[directoryIndex][31];
	fileData[5] = fileDirectory[directoryIndex][30];
	fileData[6] = fileDirectory[directoryIndex][29];
	fileData[7] = fileDirectory[directoryIndex][28];
	
	//retrieve data from data clusters using FAT16
	short int startingAddress = fileDirectory[directoryIndex][27] << 8 + fileDirectory[directoryIndex][26];
	int numberBytes = fileData[4] << 24 + fileData[5] << 16 + fileData[6] << 8 + fileData[7];

	//initalize variables that will be used for FAT16
	short int FATindices[256];
	FATindices[0] = startingAddress;
	char q = 0;

	//collect indices (remember that each spot in FAT16 table is 2 bytes)
	while (!(FATindices[q] == EOF))
	{
		q++;
		FATindices[q] = FAT16[FATindices[q - 1]];
	}

	for (int i = 0; i < q; i++)
	{
		//each iteration writes to one cluster
		fileData[(i * 4) + 8] = data[(FATindices[i] * 4)]; //offset each one by 8 because first 8 entries are used above
		fileData[(i * 4) + 1 + 8] = data[(FATindices[i] * 4) + 1];
		fileData[(i * 4) + 2 + 8] = data[(FATindices[i] * 4) + 2];
		fileData[(i * 4) + 3 + 8] = data[(FATindices[i] * 4) + 3];
	}

	return true;


}
bool FileDirectory::write(char filename[], int numberBytes, char fileData[], short int year, short int month, short int day, int hour, int minute, int second)
{
	unsigned short int i, j, firstAddress, unused[256], date, time;
	bool found = false, foundstart = false;
	char directoryIndex;


	//iterate through FAT16 with i, index 0 & 1 are reserved
	for (i = 2; i < 256; i++)
	{
		if (FAT16[i] == 0)
		{
			firstAddress = i;
			foundstart = true;
			break;
		}
	}
	

	//check file directory for filename[]
	for (i = 0; i < 4; i++)
	{
		//iterate through name of a file in file directory
		for (int j = 0; j < 8; j++)
		{

			//name doesnt not match for filedirectory[i]
			if (fileDirectory[i][j] != filename[j])
			{
				found = false;
				break;
			}

			//matching name in file directory
			if (j == 7)
			{
				directoryIndex = i;
				found = true;
				break;
			}
		}
		if (found == true)
		{
			break;
		}
	}


	if ((found == false) || (foundstart == false))
	{
		return false;
	}
	
	//initalize variables that will be used to write to FAT16
	unused[0] = firstAddress; j = 1;
	
	//collect unused indices (remember that each spot in FAT16 table is 2 bytes)
	for (i = firstAddress + 1; (i != firstAddress) && j * 4 < numberBytes; i++)
	{
		if (FAT16[i] == 0) unused[j++] = i;

		//circular
		if (i == 255)
		{
			i = -1;
		}
	}
	
	//if not enough unused indices are found, return false (remember that each spot in FAT16 table is 2 bytes)
	if (i == 256 && j * 4 < numberBytes)
	{
		return false;
	}

	
	//iterate through unused indices and create a linked list
	for (i = 0; i < j-1; i++)
	{	
		FAT16[unused[i]] = unused[i + 1];
	}
	FAT16[unused[j-1]] = EOF;
	
	
	//write to directory
	i = directoryIndex;
	for (j = 0; j < 8; j++)
	{
		fileDirectory[i][j] = filename[j];
	}

	date = ((year - 1980) << 9) + (month << 5) + day;
	fileDirectory[i][25] = (date >> 8) & 0b0000000011111111;
	fileDirectory[i][24] = date & 0b0000000011111111;
								
	time = (hour << 11) + (minute << 5) + second / 2;
	fileDirectory[i][23] = time >> 8;
	fileDirectory[i][22] = time;
	
	fileDirectory[i][31] = numberBytes >> 24;
	fileDirectory[i][30] = numberBytes >> 16;
	fileDirectory[i][29] = numberBytes >> 8;
	fileDirectory[i][28] = numberBytes;
	
	fileDirectory[i][27] = firstAddress >> 8;
	fileDirectory[i][26] = firstAddress;
	
	//write data to data sector in little endian
	//index data array by the same clusters we wrote to the FAT16 with
	//since there is 4 bytes/cluster we index in groups of 4
	for (i = 0; i * 4 < numberBytes; i++)
	{
		//each iteration writes to one cluster
		data[(unused[i] * 4)] = fileData[(i * 4)];
		data[(unused[i] * 4) + 1] = fileData[(i * 4) + 1];
		data[(unused[i] * 4) + 2] = fileData[(i * 4) + 2];
		data[(unused[i] * 4) + 3] = fileData[(i * 4) + 3];
	}
}
void FileDirectory::printClusters(char filename[]) 
{
	//check if file exists
	char directoryIndex;
	bool found = false;

	for (int i = 0; i < 4; i++)
	{
		//iterate through name of a file in file directory
		for (int j = 0; j < 8; j++)
		{

			//name doesnt not match for filedirectory[i]
			if (fileDirectory[i][j] != filename[j])
			{
				found = false;
				break;
			}

			//matching name in file directory
			if (j == 7)
			{
				directoryIndex = i;
				found = true;
				break;
			}
		}
		if (found == true)
		{
			break;
		}
	}

	if ((found == false))
	{
		cout << "File not found in directory";
		return;
	}
	//retrieve data from data clusters using FAT16
	short int startingAddress = (fileDirectory[directoryIndex][27] << 8) + fileDirectory[directoryIndex][26];

	//initalize variables that will be used for FAT16
	int FATindices[256];
	FATindices[0] = startingAddress;
	int q = 0;

	cout << "Cluster addresses are as follows for " << fileDirectory[directoryIndex][0] << fileDirectory[directoryIndex][1] << fileDirectory[directoryIndex][2] << fileDirectory[directoryIndex][3] << fileDirectory[directoryIndex][4] << fileDirectory[directoryIndex][5] << fileDirectory[directoryIndex][6] << fileDirectory[directoryIndex][7] << "\n";
	//collect indices (remember that each spot in FAT16 table is 2 bytes)

	while (!(FATindices[q] == EOF))
	{
		q++;
		cout << FATindices[q - 1] << " -> ";
		FATindices[q] = FAT16[FATindices[q - 1]];
	}

	cout << "EOF \n\n";




}
void FileDirectory::printDirectory() 
{
	//iterate through directory
	for (int i = 0; i < 4; i++)
	{
		cout << "File " << i+1 << ": \n";
		if (fileDirectory[i][0] == 0)
		{
			cout << "\tEntry is empty\n";
		}
		else
		{
			
			cout << "\tName: " << fileDirectory[i][0] << fileDirectory[i][1] << fileDirectory[i][2] << fileDirectory[i][3] << fileDirectory[i][4] << fileDirectory[i][5] << fileDirectory[i][6] << fileDirectory[i][7] << "\n";


			
			short int msb = fileDirectory[i][25];
			short int lsb = fileDirectory[i][24];
			short int date = (msb << 8) + lsb;
			short int year = date >> 9;
			short int month = date >> 5 & 0b0000000000011111; //mask out year
			short int day = date & 0b0000000000011111;

			cout << "\tDate, year: " << year + 1980 << "\n";
			cout << "\tDate, month: " << month << "\n";
			cout << "\tDate, day: " << day << "\n";
			
			//time = (hour << 11) + (minute << 5) + second / 2;
			msb = fileDirectory[i][23];
			lsb = fileDirectory[i][22];
			unsigned short int time = (msb << 8) + lsb;
			unsigned short int hour = time >> 11;
			unsigned short int minute = time >> 5 & 0b0000000000111111;
			unsigned short int second = time & 0b0000000000011111;

			cout << "\tTime, hour: " << hour << "\n";
			cout << "\tTime, minute: " << minute << "\n";
			cout << "\tTime, second: " << second * 2 << "\n";
			
			int size = (fileDirectory[i][31] << 24) + (fileDirectory[i][30] << 16) + (fileDirectory[i][29] << 8) + fileDirectory[i][28];
			cout << "\tSize: " << size << "\n";
			
			short int firstAddress = (fileDirectory[i][27] << 8) + fileDirectory[i][26];
			cout << "\tFAT16 starting address: " << firstAddress << "\n\n";
		}
	}

	
}
void FileDirectory::printData(char filename[]) 
{	
	//check if file exists
	char directoryIndex;
	bool found = false;

	for (int i = 0; i < 4; i++)
	{
		//iterate through name of a file in file directory
		for (int j = 0; j < 8; j++)
		{

			//name doesnt not match for filedirectory[i]
			if (fileDirectory[i][j] != filename[j])
			{
				found = false;
				break;
			}

			//matching name in file directory
			if (j == 7)
			{
				directoryIndex = i;
				found = true;
				break;
			}
		}
		if (found == true)
		{
			break;
		}
	}
	
	if ((found == false))
	{
		cout << "File not found in directory";
		return;
	}

	//retrieve data from data clusters using FAT16
	short int startingAddress = (fileDirectory[directoryIndex][27] << 8) + fileDirectory[directoryIndex][26];

	//initalize variables that will be used for FAT16
	int FATindices[256];
	FATindices[0] = startingAddress;
	int q = 0;
	
	cout  << "Cluster addresses are as follows for " << fileDirectory[directoryIndex][0] << fileDirectory[directoryIndex][1] << fileDirectory[directoryIndex][2] << fileDirectory[directoryIndex][3] << fileDirectory[directoryIndex][4] << fileDirectory[directoryIndex][5] << fileDirectory[directoryIndex][6] << fileDirectory[directoryIndex][7] << "\n";
	//collect indices (remember that each spot in FAT16 table is 2 bytes)
	
	while ( ! (FATindices[q] == EOF))
	{
		q++;
		cout << FATindices[q-1] << " -> ";
		FATindices[q] = FAT16[FATindices[q - 1]];
	}
	
	cout << "EOF \n\n";

	for (int i = 0; i < q; i++)
	{
		//each iteration reads and prints one cluster
		cout << "Data address: " << FATindices[i] << "\n";
		int data1 = data[(FATindices[i] * 4)];
		int data2 = data[(FATindices[i] * 4)+1];
		int data3 = data[(FATindices[i] * 4)+2];
		int data4 = data[(FATindices[i] * 4)+3];

		cout << "Byte 1: " << data1 << "\n"; //offset each one by 8 because first 8 entries are used above
		cout << "Byte 2: " << data2 << "\n";
		cout << "Byte 3: " << data3<< "\n";
		cout << "Byte 4: " << data4 << "\n\n";
	}

}