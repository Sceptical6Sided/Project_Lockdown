// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CustomGameModeBase.h"

DEFINE_LOG_CATEGORY(GameServerLog);

ACustomGameModeBase::ACustomGameModeBase()
{
}

void ACustomGameModeBase::BeginPlay()
{
	Super::BeginPlay();
#if WITH_GAMELIFT
	InitGameLift();
#endif
}

void ACustomGameModeBase::InitGameLift()
{
	UE_LOG(GameServerLog, Log, TEXT("Initializing the GameLift Server"));

    //Getting the module first.
    FGameLiftServerSDKModule* gameLiftSdkModule = &FModuleManager::LoadModuleChecked<FGameLiftServerSDKModule>(FName("GameLiftServerSDK"));

    //Define the server parameters for an Amazon GameLift Servers Anywhere fleet. These are not needed for an Amazon GameLift Servers managed EC2 fleet.
    FServerParameters serverParameters;

    //AuthToken returned from the "aws GameLift get-compute-auth-token" API. Note this will expire and require a new call to the API after 15 minutes.
    if (FParse::Value(FCommandLine::Get(), TEXT("-authtoken="), serverParameters.m_authToken))
    {
        UE_LOG(GameServerLog, Log, TEXT("AUTH_TOKEN: %s"), *serverParameters.m_authToken)
    }

    if (FParse::Value(FCommandLine::Get(), TEXT("-awsregion="), serverParameters.m_awsRegion))
    {
        UE_LOG(GameServerLog, Log, TEXT("AWS_REGION: %s"), *serverParameters.m_awsRegion)
    }


    if (FParse::Value(FCommandLine::Get(), TEXT("-accesskey="), serverParameters.m_accessKey))
    {
        UE_LOG(GameServerLog, Log, TEXT("ACCESS_KEY: %s"), *serverParameters.m_accessKey)
    }
    if (FParse::Value(FCommandLine::Get(), TEXT("-secretkey="), serverParameters.m_secretKey))
    {
        UE_LOG(GameServerLog, Log, TEXT("SECRET_KEY: % s"), *serverParameters.m_secretKey)
    }
    if (FParse::Value(FCommandLine::Get(), TEXT("-sessiontoken="), serverParameters.m_sessionToken))
    {
        UE_LOG(GameServerLog, Log, TEXT("SESSION_TOKEN: %s"), *serverParameters.m_sessionToken)
    }

    //The Host/compute-name of the Amazon GameLift Servers Anywhere instance.
    if (FParse::Value(FCommandLine::Get(), TEXT("-hostid="), serverParameters.m_hostId))
    {
        UE_LOG(GameServerLog, Log, TEXT("HOST_ID: %s"), *serverParameters.m_hostId)
    }

    //The Anywhere Fleet ID.
    if (FParse::Value(FCommandLine::Get(), TEXT("-fleetid="), serverParameters.m_fleetId))
    {
        UE_LOG(GameServerLog, Log, TEXT("FLEET_ID: %s"), *serverParameters.m_fleetId)
    }

    //The WebSocket URL (GameLiftServiceSdkEndpoint).
    if (FParse::Value(FCommandLine::Get(), TEXT("-websocketurl="), serverParameters.m_webSocketUrl))
    {
        UE_LOG(GameServerLog, Log, TEXT("WEBSOCKET_URL: %s"), *serverParameters.m_webSocketUrl)
    }



    FString glProcessId = "";
    if (FParse::Value(FCommandLine::Get(), TEXT("-processId="), glProcessId))
    {
        serverParameters.m_processId = TCHAR_TO_UTF8(*glProcessId);
    }
    else
    {
        // If no ProcessId is passed as a command line argument, generate a randomized unique string.
        FString TimeString = FString::FromInt(std::time(nullptr));
        FString ProcessId = "ProcessId_" + TimeString;
        serverParameters.m_processId = TCHAR_TO_UTF8(*ProcessId);
    }

    //The PID of the running process
    UE_LOG(GameServerLog, Log, TEXT("PID: %s"), *serverParameters.m_processId);

    //InitSDK establishes a local connection with the Amazon GameLift Servers Agent to enable further communication.
    //Use InitSDK(serverParameters) for an Amazon GameLift Servers Anywhere fleet. 
    //Use InitSDK() for Amazon GameLift Servers managed EC2 fleet.
    gameLiftSdkModule->InitSDK(serverParameters);

    //Implement callback function onStartGameSession
    //Amazon GameLift Servers sends a game session activation request to the game server
    //and passes a game session object with game properties and other settings.
    //Here is where a game server takes action based on the game session object.
    //When the game server is ready to receive incoming player connections, 
    //it invokes the server SDK call ActivateGameSession().
    auto onGameSession = [=](Aws::GameLift::Server::Model::GameSession gameSession)
        {
            FString gameSessionId = FString(gameSession.GetGameSessionId());
            UE_LOG(GameServerLog, Log, TEXT("GameSession Initializing: %s"), *gameSessionId);
            gameLiftSdkModule->ActivateGameSession();
        };

    m_params.OnStartGameSession.BindLambda(onGameSession);

    //Implement callback function OnProcessTerminate
    //Amazon GameLift Servers invokes this callback before shutting down the instance hosting this game server.
    //It gives the game server a chance to save its state, communicate with services, etc., 
    //and initiate shut down. When the game server is ready to shut down, it invokes the 
    //server SDK call ProcessEnding() to tell Amazon GameLift Servers it is shutting down.
    auto onProcessTerminate = [=]()
        {
            UE_LOG(GameServerLog, Log, TEXT("Game Server Process is terminating"));
            gameLiftSdkModule->ProcessEnding();
        };

    m_params.OnTerminate.BindLambda(onProcessTerminate);

    //Implement callback function OnHealthCheck
    //Amazon GameLift Servers invokes this callback approximately every 60 seconds.
    //A game server might want to check the health of dependencies, etc.
    //Then it returns health status true if healthy, false otherwise.
    //The game server must respond within 60 seconds, or Amazon GameLift Servers records 'false'.
    //In this example, the game server always reports healthy.
    auto onHealthCheck = []()
        {
            UE_LOG(GameServerLog, Log, TEXT("Performing Health Check"));
            return true;
        };

    m_params.OnHealthCheck.BindLambda(onHealthCheck);

    //The game server gets ready to report that it is ready to host game sessions
    //and that it will listen on port 7777 for incoming player connections.
    m_params.port = 7777;

    //Here, the game server tells Amazon GameLift Servers where to find game session log files.
    //At the end of a game session, Amazon GameLift Servers uploads everything in the specified 
    //location and stores it in the cloud for access later.
    TArray<FString> logfiles;
    logfiles.Add(TEXT("GameLift426Test/Saved/Logs/GameLift426Test.log"));
    m_params.logParameters = logfiles;

    //The game server calls ProcessReady() to tell Amazon GameLift Servers it's ready to host game sessions.
    UE_LOG(GameServerLog, Log, TEXT("Calling Process Ready"));
    gameLiftSdkModule->ProcessReady(m_params);
}
