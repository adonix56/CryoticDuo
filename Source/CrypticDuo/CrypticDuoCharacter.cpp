// Copyright Epic Games, Inc. All Rights Reserved.

#include "CrypticDuoCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ACrypticDuoCharacter

ACrypticDuoCharacter::ACrypticDuoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ACrypticDuoCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACrypticDuoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACrypticDuoCharacter::SendMessage);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACrypticDuoCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACrypticDuoCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACrypticDuoCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ACrypticDuoCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACrypticDuoCharacter::SendMessage()
{
	if (!HasAuthority()) {
		TArray<AActor*> Characters;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACrypticDuoCharacter::StaticClass(), Characters);
		for (AActor* Character : Characters) {
			if (ACrypticDuoCharacter* CDChar = Cast<ACrypticDuoCharacter>(Character)) {
				CDChar->ServerRPCFunction(ValidateInt);
			}
		}
	}
}

void ACrypticDuoCharacter::ServerRPCFunction_Implementation(int arg) {
	if (HasAuthority()) {
#if false
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, FString::Printf(TEXT("Reliable Server Message Received Implementation!")));
		UE_LOG(LogTemp, Warning, TEXT("Reliable Server Message Received"));
#endif
		if (!SphereActor) {
			return;
		}
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		AStaticMeshActor* StaticMesh = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnParameters);
		if (StaticMesh) {
			StaticMesh->SetReplicates(true);
			StaticMesh->SetReplicateMovement(true);
			StaticMesh->SetMobility(EComponentMobility::Movable);
			FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.f + GetActorUpVector() * 50.f;
			StaticMesh->SetActorLocation(SpawnLocation);
			UStaticMeshComponent* StaticMeshComponent = StaticMesh->GetStaticMeshComponent();
			if (StaticMeshComponent) {
				StaticMeshComponent->SetIsReplicated(true);
				StaticMeshComponent->SetSimulatePhysics(true);
				StaticMeshComponent->SetStaticMesh(SphereActor);
			}
		}
	}
#if false
	else {
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Magenta, FString::Printf(TEXT("Reliable Client Message Received Implementation!")));
		UE_LOG(LogTemp, Warning, TEXT("Reliable Client Message Received"));
	}
#endif
}

bool ACrypticDuoCharacter::ServerRPCFunction_Validate(int arg) {
	return (arg >= 0 && arg <= 100);
}

void ACrypticDuoCharacter::ClientRPCFunction_Implementation() {
	if (HasAuthority()) {
		if (Explosion) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Explosion, GetTransform(), true, EPSCPoolMethod::AutoRelease);
		}
	}
}