// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PawnMovementComponent.h"
#include "GridMovementComponent.generated.h"

class ANavGrid;
class USplineComponent;
class USplineMeshComponent;
class UStaticMesh;
class UNavTileComponent;

UENUM(BlueprintType)
enum class EGridMovementMode : uint8
{
	Stationary		UMETA(DisplayName = "Stationary"),
	Walking			UMETA(DisplayName = "Walking"),
	ClimbingUp 		UMETA(DisplayName = "Climbing up"),
	ClimbingDown	UMETA(DisplayName = "Climbing down"),
	InPlaceTurn     UMETA(DisplayName = "Turn in place"),
};

UENUM(BlueprintType)
enum class EGridMovementPhase : uint8
{
	Beginning		UMETA(DisplayName = "Beginning"),
	Middle			UMETA(DisplayName = "Middle"),
	Ending			UMETA(DisplayName = "Ending"),
	Done			UMETA(DisplayName = "Done")
};

USTRUCT()
struct FPathSegment
{
	GENERATED_BODY()
	FPathSegment() {}
	FPathSegment(TSet<EGridMovementMode> InMovementModes, float InStart, float InEnd);
	/* Legal movement modes for this segment */
	TSet<EGridMovementMode> MovementModes;
	/* start and end distance along the path spline this segment covers */
	float Start, End;
	FRotator PawnRotationHint;
};


/**
 * A movement component that operates on a NavGrid
 */
UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class NAVGRID_API UGridMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
public:
	UGridMovementComponent(const FObjectInitializer &ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

protected:
	/* return an transform usable for following the spline path */
	FTransform TransformFromPath(float DeltaTime);
	/* return an tranfrom usable for rotation in place */
	FTransform TransformFromRotation(float DeltaTime);
public:
	void ConsiderUpdateCurrentTile();
protected:
	/* The tile we're currently on */
	UPROPERTY()
	UNavTileComponent *CurrentTile = NULL;
	FPathSegment CurrentPathSegment;
public:

	/* Return the tiles that are in range */
	void GetTilesInRange(TArray<UNavTileComponent *> &OutTiles);
	/* Get the tile the pawn is on, returns NULL if the pawn is not on a tile */
	UNavTileComponent *GetTile();
	/* Get the tile the pawn would occupy of it was located at a different position.
	May return NULL if no tile is found.*/
	UNavTileComponent *GetTile(const FVector &Position);
	ANavGrid *GetNavGrid();
	/* How far (in tile cost) the actor can move in one go */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
	float MovementRange = 4;
	/* How fast can the actor move when walking*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
	float MaxWalkSpeed = 450;
	/* How fast can the actor move when climbing */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
	float MaxClimbSpeed = 200;
	/* How fast can the actor turn */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
	float MaxRotationSpeed = 720;
	/* Steepest slope the actor can walk up or down */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
	float MaxWalkAngle = 45;
	/* MovementModes usable for this Pawn */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Movement")
	TSet<EGridMovementMode> AvailableMovementModes;
	/* Should we ignore rotation over the X axis */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Movement")
	bool LockRoll = true;
	/* Should we ignore rotation over the Y axis */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Movement")
	bool LockPitch = true;
	/* Should we ignore rotation over the Z axis */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Movement")
	bool LockYaw = false;
	/* Should we extract root motion for speed while moving */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Movement")
	bool bUseRootMotion = true;
	/* Should we extract root motion for speed and rotation even if we are not moving*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Movement")
	bool bAlwaysUseRootMotion = false;
	/* Set this to stop moving a certain distance from the path end point, useful if
	you bAlwaysUseRootMotion and have a walk-end animation that contains some movement */
	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "Movement")
	float StoppingDistance = 0;
	/* Set this to the length of the stopping animation if you want the component to adjust
	the movent speed during the movement end-phase in order to excately stop at the path enpoint.
	Useful if you are not able to find an exact number for StoppingDistance */
	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "Movement")
	float StoppingTime = 0;

	/* Should we straighten out the path to avoid zigzaging */
	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "Movement")
	bool bStringPullPath = true;
	void StringPull(TArray<const UNavTileComponent *> &InOutPath, TArray<const UNavTileComponent*>& OutPath);

	/*
	Spline that is used as a path. The points are in world coords.

	We use ESplineCoordinateSpace::Local in the getters and setters to avoid any extra coord translation
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Visualization")
	USplineComponent *Spline = NULL;
	/* Mesh used to visualize the path */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Visualization")
	UStaticMesh *PathMesh = NULL;
	/* Distance between actor and where we start showing the path */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Visualization")
	float HorizontalOffset = 87.5;

	/* Create a path to Target, return false if no path is found */
	bool CreatePath(const UNavTileComponent &Target);
	/* Create a path and follow it if it exists */
	bool MoveTo(const UNavTileComponent &Target);
	/* Turn in place */
	void TurnTo(const FRotator &Forward);
	/* Snap actor the grid */
	void SnapToGrid();
	/* Get the remaining distance of the current path (zero if the pawn is currently not moving) */
	float GetRemainingDistance();
	/* Use actor rotation for components where we have an rotation locks, use InRotation for the rest */
	FRotator ApplyRotationLocks(const FRotator &InRotation);
protected:
	FRotator DesiredForwardRotation;
public:
	/* Visualize path */
	void ShowPath();
	/* Hide path */
	void HidePath();

	FTransform ConsumeRootMotion();

	EGridMovementMode GetMovementMode() { return MovementMode; }
	EGridMovementPhase GetMovementPhase() { return MovementPhase; }
protected:
	EGridMovementMode MovementMode;
	void ConsiderUpdateMovementMode();
	void ChangeMovementMode(EGridMovementMode NewMode);
	EGridMovementPhase MovementPhase;
public:
	/* Return the point the the pawn will reach if it continues moving for ForwardDistance */
	FVector GetForwardLocation(float ForwardDistance);

	DECLARE_EVENT(UGridMovementComponent, FOnMovementDone);
	/* Triggered when movement ends */
	FOnMovementDone& OnMovementEnd() { return OnMovementEndEvent; }
private:
	FOnMovementDone OnMovementEndEvent;

public:
	DECLARE_EVENT_TwoParams(UGridMovementComponent, FOnMovementModeChanged, EGridMovementMode, EGridMovementMode);
	/* Triggered when the movement mode changes */
	FOnMovementModeChanged& OnMovementModeChanged() { return OnMovementModeChangedEvent; }
private:
	FOnMovementModeChanged OnMovementModeChangedEvent;

protected:
	UPROPERTY()
	TArray<USplineMeshComponent *> SplineMeshes;

	/* Helper: Puts a spline mesh in the range along the spline */
	void AddSplineMesh(float From, float To);

	/* How far along the spline are we */
	float Distance = 0;

	/* the grid we're currently on */
	UPROPERTY()
	ANavGrid *Grid = NULL;

	UPROPERTY()
	UAnimInstance *AnimInstance;

	/* Return a delta FRotater that is within MaxRotationSpeed */
	FRotator LimitRotation(const FRotator &OldRotation, const FRotator &NewRotation, float DeltaTime);
	/* The rotation of the skeletal mesh (if any). Used to handle root motion rotation */
	FRotator MeshRotation;

	UPROPERTY()
	TArray<FPathSegment> PathSegments;
};
