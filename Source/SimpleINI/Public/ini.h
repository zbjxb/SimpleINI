#pragma once
#include "CoreMinimal.h"
#include <vector>


// ini structure modeling

enum LineType
{
	eComment,
	eSection,
	eSectionName,
	eSectionContent,
	eSectionContentEntry,
	eOnlyName,
	eNameValuePair,
	eWhiteLine,
	eRoot,
};

struct IniLine;
struct IniSection;
struct IniSectionContentEntry;
using IndexLevel = TMap<FString, TSharedPtr<IniLine>>;

struct IniLineContext
{
	TArray<FString>* RawLines;
	int LineNo;

	std::vector<TSharedPtr<IniLine>> Value;
};

struct IniLine
{
	LineType Type;
	FString Raw;

	IniLine( LineType T )
		: Type( T )
	{
	}
	virtual ~IniLine( ) {}
};

struct IniComment : public IniLine
{
	TSharedPtr<IniSectionContentEntry> BelongTo;

	FString Value;

	IniComment( )
		: IniLine( eComment )
	{
	}
	~IniComment( ) {}
	static TSharedPtr<IniComment> FromLineString( TSharedPtr<IniLineContext>& Context );
};
struct IniWhiteLine : public IniLine
{
	TSharedPtr<IniSectionContentEntry> BelongTo;

	IniWhiteLine( )
		: IniLine( eWhiteLine )
	{
	}
	~IniWhiteLine( ) {}
	static TSharedPtr<IniWhiteLine> FromLineString( TSharedPtr<IniLineContext>& Context );
};
struct IniOnlyName :public IniLine
{
	TSharedPtr<IniSectionContentEntry> BelongTo;

	FString Value;

	IniOnlyName( )
		: IniLine( eOnlyName )
	{
	}
	~IniOnlyName( ) {}
	static TSharedPtr<IniOnlyName> FromLineString( TSharedPtr<IniLineContext>& Context );
};
struct IniNameValuePair : public IniLine
{
	TSharedPtr<IniSectionContentEntry> BelongTo;

	FString Name;
	FString Value;

	IniNameValuePair( )
		: IniLine( eNameValuePair )
	{
	}
	~IniNameValuePair( ) {}
	static TSharedPtr<IniNameValuePair> FromLineString( TSharedPtr<IniLineContext>& Context );
};
struct IniSectionName : public IniLine
{
	TSharedPtr<IniSection> BelongTo;

	FString Value;

	IniSectionName( )
		: IniLine( eSectionName )
	{
	}
	~IniSectionName( ) {}
	static TSharedPtr<IniSectionName> FromLineString( TSharedPtr<IniLineContext>& Context );
};
struct IniSectionContentEntry : public IniLine
{
	TSharedPtr<IniSection> BelongTo;
	struct U
	{
		TSharedPtr<IniNameValuePair> NVPair;
		TSharedPtr<IniComment> Comment;
		TSharedPtr<IniWhiteLine> WhiteLine;
		TSharedPtr<IniOnlyName> OnlyName;
	} Value;
	LineType SubType;

	IniSectionContentEntry( )
		: IniLine( eSectionContentEntry )
	{
	}
	~IniSectionContentEntry( ) {}
	//static TSharedPtr<IniLine> FromLineString( );
};
struct IniSectionContent :public IniLine
{
	TSharedPtr<IniSection> BelongTo;

	TArray<TSharedPtr<IniSectionContentEntry>> Entries;

	IniSectionContent( )
		:IniLine( eSectionContent )
	{
	}
	~IniSectionContent( ) {}
	static TSharedPtr<IniSectionContent> FromLineString( TSharedPtr<IniLineContext>& Context );
};
struct IniSection :public IniLine
{
	bool IsVirtual;
	TSharedPtr<IniSectionName> SectionName;
	TSharedPtr<IniSectionContent> Content;

	IndexLevel NameIndexLevel;

	IniSection( )
		:IniLine( eSection )
		,IsVirtual( false )
	{
	}
	~IniSection( ) {}
	static TSharedPtr<IniSection> FromLineString( TSharedPtr<IniLineContext>& Context );

	void BuildIndex( );
};
struct IniRoot :public IniLine
{
	TArray<TSharedPtr<IniSection>> Lines;
	IndexLevel SectionIndexLevel;

	IniRoot( )
		:IniLine( eRoot )
	{
	}
	~IniRoot( ) {}
	static TSharedPtr<IniRoot> FromLineString( TSharedPtr<IniLineContext>& Context );

	void BuildIndex( );
};

class IniFile
{
public:
	IniFile( ){}
	virtual ~IniFile( ) {};

	bool LoadFile( const FString& FilePath, bool ClearContent = false );
	bool Save( );

	bool SectionExists( const FString& SectionName ) const;
	bool NameExists( const FString& SectionName, const FString& Name ) const;

	bool GetValue( const FString& SectionName, const FString& Name, FString& Val, bool& IsValid ) const;
	bool SetValue( const FString& SectionName, const FString& Name, const FString& Val );
	bool SetValueAndSave( const FString& SectionName, const FString& Name, const FString& Val );

public:
	FString mFilePath;

protected:
	bool Parse( );

private:
	TArray<FString> RawLines;
	TSharedPtr<IniRoot> Root;
};

