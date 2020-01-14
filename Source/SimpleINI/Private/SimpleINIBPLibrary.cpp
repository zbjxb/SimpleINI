// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "SimpleINIBPLibrary.h"
#include "SimpleINI.h"

TArray<TSharedPtr<IniFile>> USimpleINIBPLibrary::INIs;

USimpleINIBPLibrary::USimpleINIBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

//float USimpleINIBPLibrary::SimpleINISampleFunction(float Param)
//{
//	return -1;
//}

bool USimpleINIBPLibrary::LoadIniFile( const FString& FilePath, bool ClearContent /*= false*/ )
{
	TSharedPtr<IniFile> Ini = FindFileOpened( FilePath );
	if (!Ini)
	{
		Ini = TSharedPtr<IniFile>( new IniFile( ) );
		INIs.Add( Ini );
	}
	return Ini->LoadFile( FilePath, ClearContent );
}

bool USimpleINIBPLibrary::GetValue( const FString& FilePath, const FString& SectionName, const FString& Key, FString& Value, bool& IsValid, bool CloseAfterFinish/* = false*/ )
{
	TSharedPtr<IniFile> Ini = FindFileOpened( FilePath );
	if (!Ini)
	{
		Ini = TSharedPtr<IniFile>( new IniFile( ) );
		INIs.Add( Ini );
		if (!Ini->LoadFile( FilePath ))
		{ 
			return false;
		}
	}

	bool bRet = Ini->GetValue( SectionName, Key, Value, IsValid );
	if (CloseAfterFinish)
	{
		CloseIniFile( FilePath );
	}
	return bRet;
}

bool USimpleINIBPLibrary::SetValue( const FString& FilePath, const FString& SectionName, const FString& Key, const FString& Value, bool CloseAfterFinish/* = false*/ )
{
	TSharedPtr<IniFile> Ini = FindFileOpened( FilePath );
	if (!Ini)
	{
		Ini = TSharedPtr<IniFile>( new IniFile( ) );
		INIs.Add( Ini );
		if (!Ini->LoadFile( FilePath ))
		{
			return false;
		}
	}

	bool bRet = Ini->SetValue( SectionName, Key, Value );
	if (CloseAfterFinish)
	{
		bRet = (Ini->Save( ) && bRet);
		CloseIniFile( FilePath );
	}
	return bRet;
}

bool USimpleINIBPLibrary::SaveIniFile( const FString& FilePath, bool CloseAfterFinish/* = false*/ )
{
	TSharedPtr<IniFile> Ini = FindFileOpened( FilePath );
	if (!Ini)
	{
		return false;
	}

	bool bRet = Ini->Save( );
	if (CloseAfterFinish)
	{
		CloseIniFile( FilePath );
	}
	return bRet;
}

bool USimpleINIBPLibrary::ReloadIniFile( const FString& FilePath )
{
	TSharedPtr<IniFile> Ini = FindFileOpened( FilePath );
	if (!Ini)
	{
		Ini = TSharedPtr<IniFile>( new IniFile( ) );
		INIs.Add( Ini );
	}

	return Ini->LoadFile( FilePath );
}

void USimpleINIBPLibrary::CloseIniFile( const FString& FilePath )
{
	for (int i = 0; i < INIs.Num( ); ++i)
	{
		TSharedPtr<IniFile> Ini = INIs[i];
		if (Ini && (Ini->mFilePath == FilePath))
		{
			INIs.RemoveAt( i );
			return;
		}
	}
}

TSharedPtr<IniFile> USimpleINIBPLibrary::FindFileOpened( const FString& FilePath )
{
	for (int i = 0; i < INIs.Num( ); ++i)
	{
		TSharedPtr<IniFile> Ini = INIs[i];
		if (Ini && (Ini->mFilePath == FilePath))
		{
			return Ini;
		}
	}

	return nullptr;
}

