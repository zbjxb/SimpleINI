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

bool USimpleINIBPLibrary::LoadIniFile( const FString& FilePath )
{
	TSharedPtr<IniFile> Ini = FindFileOpened( FilePath );
	if (!Ini)
	{
		Ini = TSharedPtr<IniFile>( new IniFile( ) );
		INIs.Add( Ini );
	}
	return Ini->LoadFile( FilePath );
}

bool USimpleINIBPLibrary::GetValue( const FString& FilePath, const FString& SectionName, const FString& Key, FString& Value, bool& IsValid )
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

	return Ini->GetValue( SectionName, Key, Value, IsValid );
}

bool USimpleINIBPLibrary::SetValue( const FString& FilePath, const FString& SectionName, const FString& Key, const FString& Value )
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

	return Ini->SetValue( SectionName, Key, Value );
}

bool USimpleINIBPLibrary::SaveIniFile( const FString& FilePath )
{
	TSharedPtr<IniFile> Ini = FindFileOpened( FilePath );
	if (!Ini)
	{
		return false;
	}

	return Ini->Save( );
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

