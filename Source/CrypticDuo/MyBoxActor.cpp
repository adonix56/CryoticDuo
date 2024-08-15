// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBoxActor.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMyBoxActor::AMyBoxActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	SetRootComponent(SceneComponent);
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh Component"));
	StaticMeshComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMyBoxActor::BeginPlay()
{
	Super::BeginPlay();
	if (StaticMeshComponent && BoxMesh) {
		StaticMeshComponent->SetIsReplicated(true);
		StaticMeshComponent->SetStaticMesh(BoxMesh);
	}
	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called every frame
void AMyBoxActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMyBoxActor::NetMulticastRPC_Implementation() {
	if (HasAuthority()) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Server Multicast"));
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Client Multicast %d"), GPlayInEditorID));
	}

	if (!IsRunningDedicatedServer()) {
		FVector SpawnLocation = GetActorLocation() + GetActorUpVector() * 100.f;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Explosion, SpawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
	}
}