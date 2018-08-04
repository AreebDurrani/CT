/*===========================================================================
    C++ class definitions exported from UnrealScript.
    This is automatically generated by the tools.
    DO NOT modify this manually! Edit the corresponding .uc files instead!
===========================================================================*/

#ifndef MODMPGAME_NATIVE_DEFS
#define MODMPGAME_NATIVE_DEFS

#if SUPPORTS_PRAGMA_PACK
#pragma pack (push,4)
#endif

#ifndef MODMPGAME_API
#define MODMPGAME_API DLL_IMPORT
#endif


struct MODMPGAME_API FPoints
{
    TArrayNoInit<class APlayerStart*> SpawnPoints;
};


class MODMPGAME_API ABotSupport : public AActor
{
public:
    TArrayNoInit<class AMPBot*> Bots;
    TArrayNoInit<FPoints> SpawnPointsByTeam;
    class AScriptedSequence* BotScript;
    void execImportPaths(FFrame& Stack, void* Result);
    DECLARE_CLASS(ABotSupport,AActor,0,ModMPGame)
    NO_DEFAULT_CONSTRUCTOR(ABotSupport)
    DECLARE_NATIVES(ABotSupport)
};



#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif

#if __STATIC_LINK

#define AUTO_INITIALIZE_REGISTRANTS_MODMPGAME \
	ABotSupport::StaticClass(); \

#endif // __STATIC_LINK

#endif // CORE_NATIVE_DEFS
