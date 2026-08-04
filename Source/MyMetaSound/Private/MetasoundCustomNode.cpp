// Copyright Dan Reynolds, 2026

#include "MetasoundExecutableOperator.h"
#include "MetasoundEnumRegistrationMacro.h"
#include "MetasoundNodeRegistrationMacro.h"
#include "MetasoundStandardNodesNames.h"
#include "MetasoundAudioBuffer.h"
#include "MetasoundFacade.h"
#include "MetasoundParamHelper.h"
#include "MetasoundStandardNodesCategories.h"

#include "Internationalization/Text.h"
#include "DSP/FloatArrayMath.h"

#define LOCTEXT_NAMESPACE "MetasoundCustomNode"

namespace Metasound
{
	/**
	 * Vertex Name Declaration
	 * Vertex names are declared here, it is preferred to couch them in a custom namespace as they're
	 * globally declared and the namespace will help to avoid collisions with similar names in other
	 * operators.
	 */
	namespace CustomProcessorVertexNames
	{
		METASOUND_PARAM(InputA, "Input A Display Name", "Tooltip for Input A.")
		METASOUND_PARAM(InputB, "Input B Display Name", "Tooltip for Input B.")
		
		METASOUND_PARAM(OutputA, "Output A Display Name", "Tooltip for Output A.")
	}
	
	/**
	 * This is a good spot to define custom helper classes or data structs. A lot of MetaSounds use
	 * template typing and this is a good place to define the valid template type variants.
	 */
	
	// Operator Class
	class FCustomOperator : public TExecutableOperator<FCustomOperator>
	{
	public:
		
		/**
		 * Static functions expected and called to provide metadata or act as a factory
		 */
		
		// Provide system with metadata about this node class
		static const FNodeClassMetadata& GetNodeInfo();
		
		// Constructs this class's default vertex interface, types, and values
		static const FVertexInterface& GetDefaultInterface();
		
		// Factory function for constructing this class (as a pointer to IOperator interface)
		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults);
		
		/**
		 * More traditional operator interface (constructor, destructor, and I/O binding)
		 */
		
		// Constructor (called by class's own factory function) 
		// It is a good opportunity to capture MetaSound graph environment data via InSettings data
		FCustomOperator(const FOperatorSettings& InSettings,
			const FTriggerReadRef& InTrigger,
			const FAudioBufferReadRef& InAudio)
				: MyTriggerInput(InTrigger)
				, MyAudioInput(InAudio)
				, MyAudioOutput(FAudioBufferWriteRef::CreateNew(InSettings))
		{
			// Cache environment variables
			SampleRate = InSettings.GetSampleRate();
			NumFramesPerBlock = InSettings.GetNumFramesPerBlock();
			
			// Reset any important member variables/states
			MyAudioOutput->Zero();
			
			// This is a good spot to call an internal data reset function if needed.
		}
		
		// Destructor
		virtual ~FCustomOperator() = default;
		
		// Bind input refs to declared vertices (connects node I/O to ReadRef/WriteRef parameters)
		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override;
		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override;

		/**
		 * Static polymorphism functions, Reset and Execute must be in your class with these signatures.
		 * A modern optimization to avoid relying on v-table look-ups, as I understand the merit of the pattern.
		 */
		
		// Reset function also called by the Operator Cache for recycling
		void Reset(const IOperator::FResetParams& InParams)
		{
			// Cache environment variables
			SampleRate = InParams.OperatorSettings.GetSampleRate();
			NumFramesPerBlock = InParams.OperatorSettings.GetNumFramesPerBlock();
			
			// Reset any important member variables/states
			MyAudioOutput->Zero();
			
			// Similar to the constructor, this is a good spot to call an internal reset function if needed.
		}
		
		// Metasound's version of the "tick" function, called every Metasound graph update
		void Execute()
		{
			// The Trigger ExecuteBlock function takes two lambdas as arguments, these lambdas
			// represent the frames spanning the split of the remaining block.
			MyTriggerInput->ExecuteBlock(
			[](int32 StartFrame, int32 EndFrame){
				// This part of the ExecuteBlock function spans the start of the previous
				// block until the next trigger. If no trigger during this block, this will
				// span the entire block
			},
			[](int32 StartFrame, int32 EndFrame){
				// This part of the ExecuteBlock function spans the start of the block split
				// starting at the trigger until the end of the frame or the next split
			}
			);
			
			// Copies input buffer to output buffer via assignment operator
			*MyAudioOutput = *MyAudioInput;
		}
		
	private:
		
		// Vertex Refs Inputs
		FTriggerReadRef MyTriggerInput;
		FAudioBufferReadRef MyAudioInput;
		
		// Vertex Ref Output
		FAudioBufferWriteRef MyAudioOutput;
		
		// MetaSound operating environment
		float SampleRate;
		int32 NumFramesPerBlock;
	};

	const FNodeClassMetadata& FCustomOperator::GetNodeInfo()
	{
		// Stateless lambda for defining class metadata
		auto InitNodeInfo = []() -> FNodeClassMetadata
		{
			FNodeClassMetadata Info;
			
			// Class name is the exact name used by the MetaSound build system
			// You can set the cvar au.MetaSound.Editor.Debug.ShowNodeDebugData to true to expose node
			// Class names and vertex names to the Editor tooltips
			Info.ClassName = { StandardNodes::Namespace, FName("CustomNode"), TEXT("Variant") };
			
			// MetaSounds support versioning
			Info.MajorVersion = 1;
			Info.MinorVersion = 0;
			
			// Display data for readable node presentation
			Info.DisplayName = METASOUND_LOCTEXT("CustomNodeDisplayName", "Custom Node");
			Info.Description = METASOUND_LOCTEXT("CustomNodeDescriptor", "Demonstrates a custom node description.");
			
			// Additional metadata
			Info.Author = PluginAuthor;
			Info.PromptIfMissing = PluginNodeMissingPrompt;
			
			// Function call to retrieve node's interface (can be changed in more advanced implementations)
			Info.DefaultInterface = GetDefaultInterface();
			
			// How the node is organized in the node library/context menu
			Info.CategoryHierarchy.Emplace(NodeCategories::Functions);
			
			// Additional keywords to help users find your node in the library/context menu
			Info.Keywords.Append({
				METASOUND_LOCTEXT("CustomNodeKeyword_ThingAMaBob", "ThingAMaBob")
			});

			return Info;
		};
		
		static const FNodeClassMetadata Info = InitNodeInfo();
		return Info;
	}

	const FVertexInterface& FCustomOperator::GetDefaultInterface()
	{
		// Stateless lambda for defining the default interface
		// This will add Vertices and associate the previously declared names
		// with types as well as default values
		auto CreateDefaultInterface = []() -> FVertexInterface
		{
			// Input vertices
			FInputVertexInterface InputInterface;
			InputInterface.Add(TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(CustomProcessorVertexNames::InputA)));
			InputInterface.Add(TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(CustomProcessorVertexNames::InputB)));

			// Output vertices
			FOutputVertexInterface OutputInterface;
			OutputInterface.Add(TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(CustomProcessorVertexNames::OutputA)));

			return FVertexInterface(InputInterface, OutputInterface);
		};

		static const FVertexInterface Interface = CreateDefaultInterface();

		return Interface;
	}

	TUniquePtr<IOperator> FCustomOperator::CreateOperator(const FBuildOperatorParams& InParams,
		FBuildResults& OutResults)
	{
		// We use InParams to retrieve default vertex refs (or create them) based on name
		const FInputVertexInterfaceData& InputData = InParams.InputData;
		
		// We get or create refs in anticipation of our constructor requirements
		FTriggerReadRef	TriggerInput = InputData.GetOrCreateDefaultDataReadReference<FTrigger>(METASOUND_GET_PARAM_NAME(CustomProcessorVertexNames::InputA), InParams.OperatorSettings);
		FAudioBufferReadRef	AudioInput = InputData.GetOrCreateDefaultDataReadReference<FAudioBuffer>(METASOUND_GET_PARAM_NAME(CustomProcessorVertexNames::InputB), InParams.OperatorSettings);
		
		// Then we create an operator (factory style) and pass a unique pointer to the caller
		return MakeUnique<FCustomOperator>(InParams.OperatorSettings, TriggerInput, AudioInput);
	}

	void FCustomOperator::BindInputs(FInputVertexInterfaceData& InOutVertexData)
	{
		// This binds our input vertices (by name) to our operator instance's associated member variables
		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(CustomProcessorVertexNames::InputA), MyTriggerInput);
		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(CustomProcessorVertexNames::InputB), MyAudioInput);
	}

	void FCustomOperator::BindOutputs(FOutputVertexInterfaceData& InOutVertexData)
	{
		// This binds our output vertices (by name) to our operator instance's associated member variables
		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(CustomProcessorVertexNames::OutputA), MyAudioOutput);
	}
	
	// Node registration, we register nodes and invoke this node facade to wrap our operator
	using FCustomNode = TNodeFacade<FCustomOperator>;
	
	// We call this macro to queue the registration of our node (which we later call on the module startup)
	METASOUND_REGISTER_NODE(FCustomNode);
}

#undef LOCTEXT_NAMESPACE