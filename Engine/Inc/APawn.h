	//TODO: See if this can be removed
	//Also defined in UnPath.h
	enum EReachSpecFlags{
		R_WALK = 1,	//walking required
		R_FLY = 2,   //flying required
		R_SWIM = 4,  //swimming required
		R_JUMP = 8,   // jumping required
		R_DOOR = 16,
		R_SPECIAL = 32,
		R_PLAYERONLY = 64
	};

	// Constructors.
	APawn(){}

	//Overrides
private:
	virtual class UStimulus* GenerateStimulus(enum EStimulusType);
	virtual class UStimulus* GenerateStimulus(enum EStimulusType, class FVector&);
public:
	virtual void CheckForErrors();
	virtual void DebugOutput(class FOutputDevice&);
	virtual void DebugOutputSelf(class FOutputDevice&);
	virtual void DebugWindowSizeGet(int&, int&)const;
	virtual int DelayScriptReplication(float);
	virtual class FRotator FindSlopeRotation(const FVector&, const FRotator&);
	virtual class ANavigationPoint* GetAnchor();
	virtual float GetNetPriority(class FVector&, class FVector&, class AActor*, float, float);
	virtual int* GetOptimizedRepList(unsigned char*, struct FPropertyRetirement*, int*, class UPackageMap*, class UActorChannel*);
	virtual class APawn* GetPlayerPawn()const;
	virtual int IsAPawn();
	virtual int IsNetRelevantFor(class APlayerController*, class AActor*, const FVector&);
	virtual bool IsPlayer()const;
	virtual class FVector LOSTestLocation()const;
	virtual void NotifyAnimEnd(int);
	virtual void NotifyBumpEx(class AActor*);
	virtual bool PlayerControlled()const;
	virtual void PostNetReceive();
	virtual void PostNetReceiveLocation();
	virtual void PostNetSend();
	virtual void PostRender(class FLevelSceneNode*, class FRenderInterface*);
	virtual void PostScriptDestroyed();
	virtual void PreNetReceive();
	virtual void ReceiveStimulus(class UStimulus*);
	virtual void RenderEditorSelected(class FLevelSceneNode*, class FRenderInterface*, class FDynamicActor*);
	virtual void SetBase(class AActor*, const FVector&, int);
	virtual void SetZone(int, int);
	virtual int ShouldTrace(class AActor*, unsigned long);
	virtual void SmoothHitWall(const FVector&, class AActor*, unsigned char);
	virtual void SnapToNavPt(class ANavigationPoint*);
	virtual int Tick(float, enum ELevelTick);
	virtual void TickSimulated(float);
	virtual void TickSpecial(float);
	virtual void performPhysics(float);
	virtual void physFalling(float, int);
	virtual int physRootMotion(float);
	virtual void processHitWall(const FVector&, class AActor*, unsigned char);
	virtual void processLanded(const FVector&, class AActor*, float, int);
	virtual void stepUp(const FVector&, const FVector&, const FVector&, struct FCheckResult&);

	//Virtual Functions
	virtual bool IsDead()const;
	virtual bool AppearsDead()const;
	virtual int moveToward(const FVector&,class AActor*);
	virtual int pointReachable(const FVector&,int);
	virtual int actorReachable(class AActor*,int,int);
	virtual int HurtByVolume(class AActor*);
	virtual void physicsRotation(float,const FVector&);

	// Events
	void SetAnchor(class ANavigationPoint* Point);
	void StopTalking();
	void SaveSquadTravelInfo();
	void SetWalking(UBOOL bNewIsWalking);
	void ClientMessage(const FString& S, FName Type);
	void ModifyVelocity(FLOAT DeltaTime, const FVector& OldVelocity);
	void EndCrouch(FLOAT HeightAdjust);
	void StartCrouch(FLOAT HeightAdjust);
	void TossWeapon(class AWeapon* WeaponToThrow, const FVector& TossVel);
	class AInventory* FindInventoryType(class UClass* DesiredClass);
	void ChangedWeapon();
	void UpdateEyeHeight(FLOAT DeltaTime);
	FVector EyePosition();
	void DieAgain();
	void BleedOut();
	void HeadVolumeChange(class APhysicsVolume* newHeadVolume);
	void SetAnimAction(FName NewAction);
	void PawnStopFiring();
	void ChangeAnimation();
	void EndJump();
	void PlayJump();
	void PlayFalling();
	void PlayDodging(UBOOL bRight);
	void PlayDying(class UClass* DamageType, const FVector& HitLoc, FName BoneName);
	void Revive();
	UBOOL GoRagDoll(class UClass* DamageType, const FVector& HitLocation);
	void PlayIncapacitated(class UClass* DamageType, const FVector& HitLocation);
	UBOOL IsDeadOrIncapacitated();
	void SetShields(FLOAT NewShields);
	void SetHealthLevel();
	void DoMeleeAttack(FName AttackBone, const FVector& BoneOffset);
	FRotator GetWeaponRotation();
	void PlayDeathCue(class UClass* DamageType);
	void PlaySpottedCue(class AActor* Enemy);
	FString GetHudDescription();
	FString GetHudNickname();

	//Functions
	int CacheNetRelevancy(int,class APlayerController*,class AActor*);
	enum EHealthLevel CalcHealthLevel(float);
	int CanCrouchWalk(const FVector&,const FVector&);
	FVector CheckForLedges(const FVector&,const FVector&,const FVector&,int&,int&);
	void Crouch(int);
	enum ETestMoveResult FindBestJump(const FVector&);
	enum ETestMoveResult FindJumpUp(const FVector&);
	float FindPath(class ANavigationPoint*,class ANavigationPoint*,class TArray<class UReachSpec*>&,bool);
	float GetMaxSpeed();
	enum ETeamRelationship GetRelationTowards(int) const;
	enum ETeamRelationship GetRelationTowards(class APawn const*)const;
	class USound* GetSoundFromCue(unsigned char,float,bool);
	int GetTeamIndex() const;
	float GetWeaponSpeedModifier() const;
	bool HasCueBeenPlayedRecently(unsigned char,float);
	bool HasSoundForCue(unsigned char);
	enum ETestMoveResult HitGoal(class AActor*);
	int IsAvailable() const;
	int IsFlashlightOn() const;
	bool IsHumanControlled() const;
	bool IsIncapacitated() const;
	bool IsLocallyControlled() const;
	int IsUnderAttack(float) const;
	int IsWounded() const;
	FVector NewFallVelocity(const FVector&,const FVector&,float);
	int PickWallAdjust(const FVector&);
	class USound* PlayOwnedCue(unsigned char,float);
	int Reachable(const FVector&,class AActor*);
	int ReachedDestination(const FVector&,class AActor*);
	void SetDodgeInfo();
	void SetTeamIndex(int);
	float SpeedFactor(const FVector&)const;
	FVector SuggestJumpVelocity(const FVector&,float,float);
	void UnCrouch(int);
	void UpdateMovementAnimation(float);
	bool UpdateSkeletalAiming(float);
	int UseWoundedAnims() const;
	int ValidAnchor();
	int calcMoveFlags();
	enum ETestMoveResult flyMove(const FVector&,class AActor*,float);
	int flyReachable(const FVector&,int,class AActor*);
	enum ETestMoveResult jumpLanding(const FVector&,int);
	int jumpReachable(const FVector&,int,class AActor*);
	void physFlying(float,int);
	void physSpider(float,int);
	void physSwimming(float,int);
	void physWalking(float,int);
	void setMoveTimer(float);
	void startNewPhysics(float,int);
	void startSwimming(const FVector&,const FVector&,float,float,int);
	enum ETestMoveResult swimMove(const FVector&,class AActor*,float);
	int swimReachable(const FVector&,int,class AActor*);
	enum ETestMoveResult walkMove(const FVector&,struct FCheckResult&,class AActor*,float);
	int walkReachable(const FVector&,int,class AActor*);

private:
	int Pick3DWallAdjust(const FVector&);
	void SpiderstepUp(const FVector&,const FVector&,struct FCheckResult&);
	float Swim(const FVector&,struct FCheckResult&);
	void UpdateSphyll();
	void calcVelocity(const FVector&,float,float,float,int,int,int);
	int checkFloor(const FVector&,struct FCheckResult&);
	int findNewFloor(const FVector&,float,float,int);
	FVector findWaterLine(const FVector&,const FVector&);
