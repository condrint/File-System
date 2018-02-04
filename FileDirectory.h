#pragma once
class FileDirectory
{
private:
	unsigned char fileDirectory[4][32];
	unsigned short int FAT16[256];
	unsigned char data[1024];

public:
	FileDirectory(); 

	bool create(char filename[], int numberBytes);
	bool deleteFile(char filename[]);
	bool read(char filename[], char fileData[]);
	bool write(char filename[], int numberBytes, char fileData[], short int year, short int month, short int day, int hour, int minute, int second);
	void printClusters(char filename[]);
	void printDirectory();
	void printData(char filename[]);
};