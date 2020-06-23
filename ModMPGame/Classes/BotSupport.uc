class BotSupport extends AdminService native;

#exec OBJ LOAD FILE="MPGame.u"

var() config int            NumBots;             // Number of bots which are spawned when the game begins
var() config float          BotAccuracy;
var Array<MPBot>            Bots;
var() config bool           bBotsCountAsPlayers; // If this is true, adding bots will increase the player count of the server

var config bool             bAutoImportPaths;
var config bool             bAutoBuildPaths;
var config bool             bShowPaths;

var bool                    bPathsImported;       // Paths were imported using ImportPaths
var bool                    bShowPathsOnClients;
var Array<vector>           NavPtFailLocations;   // Used to debug Navigation points which failed to spawn
var Array<Pawn.PatrolPoint> BotPatrolRoute;
var Array<Actor>            NavigationPointIcons; // Intangible actors used to make navigation points visible to clients

function PostBeginPlay(){
	local int i;

	BotAccuracy = FClamp(BotAccuracy, 0.0, 1.0);

	for(i = 0; i < NumBots; ++i)
		AddBot();
}

function ShowPathsClient(){
	local NavigationPoint N;
	local Actor A;

	foreach AllActors(class'NavigationPoint', N){
		A = Spawn(class'IntangibleActor',,, N.Location);

		if(A != None){
			A.SetDrawType(DT_Sprite);

			if(PlayerStart(N) != None)
				A.Texture = Texture'Engine.S_Player';
			else if(CoverPoint(N) != None)
				A.Texture = Texture'Engine.S_CoverPoint';
			else if(PatrolPoint(N) != None)
				A.Texture = Texture'Engine.S_LookTarget';
			else
				A.Texture = Texture'Engine.S_Pickup';

			NavigationPointIcons[NavigationPointIcons.Length] = A;
		}
	}
}

function HidePathsClient(){
	local int i;

	for(i = 0; i < NavigationPointIcons.Length; ++i)
		NavigationPointIcons[i].Destroy();

	NavigationPointIcons.Length = 0;
}

function bool ExecCmd(String Cmd, optional PlayerController PC){
	local int i;
	local class<Pawn> PawnClass;
	local string StringParam;
	local NavigationPoint N;
	local vector SpawnPos;
	local Pawn P;

	if(ParseCommand(Cmd, "ADDBOT")){
		AddBot();

		return true;
	}else if(ParseCommand(Cmd, "REMOVEBOT")){
		RemoveBot();

		return true;
	}else if(ParseCommand(Cmd, "REMOVEALLBOTS")){
		while(Bots.Length > 0)
			RemoveBot();

		return true;
	}else if(ParseCommand(Cmd, "SETBOTACCURACY")){
		BotAccuracy = FClamp(float(Cmd), 0.0, 1.0);

		for(i = 0; i < Bots.Length; ++i)
			Bots[i].SetAccuracy(BotAccuracy);

		return true;
	}else if(ParseCommand(Cmd, "SPAWN")){
		StringParam = ParseToken(Cmd);

		if(StringParam != ""){
			PawnClass = class<Pawn>(DynamicLoadObject(StringParam, class'Class', true));

			// Allowing omitting package name for convenience for 'CTCharacters' and 'Properties'

			if(PawnClass == None)
				PawnClass = class<Pawn>(DynamicLoadObject("CTCharacters." $ StringParam, class'Class', true));

			if(PawnClass == None)
				PawnClass = class<Pawn>(DynamicLoadObject("Properties." $ StringParam, class'Class', true));

			if(PawnClass != None){
				if(PC != None){
					if(PC.Pawn != None)
						SpawnPos = PC.Pawn.Location + vector(PC.GetViewRotation()) * (PC.Pawn.CollisionRadius + PawnClass.default.CollisionRadius + 4);
					else
						SpawnPos = PC.Location;
				}else{
					SpawnPos = Level.NavigationPointList.Location; // TODO: Pick random navigation point
				}

				P = Spawn(PawnClass,,, SpawnPos);

				if(P != None){
					P.Controller = Spawn(P.ControllerClass);

					if(P.Controller != None){
						P.Controller.Possess(P);
						P.Controller.PlayerReplicationInfo = Spawn(class'MPPlayerReplicationInfo');
						P.Controller.InitPlayerReplicationInfo();
						P.Controller.PlayerReplicationInfo.PlayerName = string(P.Class.Name);
					}

					P.AddDefaultInventory();

					if(ParseIntParam(Cmd, "TEAM=", i))
						P.TeamIndex = i;
				}else{
					CommandFeedback(PC, "Unable to spawn pawn, try again at a different location");
				}
			}else{
				CommandFeedback(PC, "Pawn class '" $ StringParam $ "' not found");
			}
		}else if(PC != None){
			CommandFeedback(PC, "Expected class name");
		}

		return true;
	}else if(!IsLocalPlayer(PC)){
		if(ParseCommand(Cmd, "SHOWPATHS")){
			if(!bShowPathsOnClients)
				ShowPathsClient();

			bShowPathsOnClients = true;

			return true;
		}else if(ParseCommand(Cmd, "HIDEPATHS")){
			if(bShowPathsOnClients)
				HidePathsClient();

			bShowPathsOnClients = false;

			return true;
		}
	}

	if(Super.ExecCmd(Cmd, PC)){
		if(bShowPathsOnClients){ // A native command might have changed the navigation points so we have to regenerate the dummy actors
			HidePathsClient();
			ShowPathsClient();
		}

		return true;
	}

	return false;
}

event SetupPatrolRoute(){
	local NavigationPoint NavPt;
	local Pawn.PatrolPoint P;

	BotPatrolRoute.Length = 0;

	for(NavPt = Level.NavigationPointList; NavPt != None; NavPt = NavPt.nextNavigationPoint){
		if(PatrolPoint(NavPt) != None){
			P.Node = NavPt;
			P.RunToNode = true;
			P.ShootWhileMoving = true;
			P.OrientToNode = true;
			P.PatrolPriorityOverride = 0.3;
			BotPatrolRoute[BotPatrolRoute.Length] = P;
		}
	}
}

function AddBot(){
	local MPBot Bot;

	if(bBotsCountAsPlayers && Level.Game.NumPlayers >= Level.Game.MaxPlayers){
		Log("Game is full and bBotsCountAsPlayers == true");

		return;
	}

	Bot = Spawn(class'MPBot', self);

	if(Bot != None){
		Bot.Accuracy = BotAccuracy;
		Bot.PlayerReplicationInfo.PlayerName = "Bot" $ Bots.Length;
		Bot.bCanGesture = false;
		Bot.ChosenSkin = Rand(5);
		Bot.GotoState('Dead', 'MPStart');
		Bots[Bots.Length] = Bot;

		BroadcastLocalizedMessage(Level.Game.GameMessageClass, 1, Bot.PlayerReplicationInfo);

		if(bBotsCountAsPlayers)
			++Level.Game.NumPlayers;
		else
			++Level.Game.NumBots;
	}
}

function RemoveBot(){
	local int i;

	if(Bots.Length > 0){
		i = Bots.Length - 1;

		if(Bots[i].Pawn != None)
			Bots[i].Pawn.Destroy();

		Bots[i].Destroy();
		Bots.Length = i;

		if(bBotsCountAsPlayers)
			--Level.Game.NumPlayers;
		else
			--Level.Game.NumBots;
	}
}

cpptext
{
	void SpawnNavigationPoint(UClass* NavPtClass, const FVector& Location, const FRotator& Rotation = FRotator(0, 0, 0));
	void ImportPaths();
	void ExportPaths();
	void BuildPaths();
	void ClearPaths();

	// Overrides
	virtual void Spawned();
	virtual UBOOL Tick(FLOAT DeltaTime, ELevelTick TickType);
	virtual void PostRender(class FLevelSceneNode* SceneNode, class FRenderInterface* RI);
	virtual bool ExecCmd(const char* Cmd, class APlayerController* PC);
}

defaultproperties
{
	BotAccuracy=1.0
	bHidden=true
	bAutoImportPaths=true
	bAutoBuildPaths=false
	bShowPaths=false
}
