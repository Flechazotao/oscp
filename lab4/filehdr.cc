// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"

//----------------------------------------------------------------------
//	构造函数：用于初始化ModTime以及文件所占扇区分布数组
//----------------------------------------------------------------------
FileHeader::FileHeader()
{
    memset(dataSectors, 0, sizeof(dataSectors));
    lastModTime = 0;
    //numBytes = 0;           // 添加这行
    //numSectors = 0;         // 添加这行
}

//----------------------------------------------------------------------
//	扩展文件空间：每次写入后都需要调用这个函数扩展文件空间
//----------------------------------------------------------------------

bool FileHeader::ChangeFileSize (int newFileSize)
{
    if(newFileSize <= numBytes) // 如果 新文件字节数 小于 原文件字节数，则不需要扩展
        return true;

    int numSectorsSet = divRoundUp(newFileSize, SectorSize); //计算新文件所占的扇区数
    if (numSectorsSet == numSectors) { // 文件大小扩展，但是扇区数没变
        numBytes = newFileSize; 
        return true;
    }

    BitMap *freeMap = new BitMap(NumSectors);
    OpenFile *bitMapFile = new OpenFile(0);  // 0号扇区是空闲空间位图
    freeMap->FetchFrom(bitMapFile); 

    if (numSectorsSet > NumDirect || freeMap->NumClear() < (numSectorsSet - numSectors)) {
        printf("扩展文件空间失败，磁盘文件空间不足\n");
        delete bitMapFile;
        delete freeMap;
        return false;
    }

    for (int i = numSectors; i < numSectorsSet; i++)
        dataSectors[i] = freeMap->Find();

    freeMap->WriteBack(bitMapFile);
    numBytes = newFileSize;
    numSectors = numSectorsSet;
    DEBUG('f', "文件大小为： %d, %d 总扇区数为：\n", numBytes, numSectors);
    delete bitMapFile;
    delete freeMap;
    return true;
}

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space

    for (int i = 0; i < numSectors; i++)
	dataSectors[i] = freeMap->Find();
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
    for (int i = 0; i < numSectors; i++) {
	ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	freeMap->Clear((int) dataSectors[i]);
    }
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    synchDisk->ReadSector(sector, (char *)this);
    numSectors  = divRoundUp(numBytes, SectorSize);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    return(dataSectors[offset / SectorSize]);
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print(bool bPrintTime)
{
    int i, j, k;
    char *data = new char[SectorSize];

    //打印文件大小，时间，文件块号
    if(bPrintTime)
    printf("文件头信息:  文件大小: %d.  文件上次修改时间: %s. 文件所占块号:\n", numBytes, ctime((time_t*)&lastModTime));
    else printf("文件头信息:  文件大小: %d.  文件所占块号:\n", numBytes);	
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\n文件内容:\n");
    for (i = k = 0; i < numSectors; i++) {
	synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }
    delete [] data;
}

time_t FileHeader::getModTime()  // Get last modify time
{
    return (time_t)lastModTime;
}

void FileHeader::setModTime(time_t modTime)  // Set last modify time
{
    lastModTime = (unsigned)modTime;
}

