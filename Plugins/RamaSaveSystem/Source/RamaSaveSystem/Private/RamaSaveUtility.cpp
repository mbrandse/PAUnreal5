// Copyright 2018 by Nathan "Rama" Iyer. All Rights Reserved.
#include "RamaSaveSystemPrivatePCH.h"
#include "RamaSaveUtility.h"

#include "Runtime/Core/Public/Serialization/ArchiveSaveCompressedProxy.h"
#include "Runtime/Core/Public/Serialization/ArchiveLoadCompressedProxy.h"
#include "Runtime/Core/Public/Serialization/CompressedChunkInfo.h"
 
bool URamaSaveUtility::IsIllegalForSavingLoading(UClass* Class)
{
	if(!Class) return true;
	
	//Store arbitrary game data in an empty actor that is placed in world
	// or via the use of a UE4 Save Game Object
	return Class->IsChildOf(APlayerController::StaticClass())
			|| Class->IsChildOf(APlayerState::StaticClass())
			|| Class->IsChildOf(AGameState::StaticClass())
			|| Class->IsChildOf(AGameMode::StaticClass());
			 
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Save To Compressed File, by Rama
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool URamaSaveUtility::WriteToFile(TArray<uint8>& Uncompressed, const FString& FullFilePath, bool Compress)
{
	//~~~ No Data ~~~ 
	if (Uncompressed.Num() <= 0) return false;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
	//SAVING COMPRESSED IS NOT WORKING FOR SOME REASON IN UE5!
	//Compress = false;
	//UE_LOG(RamaSave, Warning,TEXT("File saved uncompressed, Rama still solving UE5 ZIP Compression issue %s"), *FullFilePath);
	  
	//Compressed?
	if(Compress)
	{
		return CompressAndWriteToFile(Uncompressed,FullFilePath);
	}
	else
	{
		//Uncompressed save!
		return FFileHelper::SaveArrayToFile( Uncompressed, *FullFilePath);
	}
} 

bool URamaSaveUtility::CompressAndWriteToFile(TArray<uint8>& Uncompressed, const FString& FullFilePath)
{
	//~~~ No Data ~~~ 
	if (Uncompressed.Num() <= 0) return false;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~
	//	Write to File

	//! Perhaps the only use of compression in the entire ue5 engine, checked Editor and Runtime
	/*
	bool FGenericPlatformSymbolication::SaveSymbolDatabaseForBinary(FString TargetFolder, FString Name, FGenericPlatformSymbolDatabase& Database)
	{
		bool bOk = false;
		
		FString ModuleName = FPaths::GetBaseFilename(Name);
		FString OutputFile = (TargetFolder / ModuleName) + TEXT(".udebugsymbols");

		TArray<uint8> DataBuffer;
		FArchiveSaveCompressedProxy DataArchive(DataBuffer, NAME_Zlib, COMPRESS_BiasSpeed);
		DataArchive << Database;
		DataArchive.Flush();
		if(!DataArchive.GetError())
		{
			if(FFileHelper::SaveArrayToFile(DataBuffer, *OutputFile))
			{
				bOk = true;
			}
			else
			{
				UE_LOG( LogGenericPlatformSymbolication, Error, TEXT( "Unable to write debug symbols to output file." ) );
			}
		}
		else
		{
			UE_LOG( LogGenericPlatformSymbolication, Error, TEXT( "Unable to serialise debug symbols." ) );
		}
		
		return bOk;
	}
	*/

	//~~~~~~~~~~~~~~~~~~~~~~~~~~
	//					Compress
	//~~~ Compress File ~~~
	//tmp compressed data array
	TArray<uint8> CompressedData;
	FArchiveSaveCompressedProxy Compressor(CompressedData, NAME_Zlib, COMPRESS_BiasSpeed);
	 
	//Send entire binary array/archive to compressor
	Compressor << Uncompressed;
	
	//send archive serialized data to binary array
	Compressor.Flush();
	
	if (!FFileHelper::SaveArrayToFile(CompressedData, *FullFilePath))
	{
		return false;
	}
	
	//~~~ Clean Up ~~~
	
	//~~~ Free Binary Arrays ~~~
	Compressor.FlushCache();
	CompressedData.Empty();
	Uncompressed.Empty();
	
	return true;
} 

static bool IsCompressedFile(const FString& FullFilePath)
{
	FArrayReader PossiblyCompressedData;
	if (!FFileHelper::LoadFileToArray(PossiblyCompressedData, *FullFilePath))
	{    
		return false;
	}
	
	//See archive.cpp
	
	// FPackageFileSummary has int32 Tag == PACKAGE_FILE_TAG
	// this header does not otherwise match FPackageFileSummary in any way

	// low 32 bits of ARCHIVE_V2_HEADER_TAG are == PACKAGE_FILE_TAG
	#define ARCHIVE_V2_HEADER_TAG	(PACKAGE_FILE_TAG | ((uint64)0x22222222<<32) )
	
	
	//previous thingie no longer in use, see the //
		
	/*
	// Serialize package file tag used to determine endianness in LoadCompressedData.
	FCompressedChunkInfo PackageFileTag;
	//PackageFileTag.CompressedSize	= PACKAGE_FILE_TAG;			//wuz this //<~~~~~~~~~~~~~~~~~~~~~~~~
	PackageFileTag.CompressedSize	= ARCHIVE_V2_HEADER_TAG;		nowz thiz	//<~~~~~~~~~~~~~~~~~~~~~~~~
	PackageFileTag.UncompressedSize	= GSavingCompressionChunkSize;
	*this << PackageFileTag;
	*/
	
	
	//! I think I'll use Oodle next time
	//! <3 Rama
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//Core Compression Test (invented by me (it was hard)! <3 Rama)
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	FCompressedChunkInfo PackageFileTag;
	PackageFileTag.CompressedSize	= 0;
	PackageFileTag.UncompressedSize	= 0;
	PossiblyCompressedData << PackageFileTag;
	 
	return PackageFileTag.CompressedSize == ARCHIVE_V2_HEADER_TAG;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 	Decompress From File, by Rama
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool URamaSaveUtility::DecompressFromFile(const FString& FullFilePath, TArray<uint8>& Uncompressed)
{
	//~~~ File Exists? ~~~
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FullFilePath)) return false;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//Uncompresssed?
	if(!IsCompressedFile(FullFilePath))
	{
		return FFileHelper::LoadFileToArray(Uncompressed, *FullFilePath);
	}
	
	//tmp compressed data array
	TArray<uint8> CompressedData;
	
	if (!FFileHelper::LoadFileToArray(CompressedData, *FullFilePath))
	{
		//! Could not load file
		return false;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	}
	
	//~~~ Decompress File ~~~
	FArchiveLoadCompressedProxy Decompressor(CompressedData, NAME_Zlib);
	
	//Decompression Error?
	if(Decompressor.GetError())
	{
		return false;
		//~~~~~~~~~~~~
	}
	
	//Send Data from Decompressor to Vibes array
	Decompressor << Uncompressed;

	return true;
}