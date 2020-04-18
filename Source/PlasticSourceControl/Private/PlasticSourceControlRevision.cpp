// Copyright (c) 2016-2020 Codice Software

#include "PlasticSourceControlRevision.h"
#include "PlasticSourceControlModule.h"
#include "PlasticSourceControlProvider.h"
#include "PlasticSourceControlState.h"
#include "PlasticSourceControlUtils.h"
#include "SPlasticSourceControlSettings.h"

#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "ISourceControlModule.h"

#define LOCTEXT_NAMESPACE "PlasticSourceControl"

bool FPlasticSourceControlRevision::Get( FString& InOutFilename ) const
{
	const FPlasticSourceControlModule& PlasticSourceControl = FModuleManager::LoadModuleChecked<FPlasticSourceControlModule>("PlasticSourceControl");
	const FString& PathToPlasticBinary = PlasticSourceControl.AccessSettings().GetBinaryPath();
	if (!State)
	{
		UE_LOG(LogSourceControl, Error, TEXT("Revision(%s %d): unknown state!"), *Filename, RevisionId);
	}

	// if a filename for the temp file wasn't supplied generate a unique-ish one
	if(InOutFilename.Len() == 0)
	{
		// create the diff dir if we don't already have it (Plastic wont)
		IFileManager::Get().MakeDirectory(*FPaths::DiffDir(), true);
		// create a unique temp file name based on the unique revision Id
		const FString TempFileName = FString::Printf(TEXT("%stemp-%d-%s"), *FPaths::DiffDir(), ChangesetNumber, *FPaths::GetCleanFilename(Filename));
		InOutFilename = FPaths::ConvertRelativePathToFull(TempFileName);
	}

	bool bCommandSuccessful;
	if(FPaths::FileExists(InOutFilename))
	{
		bCommandSuccessful = true; // if the temp file already exists, reuse it directly
	}
	else if (State)
	{
		// Format the revision specification of the file, like rev:Content/BP.uasset#cs:12@repo@server:8087
		const FString RevisionSpecification = FString::Printf(TEXT("rev:%s#cs:%d@%s"), *Filename, ChangesetNumber, *State->RepSpec);
		bCommandSuccessful = PlasticSourceControlUtils::RunDumpToFile(PathToPlasticBinary, RevisionSpecification, InOutFilename);
	}
	else
	{
		bCommandSuccessful = false;
	}
	return bCommandSuccessful;
}

bool FPlasticSourceControlRevision::GetAnnotated( TArray<FAnnotationLine>& OutLines ) const
{
	// NOTE GetAnnotated: called only by SourceControlHelpers::AnnotateFile(),
	//      called only by ICrashDebugHelper::AddAnnotatedSourceToReport() using a changelist/check identifier
	//      called only by FCrashDebugHelperWindows::CreateMinidumpDiagnosticReport() (and Mac) to Extract annotated lines from a source file stored in Perforce, and add to the crash report.
	//      called by - MinidumpDiagnosticsApp RunMinidumpDiagnostics() for Perfore ONLY "MinidumpDiagnostics.exe <Crash.dmp> [-Annotate] [-SyncSymbols] [-SyncMicrosoftSymbols]"
	//                - FWindowsErrorReport::DiagnoseReport() (and Mac)
	// Reserved for internal use by Epic Games with Perforce only
	return false;
}

bool FPlasticSourceControlRevision::GetAnnotated( FString& InOutFilename ) const
{
	// NOTE: Unused, only the above method is called by the Editor
	return false;
}

const FString& FPlasticSourceControlRevision::GetFilename() const
{
	return Filename;
}

int32 FPlasticSourceControlRevision::GetRevisionNumber() const
{
	return ChangesetNumber; // Using the Changelist as the Revision number to display in the Asset Diff Menu
}

const FString& FPlasticSourceControlRevision::GetRevision() const
{
	return Revision;
}

const FString& FPlasticSourceControlRevision::GetDescription() const
{
	return Description;
}

const FString& FPlasticSourceControlRevision::GetUserName() const
{
	return UserName;
}

const FString& FPlasticSourceControlRevision::GetClientSpec() const
{
	static FString EmptyString(TEXT("")); // NOTE Workspace/Clientspec of the submitter (Perforce only)
	return EmptyString;
}

const FString& FPlasticSourceControlRevision::GetAction() const
{
	return Action;
}

TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FPlasticSourceControlRevision::GetBranchSource() const
{
	// if this revision was copied/moved from some other revision
	return BranchSource;
}

const FDateTime& FPlasticSourceControlRevision::GetDate() const
{
	return Date;
}

int32 FPlasticSourceControlRevision::GetCheckInIdentifier() const
{
	return ChangesetNumber;
}

int32 FPlasticSourceControlRevision::GetFileSize() const
{
	return FileSize;
}

#undef LOCTEXT_NAMESPACE
