// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MovieSceneToolsModule.h"

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Curves/RichCurve.h"
#include "ISequencerModule.h"
#include "ICurveEditorModule.h"
#include "MovieSceneToolsProjectSettingsCustomization.h"

#include "TrackEditors/PropertyTrackEditors/BoolPropertyTrackEditor.h"
#include "TrackEditors/PropertyTrackEditors/BytePropertyTrackEditor.h"
#include "TrackEditors/PropertyTrackEditors/ColorPropertyTrackEditor.h"
#include "TrackEditors/PropertyTrackEditors/FloatPropertyTrackEditor.h"
#include "TrackEditors/PropertyTrackEditors/IntegerPropertyTrackEditor.h"
#include "TrackEditors/PropertyTrackEditors/VectorPropertyTrackEditor.h"
#include "TrackEditors/PropertyTrackEditors/TransformPropertyTrackEditor.h"
#include "TrackEditors/PropertyTrackEditors/EulerTransformPropertyTrackEditor.h"
#include "TrackEditors/PropertyTrackEditors/VisibilityPropertyTrackEditor.h"
#include "TrackEditors/PropertyTrackEditors/ActorReferencePropertyTrackEditor.h"
#include "TrackEditors/PropertyTrackEditors/StringPropertyTrackEditor.h"

#include "TrackEditors/TransformTrackEditor.h"
#include "TrackEditors/CameraCutTrackEditor.h"
#include "TrackEditors/CinematicShotTrackEditor.h"
#include "TrackEditors/SlomoTrackEditor.h"
#include "TrackEditors/SubTrackEditor.h"
#include "TrackEditors/AudioTrackEditor.h"
#include "TrackEditors/SkeletalAnimationTrackEditor.h"
#include "TrackEditors/ParticleTrackEditor.h"
#include "TrackEditors/ParticleParameterTrackEditor.h"
#include "TrackEditors/AttachTrackEditor.h"
#include "TrackEditors/EventTrackEditor.h"
#include "TrackEditors/PathTrackEditor.h"
#include "TrackEditors/MaterialTrackEditor.h"
#include "TrackEditors/FadeTrackEditor.h"
#include "TrackEditors/SpawnTrackEditor.h"
#include "TrackEditors/LevelVisibilityTrackEditor.h"
#include "TrackEditors/CameraAnimTrackEditor.h"
#include "TrackEditors/CameraShakeTrackEditor.h"
#include "TrackEditors/MaterialParameterCollectionTrackEditor.h"
#include "TrackEditors/ObjectPropertyTrackEditor.h"
#include "TrackEditors/PrimitiveMaterialTrackEditor.h"

#include "MovieSceneBuiltInEasingFunctionCustomization.h"
#include "MovieSceneObjectBindingIDCustomization.h"
#include "MovieSceneEventCustomization.h"
#include "SequencerClipboardReconciler.h"
#include "ClipboardTypes.h"
#include "ISettingsModule.h"
#include "PropertyEditorModule.h"
#include "IMovieSceneTools.h"
#include "MovieSceneToolsProjectSettings.h"

#include "ISequencerChannelInterface.h"
#include "SequencerChannelInterface.h"
#include "Channels/BuiltInChannelEditors.h"
#include "Channels/MovieSceneObjectPathChannel.h"
#include "Channels/MovieSceneEventChannel.h"
#include "Channels/EventChannelCurveModel.h"
#include "Channels/SCurveEditorEventChannelView.h"
#include "Sections/MovieSceneEventSection.h"


#define LOCTEXT_NAMESPACE "FMovieSceneToolsModule"

void FMovieSceneToolsModule::StartupModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Editor", "Level Sequences",
			LOCTEXT("RuntimeSettingsName", "Level Sequences"),
			LOCTEXT("RuntimeSettingsDescription", "Configure project settings relating to Level Sequences"),
			GetMutableDefault<UMovieSceneToolsProjectSettings>()
		);
	}

	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>( "Sequencer" );

	// register property track editors
	BoolPropertyTrackCreateEditorHandle = SequencerModule.RegisterPropertyTrackEditor<FBoolPropertyTrackEditor>();
	BytePropertyTrackCreateEditorHandle = SequencerModule.RegisterPropertyTrackEditor<FBytePropertyTrackEditor>();
	ColorPropertyTrackCreateEditorHandle = SequencerModule.RegisterPropertyTrackEditor<FColorPropertyTrackEditor>();
	FloatPropertyTrackCreateEditorHandle = SequencerModule.RegisterPropertyTrackEditor<FFloatPropertyTrackEditor>();
	IntegerPropertyTrackCreateEditorHandle = SequencerModule.RegisterPropertyTrackEditor<FIntegerPropertyTrackEditor>();
	VectorPropertyTrackCreateEditorHandle = SequencerModule.RegisterPropertyTrackEditor<FVectorPropertyTrackEditor>();
	TransformPropertyTrackCreateEditorHandle = SequencerModule.RegisterPropertyTrackEditor<FTransformPropertyTrackEditor>();
	EulerTransformPropertyTrackCreateEditorHandle = SequencerModule.RegisterPropertyTrackEditor<FEulerTransformPropertyTrackEditor>();
	VisibilityPropertyTrackCreateEditorHandle = SequencerModule.RegisterPropertyTrackEditor<FVisibilityPropertyTrackEditor>();
	ActorReferencePropertyTrackCreateEditorHandle = SequencerModule.RegisterPropertyTrackEditor<FActorReferencePropertyTrackEditor>();
	StringPropertyTrackCreateEditorHandle = SequencerModule.RegisterPropertyTrackEditor<FStringPropertyTrackEditor>();
	ObjectTrackCreateEditorHandle = SequencerModule.RegisterPropertyTrackEditor<FObjectPropertyTrackEditor>();

	// register specialty track editors
	AnimationTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &FSkeletalAnimationTrackEditor::CreateTrackEditor ) );
	AttachTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &F3DAttachTrackEditor::CreateTrackEditor ) );
	AudioTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &FAudioTrackEditor::CreateTrackEditor ) );
	EventTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &FEventTrackEditor::CreateTrackEditor ) );
	ParticleTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &FParticleTrackEditor::CreateTrackEditor ) );
	ParticleParameterTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &FParticleParameterTrackEditor::CreateTrackEditor ) );
	PathTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &F3DPathTrackEditor::CreateTrackEditor ) );
	CameraCutTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &FCameraCutTrackEditor::CreateTrackEditor ) );
	CinematicShotTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &FCinematicShotTrackEditor::CreateTrackEditor ) );
	SlomoTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &FSlomoTrackEditor::CreateTrackEditor ) );
	SubTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &FSubTrackEditor::CreateTrackEditor ) );
	TransformTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &F3DTransformTrackEditor::CreateTrackEditor ) );
	ComponentMaterialTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &FComponentMaterialTrackEditor::CreateTrackEditor ) );
	FadeTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &FFadeTrackEditor::CreateTrackEditor ) );
	SpawnTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &FSpawnTrackEditor::CreateTrackEditor ) );
	LevelVisibilityTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor( FOnCreateTrackEditor::CreateStatic( &FLevelVisibilityTrackEditor::CreateTrackEditor ) );
	CameraAnimTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FCameraAnimTrackEditor::CreateTrackEditor));
	CameraShakeTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FCameraShakeTrackEditor::CreateTrackEditor));
	MPCTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FMaterialParameterCollectionTrackEditor::CreateTrackEditor));
	PrimitiveMaterialCreateEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FPrimitiveMaterialTrackEditor::CreateTrackEditor));

	RegisterClipboardConversions();

	// register details customization
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("MovieSceneToolsProjectSettings", FOnGetDetailCustomizationInstance::CreateStatic(&FMovieSceneToolsProjectSettingsCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout("MovieSceneBuiltInEasingFunction", FOnGetDetailCustomizationInstance::CreateLambda(&MakeShared<FMovieSceneBuiltInEasingFunctionCustomization>));
	PropertyModule.RegisterCustomPropertyTypeLayout("MovieSceneObjectBindingID", FOnGetPropertyTypeCustomizationInstance::CreateLambda(&MakeShared<FMovieSceneObjectBindingIDCustomization>));
	PropertyModule.RegisterCustomPropertyTypeLayout("MovieSceneEvent", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FMovieSceneEventCustomization::MakeInstance));

	SequencerModule.RegisterChannelInterface<FMovieSceneBoolChannel>();
	SequencerModule.RegisterChannelInterface<FMovieSceneByteChannel>();
	SequencerModule.RegisterChannelInterface<FMovieSceneIntegerChannel>();
	SequencerModule.RegisterChannelInterface<FMovieSceneFloatChannel>();
	SequencerModule.RegisterChannelInterface<FMovieSceneStringChannel>();
	SequencerModule.RegisterChannelInterface<FMovieSceneParticleChannel>();
	SequencerModule.RegisterChannelInterface<FMovieSceneActorReferenceData>();
	SequencerModule.RegisterChannelInterface<FMovieSceneEventSectionData>();
	SequencerModule.RegisterChannelInterface<FMovieSceneObjectPathChannel>();

	SequencerModule.RegisterChannelInterface<FMovieSceneEventChannel>();

	ICurveEditorModule& CurveEditorModule = FModuleManager::LoadModuleChecked<ICurveEditorModule>("CurveEditor");

	FEventChannelCurveModel::EventView = CurveEditorModule.RegisterView(FOnCreateCurveEditorView::CreateStatic(
		[](TWeakPtr<FCurveEditor> WeakCurveEditor) -> TSharedRef<SCurveEditorView>
		{
			return SNew(SCurveEditorEventChannelView, WeakCurveEditor);
		}
	));
}

void FMovieSceneToolsModule::ShutdownModule()
{
	if (ICurveEditorModule* CurveEditorModule = FModuleManager::GetModulePtr<ICurveEditorModule>("CurveEditor"))
	{
		CurveEditorModule->UnregisterView(FEventChannelCurveModel::EventView);
	}

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Editor", "Level Sequences");
	}

	if (!FModuleManager::Get().IsModuleLoaded("Sequencer"))
	{
		return;
	}

	ISequencerModule& SequencerModule = FModuleManager::Get().GetModuleChecked<ISequencerModule>( "Sequencer" );

	// unregister property track editors
	SequencerModule.UnRegisterTrackEditor( BoolPropertyTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( BytePropertyTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( ColorPropertyTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( FloatPropertyTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( IntegerPropertyTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( VectorPropertyTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( TransformPropertyTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( EulerTransformPropertyTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( VisibilityPropertyTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( ActorReferencePropertyTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( StringPropertyTrackCreateEditorHandle );

	// unregister specialty track editors
	SequencerModule.UnRegisterTrackEditor( AnimationTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( AttachTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( AudioTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( EventTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( ParticleTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( ParticleParameterTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( PathTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( CameraCutTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( CinematicShotTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( SlomoTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( SubTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( TransformTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( ComponentMaterialTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( FadeTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( SpawnTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( LevelVisibilityTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( CameraAnimTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( CameraShakeTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( MPCTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( ObjectTrackCreateEditorHandle );
	SequencerModule.UnRegisterTrackEditor( PrimitiveMaterialCreateEditorHandle );

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{	
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("MovieSceneToolsProjectSettings");
		PropertyModule.UnregisterCustomClassLayout("MovieSceneBuiltInEasingFunction");
		PropertyModule.UnregisterCustomPropertyTypeLayout("MovieSceneObjectBindingID");
		PropertyModule.UnregisterCustomPropertyTypeLayout("MovieSceneEvent");
	}
}

void FMovieSceneToolsModule::RegisterClipboardConversions()
{
	using namespace MovieSceneClipboard;

	DefineImplicitConversion<int32, uint8>();
	DefineImplicitConversion<int32, bool>();

	DefineImplicitConversion<uint8, int32>();
	DefineImplicitConversion<uint8, bool>();

	DefineExplicitConversion<int32, FMovieSceneFloatValue>([](const int32& In) -> FMovieSceneFloatValue { return FMovieSceneFloatValue(In);	});
	DefineExplicitConversion<uint8, FMovieSceneFloatValue>([](const uint8& In) -> FMovieSceneFloatValue { return FMovieSceneFloatValue(In);	});
	DefineExplicitConversion<FMovieSceneFloatValue, int32>([](const FMovieSceneFloatValue& In) -> int32 { return In.Value; 					});
	DefineExplicitConversion<FMovieSceneFloatValue, uint8>([](const FMovieSceneFloatValue& In) -> uint8 { return In.Value; 					});
	DefineExplicitConversion<FMovieSceneFloatValue, bool>([](const FMovieSceneFloatValue& In) -> bool	{ return !!In.Value; 				});

	FSequencerClipboardReconciler::AddTrackAlias("Location.X", "R");
	FSequencerClipboardReconciler::AddTrackAlias("Location.Y", "G");
	FSequencerClipboardReconciler::AddTrackAlias("Location.Z", "B");

	FSequencerClipboardReconciler::AddTrackAlias("Rotation.X", "R");
	FSequencerClipboardReconciler::AddTrackAlias("Rotation.Y", "G");
	FSequencerClipboardReconciler::AddTrackAlias("Rotation.Z", "B");

	FSequencerClipboardReconciler::AddTrackAlias("Scale.X", "R");
	FSequencerClipboardReconciler::AddTrackAlias("Scale.Y", "G");
	FSequencerClipboardReconciler::AddTrackAlias("Scale.Z", "B");

	FSequencerClipboardReconciler::AddTrackAlias("X", "R");
	FSequencerClipboardReconciler::AddTrackAlias("Y", "G");
	FSequencerClipboardReconciler::AddTrackAlias("Z", "B");
	FSequencerClipboardReconciler::AddTrackAlias("W", "A");
}

void FMovieSceneToolsModule::RegisterTakeData(IMovieSceneToolsTakeData* InTakeData)
{
	checkf(!TakeDatas.Contains(InTakeData), TEXT("Take Data is already registered"));
	TakeDatas.Add(InTakeData);
}

void FMovieSceneToolsModule::UnregisterTakeData(IMovieSceneToolsTakeData* InTakeData)
{
	checkf(TakeDatas.Contains(InTakeData), TEXT("Take Data is not registered"));
	TakeDatas.Remove(InTakeData);
}

bool FMovieSceneToolsModule::GatherTakes(const UMovieSceneSection* Section, TArray<uint32>& TakeNumbers, uint32& CurrentTakeNumber)
{
	for (IMovieSceneToolsTakeData* TakeData : TakeDatas)
	{
		if (TakeData->GatherTakes(Section, TakeNumbers, CurrentTakeNumber))
		{
			return true;
		}
	}

	return false;
}


UObject* FMovieSceneToolsModule::GetTake(const UMovieSceneSection* Section, uint32 TakeNumber)
{
	for (IMovieSceneToolsTakeData* TakeData : TakeDatas)
	{
		if (UObject* TakeObject = TakeData->GetTake(Section, TakeNumber))
		{
			return TakeObject;
		}
	}

	return nullptr;
}

IMPLEMENT_MODULE( FMovieSceneToolsModule, MovieSceneTools );

#undef LOCTEXT_NAMESPACE

