// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "RiderSourceCodeAccessor.h"

#include "RiderPathLocator.h"

#include "ISourceCodeAccessModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformTime.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#endif
#include "IHotReload.h"
#include "DesktopPlatformModule.h"

#define LOCTEXT_NAMESPACE "RiderSourceCodeAccessor"

DEFINE_LOG_CATEGORY_STATIC(LogRiderAccessor, Log, All);


/** save all open documents in visual studio, when recompiling */
static void OnModuleCompileStarted(bool bIsAsyncCompile)
{
	ISourceCodeAccessModule& SourceCodeAccessModule = FModuleManager::LoadModuleChecked<ISourceCodeAccessModule>("SourceCodeAccess");
	SourceCodeAccessModule.GetAccessor().SaveAllOpenDocuments();
}

void FRiderSourceCodeAccessor::RefreshAvailability()
{
	// If we have an executable path, we certainly have it installed!
	bHasRiderInstalled = !ExecutablePath.IsEmpty() && FPaths::FileExists(ExecutablePath);
}

bool FRiderSourceCodeAccessor::AddSourceFiles(const TArray<FString>& AbsoluteSourcePaths, const TArray<FString>& AvailableModules)
{
	// @todo.Rider Manually add to folders? Or just regenerate
	return false;
}

bool FRiderSourceCodeAccessor::CanAccessSourceCode() const
{
	return bHasRiderInstalled;
}

bool FRiderSourceCodeAccessor::DoesSolutionExist() const
{
	FString SolutionPath;
	return FDesktopPlatformModule::Get()->GetSolutionPath(SolutionPath);
}

FText FRiderSourceCodeAccessor::GetDescriptionText() const
{
	return LOCTEXT("RiderDisplayDesc", "Open source code files in Rider");
}

FName FRiderSourceCodeAccessor::GetFName() const
{
	return RiderName;
}

FText FRiderSourceCodeAccessor::GetNameText() const
{
	return FText::FromName(RiderName);
}

bool FRiderSourceCodeAccessor::OpenFileAtLine(const FString& FullPath, int32 LineNumber, int32)
{
	if(!bHasRiderInstalled) return false;
	FString SolutionPath;
	if(!FDesktopPlatformModule::Get()->GetSolutionPath(SolutionPath)) return false;

	FString Path = FullPath;
	if(FPaths::IsRelative(FullPath))
	{
		Path = FString::Printf(TEXT("\"%s\" --line %d \"%s\""), *FPaths::ConvertRelativePathToFull(*FPaths::ProjectDir()), LineNumber, *Path);
	}

	const FString Params = FString::Printf(TEXT("\"%s\" %s"), *SolutionPath, *Path);

	FProcHandle Proc = FPlatformProcess::CreateProc(*ExecutablePath, *Params, true, true, false, nullptr, 0, nullptr, nullptr);
	if (!Proc.IsValid())
	{
		UE_LOG(LogRiderAccessor, Warning, TEXT("Opening file (%s) at a specific line failed."), *Path);
		FPlatformProcess::CloseProc(Proc);
		return false;
	}

	return true;
}

bool FRiderSourceCodeAccessor::OpenSolution()
{
	if(!bHasRiderInstalled) return false;

	FString SolutionPath;
	if(FDesktopPlatformModule::Get()->GetSolutionPath(SolutionPath))
	{
		const FString FullPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead( *SolutionPath );
		FPlatformProcess::CreateProc(*ExecutablePath, *FullPath, true, true, false, nullptr, 0, nullptr, nullptr);
		return true;
	}
	
	return false;
}

bool FRiderSourceCodeAccessor::OpenSolutionAtPath(const FString& InSolutionPath)
{
	if(!bHasRiderInstalled) return false;
	
	FString CorrectSolutionPath = InSolutionPath;
	if(!CorrectSolutionPath.EndsWith(".sln"))
	{
		CorrectSolutionPath += ".sln";
	}
	FProcHandle Proc = FPlatformProcess::CreateProc(*ExecutablePath, *CorrectSolutionPath, true, true, false, nullptr, 0, nullptr, nullptr);
	if(!Proc.IsValid())
	{
		UE_LOG(LogRiderAccessor, Warning, TEXT("Opening the project file (%s) failed."), *CorrectSolutionPath);
		FPlatformProcess::CloseProc(Proc);
		return false;
	}
	return true;
}

bool FRiderSourceCodeAccessor::OpenSourceFiles(const TArray<FString>& AbsoluteSourcePaths)
{
	if(!bHasRiderInstalled) return false;
	FString SolutionPath;
	if(!FDesktopPlatformModule::Get()->GetSolutionPath(SolutionPath)) return false;

	FString SourceFilesList = "";

	// Build our paths based on what unreal sends to be opened
	for (const FString& SourcePath : AbsoluteSourcePaths)
	{
		SourceFilesList += FString::Printf(TEXT("\"%s\" "), *SourcePath);
	}

	// Trim any whitespace on our source file list
	SourceFilesList.TrimStartInline();
	SourceFilesList.TrimEndInline();

	const FString Params = FString::Printf(TEXT("\"%s\" %s"), *SolutionPath, *SourceFilesList);

	FProcHandle Proc = FPlatformProcess::CreateProc(*ExecutablePath, *SourceFilesList, true, false, false, nullptr, 0, nullptr, nullptr);
	if (!Proc.IsValid())
	{
		UE_LOG(LogRiderAccessor, Warning, TEXT("Opening the source file (%s) failed."), *SourceFilesList);
		FPlatformProcess::CloseProc(Proc);
		return false;
	}

	return true;
}

bool FRiderSourceCodeAccessor::SaveAllOpenDocuments() const
{
	return false;
}

void FRiderSourceCodeAccessor::Startup(const FRiderPathLocator::FInstallInfo & Info)
{
	ExecutablePath = Info.Path;
	const FString IsToolboxText = Info.IsToolbox ? TEXT("(toolbox)") : TEXT("(installed)");
	RiderName = *FString::Format(TEXT("Rider {0} {1}"), { Info.Version, IsToolboxText });
#if WITH_EDITOR
	// Setup compilation for saving all VS documents upon compilation start
	BlockEditingInRiderDocumentsDelegateHandle = IHotReloadModule::Get().OnModuleCompilerStarted().AddStatic( &OnModuleCompileStarted );
#endif

	RefreshAvailability();
}

#undef LOCTEXT_NAMESPACE
