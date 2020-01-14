#include "ini.h"

bool IniFile::LoadFile( const FString& FilePath, bool ClearContent /*= false*/ )
{
	RawLines.Empty( );
	Root = nullptr;

	mFilePath = FilePath;
	if (ClearContent)
	{
		return Parse( );
	}
	else
	{
		return FFileHelper::LoadFileToStringArray( RawLines, *FilePath ) && Parse( );
	}
}

bool IniFile::Parse( )
{
	TSharedPtr<IniLineContext> Context( new IniLineContext( ) );
	Context->RawLines = &RawLines;
	Context->LineNo = 0;

	Root = IniRoot::FromLineString( Context );
	if (Root)
	{
		Root->BuildIndex( );
		return true;
	}
	else
	{
		return false;
	}
}

bool IniFile::Save( )
{
	if (Root && Root->Lines.Num() > 0)
	{
		RawLines.Empty( );
		for (int i = 0; i < Root->Lines.Num( ); ++i)
		{
			TSharedPtr<IniSection>& Section = Root->Lines[i];
			if (Section->SectionName)
			{
				RawLines.Add( Section->SectionName->Raw );
			}

			TSharedPtr<IniSectionContent>& SectionContent = Section->Content;
			if (SectionContent)
			{
				for (int j = 0; j < SectionContent->Entries.Num( ); ++j)
				{
					TSharedPtr<IniSectionContentEntry>& SectionEntry = SectionContent->Entries[j];
					if (SectionEntry)
					{
						switch (SectionEntry->SubType)
						{
						case eWhiteLine:
							RawLines.Add( SectionEntry->Value.WhiteLine->Raw );
							break;
						case eComment:
							RawLines.Add( SectionEntry->Value.Comment->Raw );
							break;
						case eOnlyName:
							RawLines.Add( SectionEntry->Value.OnlyName->Raw );
							break;
						case eNameValuePair:
							RawLines.Add( SectionEntry->Value.NVPair->Raw );
							break;
						default:
							break;
						}
					}
				}
			}
		}
		if (RawLines.Num( ) > 0)
		{
			return FFileHelper::SaveStringArrayToFile( RawLines, *mFilePath );
		}
	}

	return false;
}

bool IniFile::SectionExists( const FString& SectionName ) const
{
	if (Root)
	{
		return Root->SectionIndexLevel.Find( SectionName ) != nullptr;
	}
	else
	{
		return false;
	}
}

bool IniFile::NameExists( const FString& SectionName, const FString& Name ) const
{
	if (Root)
	{
		FString SearchSectionName = SectionName;
		if (SectionName.IsEmpty( ))
		{
			SearchSectionName = TEXT( "##VirtualSection##" );
		}
		TSharedPtr<IniLine>* Val = Root->SectionIndexLevel.Find( SearchSectionName );
		if (Val != nullptr)
		{
			TSharedPtr<IniSection> Section = StaticCastSharedPtr<IniSection>( *Val );
			return Section && (Section->NameIndexLevel.Find( Name ) != nullptr);
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool IniFile::GetValue( const FString& SectionName, const FString& Name, FString& Val, bool& IsValid ) const
{
	IsValid = false;

	if (Root)
	{
		FString SearchSectionName = SectionName;
		if (SectionName.IsEmpty( ))
		{
			SearchSectionName = TEXT( "##VirtualSection##" );
		}
		TSharedPtr<IniLine>* SectionPtr = Root->SectionIndexLevel.Find( SearchSectionName );
		if (SectionPtr != nullptr)
		{
			TSharedPtr<IniSection> Section = StaticCastSharedPtr<IniSection>( *SectionPtr );
			if (Section)
			{
				TSharedPtr<IniLine>* EntryPtr = Section->NameIndexLevel.Find( Name );
				if (EntryPtr)
				{
					TSharedPtr<IniSectionContentEntry> SectionEntry = StaticCastSharedPtr<IniSectionContentEntry>( *EntryPtr );
					if (SectionEntry->SubType == eOnlyName)
					{
						IsValid = false;
						return true;
					}
					else
					{
						IsValid = true;
						Val = SectionEntry->Value.NVPair->Value;
						return true;
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool IniFile::SetValue( const FString& SectionName, const FString& Name, const FString& Val )
{
	if (Root)
	{
		FString SearchSectionName = SectionName;
		if (SectionName.IsEmpty( ))
		{
			SearchSectionName = TEXT( "##VirtualSection##" );
		}
		TSharedPtr<IniLine>* SectionPtr = Root->SectionIndexLevel.Find( SearchSectionName );
		if (SectionPtr != nullptr)
		{
			TSharedPtr<IniSection> Section = StaticCastSharedPtr<IniSection>( *SectionPtr );
			if (Section)
			{
				TSharedPtr<IniLine>* EntryPtr = Section->NameIndexLevel.Find( Name );
				if (EntryPtr)
				{
					TSharedPtr<IniSectionContentEntry> SectionEntry = StaticCastSharedPtr<IniSectionContentEntry>( *EntryPtr );
					if (SectionEntry->SubType == eOnlyName)
					{
						SectionEntry->SubType = eNameValuePair;
						SectionEntry->Value.OnlyName = nullptr;
						SectionEntry->Value.NVPair = TSharedPtr<IniNameValuePair>( new IniNameValuePair( ) );
						SectionEntry->Value.NVPair->Name = Name;
						SectionEntry->Value.NVPair->Value = Val;
						SectionEntry->Value.NVPair->Raw = Name + TEXT( "=" ) + Val;
						SectionEntry->Value.NVPair->BelongTo = SectionEntry;
						return true;
					}
					else
					{
						SectionEntry->Value.NVPair->Value = Val;
						SectionEntry->Value.NVPair->Raw = Name + TEXT( "=" ) + Val;
						return true;
					}
				}
				else
				{
					TSharedPtr<IniSectionContentEntry> SectionEntry( new IniSectionContentEntry( ) );
					SectionEntry->BelongTo = Section;
					SectionEntry->SubType = eNameValuePair;
					SectionEntry->Value.NVPair = TSharedPtr<IniNameValuePair>( new IniNameValuePair( ) );
					SectionEntry->Value.NVPair->Name = Name;
					SectionEntry->Value.NVPair->Value = Val;
					SectionEntry->Value.NVPair->Raw = Name + TEXT( "=" ) + Val;
					SectionEntry->Value.NVPair->BelongTo = SectionEntry;

					// add new item
					Section->Content->Entries.Add( SectionEntry );
					// update index
					Section->NameIndexLevel.Add( Name, StaticCastSharedPtr<IniLine>( SectionEntry ) );
					return true;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			TArray<FString> StrArr;
			if (!SectionName.IsEmpty( ))
			{
				StrArr.Add( TEXT( "[" ) + SectionName + TEXT( "]" ) );
			}
			StrArr.Add( Name + TEXT( "=" ) + Val );

			TSharedPtr<IniLineContext> Context( new IniLineContext( ) );
			Context->RawLines = &StrArr;
			Context->LineNo = 0;

			// Create Section
			TSharedPtr<IniSection> Section = IniSection::FromLineString( Context );
			Root->Lines.Add( Section );

			// Update index
			Section->BuildIndex( );
			if (!Section->SectionName)
			{
				Root->SectionIndexLevel.Add( TEXT( "##VirtualSection##" ), Section );
			}
			else
			{
				Root->SectionIndexLevel.Add( Section->SectionName->Value, Section );
			}
			return true;
		}
	}
	else
	{
		return false;
	}
}

bool IniFile::SetValueAndSave( const FString& SectionName, const FString& Name, const FString& Val )
{
	bool SetResult = SetValue( SectionName, Name, Val );
	if (SetResult)
	{
		return Save( );
	}
	else
	{
		// no need to save because nothing change
		return false;
	}
}

TSharedPtr<IniRoot> IniRoot::FromLineString( TSharedPtr<IniLineContext>& Context )
{
	TSharedPtr<IniRoot> RootIni( new IniRoot( ) );
	if (Context.IsValid( ))
	{
		int& LineNo = Context->LineNo;
		while (LineNo < Context->RawLines->Num( ))
		{
			TSharedPtr<IniSection> Section = IniSection::FromLineString( Context );
			if (!Section.IsValid())
			{
				return nullptr;
			}
			RootIni->Lines.Add( Section );
		}
		return MoveTemp( RootIni );
	}
	else
	{
		return nullptr;
	}
}

void IniRoot::BuildIndex( )
{
	for (int i = 0; i < Lines.Num( ); ++i)
	{
		TSharedPtr<IniSection>& Section = Lines[i];
		if (Section && Section->Content)
		{
			Section->BuildIndex( );
			if (!Section->SectionName)
			{
				SectionIndexLevel.Add( TEXT( "##VirtualSection##" ), Section );
			}
			else
			{
				SectionIndexLevel.Add( Section->SectionName->Value, Section );
			}
		}
	}
}

TSharedPtr<IniSection> IniSection::FromLineString( TSharedPtr<IniLineContext>& Context )
{
	TSharedPtr<IniSection> Section( new IniSection( ) );

	const FString& LineString = (*Context->RawLines)[Context->LineNo];
	FString TrimedString = LineString.TrimStartAndEnd( );
	if (LineString.IsEmpty( ) || 
		TrimedString.IsEmpty( ) ||
		TrimedString[0] != TEXT('['))
	{
		Section->IsVirtual = true;
		Context->Value.push_back( StaticCastSharedPtr<IniLine>( Section ) );

		Section->SectionName = nullptr;
		Section->Content = IniSectionContent::FromLineString( Context );

		Context->Value.pop_back( );
	}
	else
	{ 
		Section->IsVirtual = false;
		Context->Value.push_back( StaticCastSharedPtr<IniLine>( Section ) );

		Section->SectionName = IniSectionName::FromLineString( Context );
		Section->Content = IniSectionContent::FromLineString( Context );

		Context->Value.pop_back( );
	}

	return MoveTemp( Section );
}


void IniSection::BuildIndex( )
{
	if (Content.IsValid( ))
	{
		for (int i = 0; i < Content->Entries.Num(); ++i)
		{
			TSharedPtr<IniSectionContentEntry>& Entry = Content->Entries[i];
			if (Entry.IsValid( ))
			{
				if (Entry->SubType == eOnlyName && Entry->Value.OnlyName.IsValid( ))
				{
					NameIndexLevel.Add( Entry->Value.OnlyName->Value );
				}
				else if (Entry->SubType == eNameValuePair && Entry->Value.NVPair.IsValid( ))
				{
					NameIndexLevel.Add( Entry->Value.NVPair->Name, StaticCastSharedPtr<IniLine>( Entry ) );
				}
			}
		}
	}
}

TSharedPtr<IniSectionName> IniSectionName::FromLineString( TSharedPtr<IniLineContext>& Context )
{
	TSharedPtr<IniSectionName> SectionName( new IniSectionName( ) );
	SectionName->BelongTo = StaticCastSharedPtr<IniSection>( Context->Value.back( ) );
	SectionName->Raw = (*Context->RawLines)[Context->LineNo++];
	SectionName->Value = SectionName->Raw.TrimStartAndEnd( );
	SectionName->Value = SectionName->Value.Mid( 1, SectionName->Value.Len( ) - 2 ).TrimStartAndEnd( );
	return MoveTemp( SectionName );
}

TSharedPtr<IniSectionContent> IniSectionContent::FromLineString( TSharedPtr<IniLineContext>& Context )
{
	TSharedPtr<IniSectionContent> SectionContent( new IniSectionContent( ) );
	SectionContent->BelongTo = StaticCastSharedPtr<IniSection>( Context->Value.back( ) );

	while (Context->LineNo < Context->RawLines->Num( ))
	{
		TSharedPtr<IniSectionContentEntry> ContentEntry( new IniSectionContentEntry( ) );
		ContentEntry->BelongTo = StaticCastSharedPtr<IniSection>( Context->Value.back( ) );
		Context->Value.push_back( StaticCastSharedPtr<IniLine>( ContentEntry ) );

		const FString& LineString = (*Context->RawLines)[Context->LineNo];
		FString TrimedString = LineString.TrimStartAndEnd( );

		if (TrimedString.IsEmpty( ))
		{
			ContentEntry->SubType = eWhiteLine;
			ContentEntry->Value.WhiteLine = IniWhiteLine::FromLineString( Context );
		}
		else if (TrimedString[0] == TEXT( '#' )
			|| TrimedString.Find( TEXT( "//" ) ) == 0)
		{
			// comment starts with # or //
			ContentEntry->SubType = eComment;
			ContentEntry->Value.Comment = IniComment::FromLineString( Context );
		}
		else if (TrimedString.Find( TEXT( "=" ) ) > 0)
		{
			ContentEntry->SubType = eNameValuePair;
			ContentEntry->Value.NVPair = IniNameValuePair::FromLineString( Context );
		}
		else if (TrimedString[0] != TEXT( '[' ))
		{
			ContentEntry->SubType = eOnlyName;
			ContentEntry->Value.OnlyName = IniOnlyName::FromLineString( Context );
		}
		else
		{
			Context->Value.pop_back( ); // pop ContentEntry
			return MoveTemp( SectionContent );
		}

		Context->Value.pop_back( ); // pop ContentEntry
		SectionContent->Entries.Add( ContentEntry );
	}

	return MoveTemp( SectionContent );
}

TSharedPtr<IniWhiteLine> IniWhiteLine::FromLineString( TSharedPtr<IniLineContext>& Context )
{
	TSharedPtr<IniWhiteLine> WhiteLine( new IniWhiteLine( ) );
	WhiteLine->BelongTo = StaticCastSharedPtr<IniSectionContentEntry>( Context->Value.back( ) );
	WhiteLine->Raw = (*Context->RawLines)[Context->LineNo++];
	return MoveTemp( WhiteLine );
}

TSharedPtr<IniComment> IniComment::FromLineString( TSharedPtr<IniLineContext>& Context )
{
	TSharedPtr<IniComment> Comment( new IniComment( ) );
	Comment->BelongTo = StaticCastSharedPtr<IniSectionContentEntry>( Context->Value.back( ) );

	const FString& LineString = (*Context->RawLines)[Context->LineNo++];
	FString TrimedString = LineString.TrimStartAndEnd( );

	Comment->Raw = LineString;
	if (TrimedString[0] == TEXT( '#' ))
	{
		Comment->Value = TrimedString.Mid( 1 ).TrimStart( );
	}
	else
	{
		Comment->Value = TrimedString.Mid( 2 ).TrimStart( );
	}

	return MoveTemp( Comment );
}

TSharedPtr<IniOnlyName> IniOnlyName::FromLineString( TSharedPtr<IniLineContext>& Context )
{
	TSharedPtr<IniOnlyName> OnlyName( new IniOnlyName( ) );
	OnlyName->BelongTo = StaticCastSharedPtr<IniSectionContentEntry>( Context->Value.back( ) );

	const FString& LineString = (*Context->RawLines)[Context->LineNo++];
	FString TrimedString = LineString.TrimStartAndEnd( );

	OnlyName->Raw = LineString;
	OnlyName->Value = TrimedString;

	return MoveTemp( OnlyName );
}

TSharedPtr<IniNameValuePair> IniNameValuePair::FromLineString( TSharedPtr<IniLineContext>& Context )
{
	TSharedPtr<IniNameValuePair> NVPair( new IniNameValuePair( ) );
	NVPair->BelongTo = StaticCastSharedPtr<IniSectionContentEntry>( Context->Value.back( ) );

	const FString& LineString = (*Context->RawLines)[Context->LineNo++];
	FString TrimedString = LineString.TrimStartAndEnd( );

	NVPair->Raw = LineString;

	int Index = TrimedString.Find( TEXT( "=" ) );
	NVPair->Name = TrimedString.Mid( 0, Index ).TrimEnd( );

	if (Index + 1 == TrimedString.Len( ))
	{
		NVPair->Value = TEXT( "" );
	}
	else
	{
		NVPair->Value = TrimedString.Mid( Index + 1 ).TrimStart( );
	}

	return MoveTemp( NVPair );
}
